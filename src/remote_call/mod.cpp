#include "remote_call/mod.h"


#include "ll/api/mod/RegisterHelper.h"
void export_api();
namespace mod {

Mod& Mod::getInstance() {
    static Mod instance;
    return instance;
}

bool Mod::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    export_api();
    return true;
}

bool Mod::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    return true;
}

bool Mod::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace mod

LL_REGISTER_MOD(mod::Mod, mod::Mod::getInstance());

#include <RemoteCallAPI.h>

void export_api() {
    RemoteCall::exportAs("thirst", "getThirst", get_thirst);
    RemoteCall::exportAs("thirst", "setThirst", set_thirst);
}