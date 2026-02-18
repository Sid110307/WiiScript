#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Input
{
    enum class Key : uint16_t
    {
        None = 0,
        Home,
        Up,
        Down,
        Left,
        Right,
        A,
        B,
        Plus,
        Minus,
        One,
        Two,
        C,
        Z,
        StickLeft,
        StickRight,
        StickUp,
        StickDown,
    };

    struct PointerState
    {
        bool valid = false;
        float x = 0, y = 0;
    };

    struct InputEvent
    {
        enum class Type : uint8_t
        {
            PointerMove,
            PointerDown,
            PointerUp,
            Scroll,
            KeyDown,
            KeyUp,
            TextChar,
        } type = Type::PointerMove;

        PointerState pointer = {};
        int scrollLines = 0;
        Key key = Key::None;
        uint32_t keyMods = 0, ch = 0;
    };

    struct InputFrame
    {
        PointerState pointer = {};
        uint32_t wpadDown = 0, wpadHeld = 0, wpadUp = 0;
    };

    bool init();
    void poll(InputFrame* outFrame, std::vector<InputEvent>& outEvents);
}

namespace Time
{
    bool init();
    uint64_t ticks();
    double seconds();
}

namespace FileSystem
{
    struct DirEntry
    {
        std::string name, path;
        bool isDir = false;
        uint64_t size = 0;
    };

    bool init();
    bool isReady();

    bool ensureDir(const std::string& path);
    bool exists(const std::string& path);
    bool isDir(const std::string& path);

    bool listDir(const std::string& path, std::vector<DirEntry>& outEntries, bool sort = true);
    bool readFile(const std::string& path, std::string& outData);
    bool writeFile(const std::string& path, const std::string& data);

    inline std::string appRoot = "sd:/apps/WiiScript/", workspaceRoot = "sd:/WiiScript/";
}
