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
    RemoteCall::exportAs("thirst", "setShowThirst", set_show_thirst);
    RemoteCall::exportAs(
        "thirst",
        "registerOnTickCallback",
        [](const std::string& uuid, const std::string& ns, const std::string& fn) -> std::string {
            static std::size_t unregister_callback_idx = 0;
            try {
                auto unregister_callback = register_on_tick_callback(uuid, RemoteCall::importAs<void()>(ns, fn));
                if (unregister_callback) {
                    RemoteCall::exportAs("thirst", "_" + std::to_string(unregister_callback_idx), unregister_callback);
                    return "_" + std::to_string(unregister_callback_idx++);
                } else {
                    throw std::runtime_error("unable to register callback");
                }
            } catch (std::exception& e) {
                mod::Mod::getInstance().getSelf().getLogger().error("{}", e.what());
                return "";
            }
        }
    );
}