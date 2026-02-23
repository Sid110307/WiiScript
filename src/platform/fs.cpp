#include "./platform.h"

#include <cstring>
#include <algorithm>

#include <dirent.h>
#include <fat.h>

static bool ready = false;

static std::string normalize(std::string path)
{
    for (auto& c : path) if (c == '\\') c = '/';
    return path;
}

static std::string join(const std::string& a, const std::string& b)
{
    if (a.empty()) return normalize(b);
    if (b.empty()) return normalize(a);
    if (a.back() == '/' || a.back() == '\\') return normalize(a + b);
    return normalize(a + "/" + b);
}

static bool hasParentPath(const std::string& p)
{
    return p == ".." || p.rfind("../", 0) == 0 || p.find("/../") != std::string::npos ||
        (p.size() >= 3 && p.compare(p.size() - 3, 3, "/..") == 0);
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

    if (p.empty() || hasParentPath(p) || p.rfind(workspaceRoot, 0) != 0) return false;
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
