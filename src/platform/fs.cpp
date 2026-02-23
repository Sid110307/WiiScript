#include "./platform.h"

#include <cstring>
#include <algorithm>

#include <sys/unistd.h>
#include <dirent.h>
#include <fat.h>

static bool ready = false;

static bool isInsideWorkspace(const std::string& path)
{
    const std::string p = FileSystem::normalize(path);
    return !p.empty() && p != ".." && p.rfind("../", 0) != 0 && p.find("/../") == std::string::npos && (p.size() < 3 ||
        p.compare(p.size() - 3, 3, "/..") != 0) && p.rfind(FileSystem::workspaceRoot, 0) == 0;
}

static std::string stripPrefix(std::string s)
{
    s = FileSystem::normalize(std::move(s));
    while (s.size() > 3 && s.back() == '/') s.pop_back();

    return s;
}

static bool copyFileTo(const std::string& src, const std::string& dst)
{
    std::vector<uint8_t> data;
    if (!FileSystem::readFile(src, data)) return false;

    return FileSystem::writeFile(dst, data);
}

static bool copyDirRecursive(const std::string& srcDir, const std::string& dstDir)
{
    if (!FileSystem::makeDir(dstDir)) return false;

    std::vector<FileSystem::DirEntry> entries;
    if (!FileSystem::listDir(srcDir, entries, false)) return false;

    return std::all_of(entries.begin(), entries.end(), [&](const FileSystem::DirEntry& e)
    {
        const std::string src = e.path;
        const std::string dst = FileSystem::normalize(dstDir + "/" + e.name);

        return e.isDir ? copyDirRecursive(src, dst) : copyFileTo(src, dst);
    });
}

static bool deleteDirRecursive(const std::string& dir)
{
    std::vector<FileSystem::DirEntry> entries;
    if (!FileSystem::listDir(dir, entries, false)) return false;

    return std::all_of(entries.begin(), entries.end(), [&](const FileSystem::DirEntry& e)
    {
        const std::string path = e.path;
        return e.isDir ? deleteDirRecursive(path) : remove(path.c_str()) == 0;
    }) && rmdir(dir.c_str()) == 0;
}

bool FileSystem::init()
{
    if (ready) return true;

    ready = fatInitDefault();
    return ready;
}

bool FileSystem::isReady() { return ready; }

bool FileSystem::ensureDir(const std::string& path)
{
    std::string p = normalize(path);
    if (p.empty()) return false;
    if (isDir(p)) return true;

    std::string cur;
    if (p.rfind("sd:/", 0) == 0)
    {
        cur = "sd:/";
        p = p.substr(4);
    }

    size_t i = 0;
    while (i < p.size())
    {
        const size_t j = p.find('/', i);
        if (std::string part = j == std::string::npos ? p.substr(i) : p.substr(i, j - i); !part.empty())
        {
            cur = join(cur, part);
            if (!isDir(cur) && mkdir(cur.c_str(), 0777) != 0 && errno != EEXIST) return false;
        }

        if (j == std::string::npos) break;
        i = j + 1;
    }

    return isDir(cur);
}

bool FileSystem::exists(const std::string& path)
{
    struct stat st = {};
    return stat(normalize(path).c_str(), &st) == 0;
}

bool FileSystem::isDir(const std::string& path)
{
    struct stat st = {};
    if (stat(normalize(path).c_str(), &st) != 0) return false;

    return S_ISDIR(st.st_mode);
}

std::string FileSystem::normalize(std::string path)
{
    for (auto& c : path) if (c == '\\') c = '/';
    return path;
}

std::string FileSystem::join(const std::string& a, const std::string& b)
{
    if (a.empty()) return normalize(b);
    if (b.empty()) return normalize(a);
    if (a.back() == '/' || a.back() == '\\') return normalize(a + b);
    return normalize(a + "/" + b);
}

bool FileSystem::listDir(const std::string& path, std::vector<DirEntry>& outEntries, bool sort)
{
    outEntries.clear();
    const std::string p = normalize(path);
    if (p.empty()) return false;

    DIR* dir = opendir(p.c_str());
    if (!dir) return false;

    while (true)
    {
        errno = 0;
        dirent* entry = readdir(dir);

        if (!entry) break;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        DirEntry e = {.name = entry->d_name, .path = join(p, entry->d_name)};

        struct stat st = {};
        if (stat(e.path.c_str(), &st) == 0)
        {
            e.isDir = S_ISDIR(st.st_mode);
            e.size = st.st_size;
        }
        else
        {
            e.isDir = entry->d_type == DT_DIR;
            e.size = 0;
        }

        outEntries.push_back(std::move(e));
    }

    closedir(dir);
    if (sort)
        std::sort(outEntries.begin(), outEntries.end(), [](const DirEntry& a, const DirEntry& b)
        {
            return a.isDir != b.isDir ? a.isDir > b.isDir : a.name < b.name;
        });

    return true;
}

bool FileSystem::readFile(const std::string& path, std::vector<uint8_t>& outData)
{
    outData.clear();
    const std::string p = normalize(path);
    if (p.empty()) return false;

    FILE* f = fopen(p.c_str(), "rb");
    if (!f) return false;

    if (fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        return false;
    }

    const long size = ftell(f);
    if (size < 0)
    {
        fclose(f);
        return false;
    }

    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return false;
    }

    outData.resize(size);
    if (fread(outData.data(), 1, size, f) != static_cast<size_t>(size))
    {
        fclose(f);
        outData.clear();

        return false;
    }

    fclose(f);
    return true;
}

bool FileSystem::writeFile(const std::string& path, const std::vector<uint8_t>& data)
{
    const std::string p = normalize(path);

    if (!isInsideWorkspace(p)) return false;
    if (const auto slash = p.find_last_of('/'); slash != std::string::npos && !ensureDir(p.substr(0, slash)))
        return false;

    const std::string temp = p + ".tmp";
    FILE* f = fopen(temp.c_str(), "wb");
    if (!f) return false;

    if (!data.empty() && fwrite(data.data(), 1, data.size(), f) != data.size())
    {
        fclose(f);
        remove(temp.c_str());

        return false;
    }

    fflush(f);
    fclose(f);
    remove(p.c_str());

    return rename(temp.c_str(), p.c_str()) == 0;
}

bool FileSystem::makeDir(const std::string& path)
{
    const std::string p = normalize(path);
    if (!isInsideWorkspace(p)) return false;
    if (p.empty()) return false;
    if (isDir(p)) return true;

    return mkdir(p.c_str(), 0777) == 0 || errno == EEXIST;
}

bool FileSystem::renamePath(const std::string& from, const std::string& to)
{
    const std::string src = normalize(from), dst = normalize(to);
    return !isInsideWorkspace(src) || !isInsideWorkspace(dst) || !exists(src) || exists(dst)
               ? false
               : rename(src.c_str(), dst.c_str()) == 0;
}

bool FileSystem::copyPath(const std::string& from, const std::string& to)
{
    const std::string src = stripPrefix(from), dst = stripPrefix(to);

    if (!isInsideWorkspace(src) || !isInsideWorkspace(dst)) return false;
    if (!exists(src)) return false;
    if (exists(dst)) return false;

    return isDir(src) ? copyDirRecursive(src, dst) : copyFileTo(src, dst);
}

bool FileSystem::removePath(const std::string& path)
{
    const std::string p = stripPrefix(path);
    if (!isInsideWorkspace(p) || !exists(p)) return false;

    return isDir(p) ? deleteDirRecursive(p) : remove(p.c_str()) == 0;
}
