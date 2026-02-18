#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Input
{
    enum class Key : uint16_t
    {
        Left, Right, Up, Down,
        Home, End,
        PageUp, PageDown,
        Enter, Backspace, Delete,
        Tab, Escape,
        A, B, Plus, Minus, One, Two
    };

    enum class Command : uint16_t
    {
        None = 0,
        Quit,
        ToggleFileBrowser,
        ToggleConsole,
        ToggleRuntimeView,
        OpenMenu,
        NewFile,
        OpenFile,
        SaveFile,
        SaveAs,
        Find,
        Replace,
        Undo,
        Redo,
        Run,
        Stop,
        ResetRuntime,
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
            Command,
        } type = Type::PointerMove;

        PointerState pointer = {};
        int scrollLines = 0;
        Key key = Key::Escape;
        uint32_t keyMods = 0, ch = 0;
        Command cmd = Command::None;
    };

    struct InputFrame
    {
        PointerState pointer = {};
        uint32_t wpadDown = 0, wpadHeld = 0, wpadUp = 0;
    };

    struct KeyRepeat
    {
        const double initialDelay = 0.35, repeatRate = 0.045;
        bool active = false;
        double t0 = 0.0, lastFire = 0.0;

        void reset()
        {
            active = false;
            t0 = 0.0;
            lastFire = 0.0;
        }

        bool update(const bool held, const double now)
        {
            if (!held)
            {
                reset();
                return false;
            }

            if (!active)
            {
                active = true;
                t0 = now;
                lastFire = now;

                return true;
            }

            if (now - t0 < initialDelay) return false;
            if (now - lastFire >= repeatRate)
            {
                lastFire = now;
                return true;
            }

            return false;
        }
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

    std::string appRoot();
    std::string workspaceRoot();

    bool ensureDir(const std::string& path);
    bool exists(const std::string& path);
    bool isDir(const std::string& path);

    bool listDir(const std::string& path, std::vector<DirEntry>& outEntries, bool sort = true);
    bool readFile(const std::string& path, std::string& outData);
    bool writeFile(const std::string& path, const std::string& data);
}
