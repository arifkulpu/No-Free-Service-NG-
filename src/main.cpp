#include "PCH.h"
#include "EconomyManager.h"
#include "RecruitmentHandler.h"
#include "Settings.h"
#include "SerializationManager.h"

// AE için Versiyon Verisi
extern "C" __declspec(dllexport) constinit SKSE::PluginVersionData SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion(REL::Version{ 1, 0, 0, 0 });
    v.PluginName("NoFreeService");
    v.AuthorName("Antigravity");
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST_AE });
    return v;
}();

void InitializeLogging()
{
    auto path = SKSE::log::log_directory();
    if (!path) return;

    *path /= "NoFreeService.log";

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] %v");
    
    SKSE::log::info("Logging initialized - V3.");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
        Settings::Load();
        RecruitmentHandler::Install();
    }
}

struct PlayerUpdateHook
{
    static void Hook_Update(RE::PlayerCharacter* a_this, float a_delta)
    {
        func(a_this, a_delta);
        RecruitmentHandler::Update();
    }
    static inline REL::Relocation<decltype(Hook_Update)> func;
};

void InitializeHook()
{
    REL::Relocation<std::uintptr_t> playerVTable{ RE::VTABLE_PlayerCharacter[0] };
    PlayerUpdateHook::func = playerVTable.write_vfunc(0xAD, PlayerUpdateHook::Hook_Update);
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    InitializeLogging();
    SKSE::log::info("Plugin Loading...");

    SKSE::Init(a_skse);

    auto* messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener(MessageHandler);
    }

    SerializationManager::Register();

    InitializeHook();
    SKSE::log::info("Plugin Loaded Successfully.");

    return true;
}
