#include "PCH.h"
#include "EconomyManager.h"
#include "SerializationManager.h"

namespace RecruitmentHandler { 
    void Install(); 
    void Update();
}

/**
 * SKSE Plugin Information
 */
SKSEPluginInfo(
    .Version = { 1, 0, 0, 0 },
    .Name = "NoFreeService",
    .Author = "Antigravity",
    .SupportEmail = "",
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
)

/**
 * Hook for Player Update Loop
 * This is much more reliable than RE::Main in AE.
 */
struct PlayerUpdateHook
{
    static void Hook_Update(RE::PlayerCharacter* a_this, float a_delta)
    {
        func(a_this, a_delta);
        RecruitmentHandler::Update();
    }

    static inline REL::Relocation<decltype(Hook_Update)> func;
};

void InitializeLogging()
{
    auto path = logger::log_directory();
    if (!path) return;

    *path /= "NoFreeService.log"sv;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v"s);

    logger::info("No Free Service v1.0.0 initialized");
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitializeLogging();
    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(128);

    RecruitmentHandler::Install();
    SerializationManager::Register();

    // Hook PlayerCharacter::Update (Index 0xAD / 173)
    REL::Relocation<std::uintptr_t> playerVTable{ RE::VTABLE_PlayerCharacter[0] };
    PlayerUpdateHook::func = playerVTable.write_vfunc(0xAD, PlayerUpdateHook::Hook_Update);

    logger::info("No Free Service: Player update hook installed at index 0xAD.");

    return true;
}
