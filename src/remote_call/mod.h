#pragma once

#include "ll/api/mod/NativeMod.h"
__declspec(dllimport) int  get_thirst(const std::string& uuid);
__declspec(dllimport) void set_thirst(const std::string& uuid, int value);
__declspec(dllimport) void set_show_thirst(const std::string& uuid, bool value);
namespace mod {

class Mod {

public:
    static Mod& getInstance();

    Mod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace mod