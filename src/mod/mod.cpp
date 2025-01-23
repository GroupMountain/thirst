#include "mod/mod.h"

#include "ll/api/mod/RegisterHelper.h"
#include "nlohmann/json.hpp"

void init();
void deinit();

namespace mod {

Mod& Mod::getInstance() {
    static Mod instance;
    return instance;
}

bool Mod::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool Mod::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    init();
    return true;
}

bool Mod::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    deinit();
    return true;
}

} // namespace mod

LL_REGISTER_MOD(mod::Mod, mod::Mod::getInstance());
#include "ll/api/Config.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ServerThreadExecutor.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/deps/core/utility/MCRESULT.h" // IWYU pragma: keep
#include "mc/nbt/CompoundTag.h"            // IWYU pragma: keep
#include "mc/network/packet/SetTitlePacket.h"
#include "mc/platform/UUID.h" // IWYU pragma: keep
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/util/MolangScriptArg.h"
#include "mc/util/MolangVariable.h"
#include "mc/util/MolangVariableMap.h"
#include "mc/world/Minecraft.h"
#include "mc/world/level/BlockSource.h" // IWYU pragma: keep
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/phys/HitResult.h" // IWYU pragma: keep


#include <charconv>
#include <ranges> // IWYU pragma: keep

struct Modification {
    int                      value;
    std::vector<std::string> commands;
};
std::unordered_map<void*, int>                            current_thirsts;
std::unordered_map<std::string, Modification>             modifications;
std::unordered_map<std::string, std::vector<std::string>> commands;
std::unordered_map<int, std::string>                      texts;
int                                                       thirst_base;
int                                                       thirst_max;
int                                                       thirst_min;
int                                                       thirst_tick;

bool exec_molang(Player* player, const std::string& script) {
    player->getMolangVariables().setMolangVariable(
        "variable.current_thirst",
        static_cast<float>(current_thirsts[player])
    );

    auto res = player->evalMolang(script) != 0.0;

    current_thirsts[player] = static_cast<int>(
        player->getMolangVariables()
            .getMolangVariable(HashedString{"variable.current_thirst"}.getHash(), "variable.current_thirst")
            .mPOD.mFloat
    );
    return res;
}

nlohmann::json& data() {
    static nlohmann::json data = [] {
        auto dir = mod::Mod::getInstance().getSelf().getDataDir();
        if (!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);
        if (!std::filesystem::exists(dir / "data.json")) std::ofstream(dir / "data.json") << "{}";
        return nlohmann::json::parse(std::ifstream(dir / "data.json"), nullptr, true, false);
    }();
    return data;
}

nlohmann::json& set_data(const std::string& uuid, int thirst) {
    data()[uuid] = thirst;
    return data();
}

void load_config() {
    struct {
        int                                                       version       = 0;
        std::unordered_map<std::string, Modification>             modifications = {};
        std::unordered_map<std::string, std::vector<std::string>> commands      = {};
        std::unordered_map<std::string, std::string>              texts         = {};
        int                                                       thirst_base   = 20;
        int                                                       thirst_max    = 20;
        int                                                       thirst_min    = 0;
        int                                                       thirst_tick   = 20;

    } config;
    auto dir = mod::Mod::getInstance().getSelf().getConfigDir();
    if (!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);
    if (!std::filesystem::exists(dir / "config.json")) ll::config::saveConfig(config, dir / "config.json");
    ll::config::loadConfig(config, dir / "config.json");
    modifications = config.modifications;
    commands      = config.commands;
    std::transform(config.texts.begin(), config.texts.end(), std::inserter(texts, texts.end()), [](auto&& val) {
        int thirst;
        std::from_chars(val.first.data(), val.first.data() + val.first.size(), thirst);
        return std::make_pair(thirst, val.second);
    });
    thirst_base = config.thirst_base;
    thirst_max  = config.thirst_max;
    thirst_min  = config.thirst_min;
    thirst_tick = config.thirst_tick;
}
struct EatEvent final : ll::event::PlayerEvent {
    ItemStack& item;
    constexpr explicit EatEvent(Player& player, ItemStack& item) : ll::event::PlayerEvent(player), item(item) {}
};
class EatEventEmitter : public ll::event::Emitter<[](auto&&...) { return nullptr; }, EatEvent> {};
LL_AUTO_TYPE_INSTANCE_HOOK(PlayerEatHook, HookPriority::Normal, Player, &Player::eat, void, ItemStack const& instance) {
    ll::event::EventBus::getInstance().publish(EatEvent{*this, const_cast<ItemStack&>(instance)});
    origin(instance);
}
void show_text(Player* player) {
    auto           val = current_thirsts[player];
    SetTitlePacket packet{SetTitlePacket::TitleType::Actionbar, texts[val], std::nullopt};
    packet.mFadeInTime  = 0;
    packet.mFadeOutTime = INT_MAX;
    packet.mStayTime    = INT_MAX;
    player->sendNetworkPacket(packet);
}
void init() {
    data();
    load_config();
    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerJoinEvent>([](auto& event) {
        void* player = std::addressof(event.self());
        if (data().contains(event.self().getUuid().asString()))
            current_thirsts[player] = data()[event.self().getUuid().asString()];
        else current_thirsts[player] = thirst_base;
        ll::coro::keepThis([player]() -> ll::coro::CoroTask<> {
            while (ll::getGamingStatus() == ll::GamingStatus::Running && current_thirsts.contains(player)) {
                co_await ll::chrono::ticks(thirst_tick);
                if (!current_thirsts.contains(player)) continue;
                for (const auto& [molang, cmds] : commands) {
                    if (exec_molang(static_cast<Player*>(player), molang))
                        for (const auto& cmd : cmds) {
                            CommandContext context = CommandContext(
                                cmd,
                                std::make_unique<PlayerCommandOrigin>(*static_cast<Player*>(player)),
                                CommandVersion::CurrentVersion()
                            );
                            ll::service::getMinecraft()->getCommands().executeCommand(context, false);
                        }
                }
                show_text(static_cast<Player*>(player));
                if (auto& val = current_thirsts[player]; val > thirst_min) val--;
                if (current_thirsts[player] > thirst_max) current_thirsts[player] = thirst_max;
                if (current_thirsts[player] < thirst_min) current_thirsts[player] = thirst_min;
            }
            co_return;
        }).launch(ll::thread::ServerThreadExecutor::getDefault());
    });
    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerDisconnectEvent>([](auto& event) {
        auto val = current_thirsts[std::addressof(event.self())];
        current_thirsts.erase(std::addressof(event.self()));
        auto uuidstr = event.self().getUuid().asString();
        set_data(uuidstr, val);
    });
    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerUseItemEvent>([](auto& event) {
        if (event.item().getComponent("minecraft:food")) return;

        if (auto hitResult = event.self().traceRay(5.5f, false, true); hitResult.mIsHitLiquid) {
            auto& block = event.self().getDimensionBlockSource().getBlock(hitResult.mLiquid);
            auto  name  = block.getTypeName();
            if (modifications.contains(name)) {
                current_thirsts[std::addressof(event.self())] += modifications[name].value;
                for (const auto& cmd : modifications[name].commands) {
                    CommandContext context = CommandContext(
                        cmd,
                        std::make_unique<PlayerCommandOrigin>(event.self()),
                        CommandVersion::CurrentVersion()
                    );
                    ll::service::getMinecraft()->getCommands().executeCommand(context, false);
                }
            } else {
                name = name.substr(name.find(":") + 1);
                if (modifications.contains(name)) {
                    current_thirsts[std::addressof(event.self())] += modifications[name].value;
                    for (const auto& cmd : modifications[name].commands) {
                        CommandContext context = CommandContext(
                            cmd,
                            std::make_unique<PlayerCommandOrigin>(event.self()),
                            CommandVersion::CurrentVersion()
                        );
                        ll::service::getMinecraft()->getCommands().executeCommand(context, false);
                    }
                }
            }
        }
        auto name = event.item().getTypeName();
        if (modifications.contains(name)) {
            current_thirsts[std::addressof(event.self())] += modifications[name].value;
            for (const auto& cmd : modifications[name].commands) {
                CommandContext context = CommandContext(
                    cmd,
                    std::make_unique<PlayerCommandOrigin>(event.self()),
                    CommandVersion::CurrentVersion()
                );
                ll::service::getMinecraft()->getCommands().executeCommand(context, false);
            }
            const_cast<ItemStack*>(event.self().getAllHand()[0])->remove(1);
        } else {
            name = name.substr(name.find(":") + 1);
            if (modifications.contains(name)) {
                current_thirsts[std::addressof(event.self())] += modifications[name].value;
                for (const auto& cmd : modifications[name].commands) {
                    CommandContext context = CommandContext(
                        cmd,
                        std::make_unique<PlayerCommandOrigin>(event.self()),
                        CommandVersion::CurrentVersion()
                    );
                    ll::service::getMinecraft()->getCommands().executeCommand(context, false);
                }
                const_cast<ItemStack*>(event.self().getAllHand()[0])->remove(1);
            } else {
                return;
            }
        }
        if (current_thirsts[std::addressof(event.self())] > thirst_max)
            current_thirsts[std::addressof(event.self())] = thirst_max;
        if (current_thirsts[std::addressof(event.self())] < thirst_min)
            current_thirsts[std::addressof(event.self())] = thirst_min;
    });
    ll::event::EventBus::getInstance().emplaceListener<EatEvent>([](auto& event) {
        auto name = event.item.getTypeName();
        if (modifications.contains(name)) {
            current_thirsts[std::addressof(event.self())] += modifications[name].value;
            for (const auto& cmd : modifications[name].commands) {
                CommandContext context = CommandContext(
                    cmd,
                    std::make_unique<PlayerCommandOrigin>(event.self()),
                    CommandVersion::CurrentVersion()
                );
                ll::service::getMinecraft()->getCommands().executeCommand(context, false);
            }
        } else {
            name = name.substr(name.find(":") + 1);
            if (modifications.contains(name)) {
                current_thirsts[std::addressof(event.self())] += modifications[name].value;
                for (const auto& cmd : modifications[name].commands) {
                    CommandContext context = CommandContext(
                        cmd,
                        std::make_unique<PlayerCommandOrigin>(event.self()),
                        CommandVersion::CurrentVersion()
                    );
                    ll::service::getMinecraft()->getCommands().executeCommand(context, false);
                }
            } else {
                return;
            }
        }
        if (current_thirsts[std::addressof(event.self())] > thirst_max)
            current_thirsts[std::addressof(event.self())] = thirst_max;
        if (current_thirsts[std::addressof(event.self())] < thirst_min)
            current_thirsts[std::addressof(event.self())] = thirst_min;
    });
}
void deinit() {
    for (auto [player, value] : current_thirsts) {
        set_data(static_cast<const Player*>(player)->getUuid().asString(), value);
    }
    std::ofstream(mod::Mod::getInstance().getSelf().getDataDir() / "data.json") << data();
}