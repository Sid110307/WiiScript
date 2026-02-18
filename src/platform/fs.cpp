#include "./platform.h"

#include <cstring>
#include <algorithm>

#include <dirent.h>
#include <fat.h>

static bool ready = false;

static bool endsWithSlash(const std::string& path)
{
    return !path.empty() && (path.back() == '/' || path.back() == '\\');
}

static std::string normalizePath(std::string path)
{
    for (auto& c : path) if (c == '\\') c = '/';
    return path;
}


static std::string join(const std::string& a, const std::string& b)
{
    if (a.empty()) return normalizePath(b);
    if (b.empty()) return normalizePath(a);
    if (endsWithSlash(a)) return normalizePath(a + b);
    return normalizePath(a + "/" + b);
}

bool FileSystem::init()
{
    if (ready) return true;

    ready = fatInitDefault();
    return ready;
}

bool isReady() { return ready; }

std::string FileSystem::appRoot() { return "sd:/apps/WiiScript/"; }
std::string FileSystem::workspaceRoot() { return "sd:/WiiScript/"; }

bool FileSystem::ensureDir(const std::string& path)
{
    std::string p = normalizePath(path);
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
        size_t j = p.find('/', i);
        std::string part = j == std::string::npos ? p.substr(i) : p.substr(i, j - i);

        if (!part.empty())
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
    return stat(normalizePath(path).c_str(), &st) == 0;
}

bool FileSystem::isDir(const std::string& path)
{
    struct stat st = {};
    if (stat(normalizePath(path).c_str(), &st) != 0) return false;

    return S_ISDIR(st.st_mode);
}

bool FileSystem::listDir(const std::string& path, std::vector<DirEntry>& outEntries, bool sort)
{
    outEntries.clear();
    std::string p = normalizePath(path);
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

bool FileSystem::readFile(const std::string& path, std::string& outData)
{
    outData.clear();
    const std::string p = normalizePath(path);
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
    std::string normalize;
    normalize.reserve(outData.size());

    for (size_t i = 0; i < outData.size(); ++i)
        if (const char c = outData[i]; c == '\r')
        {
            if (i + 1 < outData.size() && outData[i + 1] == '\n') continue;
            normalize.push_back('\n');
        }
        else normalize.push_back(c);

    outData.swap(normalize);
    return true;
}

bool FileSystem::writeFile(const std::string& path, const std::string& data)
{
    const std::string p = normalizePath(path);
    if (p.empty()) return false;

    auto slash = p.find_last_of('/');
    if (slash != std::string::npos)
    {
        const std::string dir = p.substr(0, slash);
        if (!ensureDir(dir)) return false;
    }

    std::string temp = p + ".tmp";
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

    if (rename(temp.c_str(), p.c_str()) != 0)
    {
        remove(temp.c_str());
        return false;
    }
    return true;
}
