#pragma once

#include <string>
#include <vector>
#include <array>

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

    enum class KeyMods : uint8_t
    {
        None = 0,
        ContextMenu = 1 << 0,
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
            Pointer,
            Scroll,
            KeyDown,
            KeyUp,
        } type = Type::Pointer;

        PointerState pointer = {};
        int scrollX = 0, scrollY = 0;
        Key key = Key::None;
        KeyMods mods = KeyMods::None;
    };

    struct InputFrame
    {
        PointerState pointer = {};
        uint32_t wpadDown = 0, wpadHeld = 0, wpadUp = 0;
    };

    class KeyRepeat
    {
    public:
        void generate(const InputFrame& frame, double dt, std::vector<InputEvent>& outEvents);

    private:
        struct State
        {
            double heldTime = 0.0f, nextTime = 0.0f;
            bool wasHeld = false;
        };

        std::array<State, static_cast<size_t>(Key::StickDown) + 1> keyStates = {};
        void repeatKey(double dt, std::vector<InputEvent>& outEvents, Key key, bool held);
    };

    bool init();
    void exit();
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

    std::string normalize(std::string path);
    std::string join(const std::string& a, const std::string& b);

    bool listDir(const std::string& path, std::vector<DirEntry>& outEntries, bool sort = true);
    bool readFile(const std::string& path, std::vector<uint8_t>& outData);
    bool writeFile(const std::string& path, const std::vector<uint8_t>& data);

    bool makeDir(const std::string& path);
    bool renamePath(const std::string& from, const std::string& to);
    bool copyPath(const std::string& from, const std::string& to);
    bool removePath(const std::string& path);

    inline std::string appRoot = "sd:/apps/WiiScript/", workspaceRoot = "sd:/WiiScript/";
}
