#include "PCH.h"
#include "Settings.h"
#include <RE/Skyrim.h>
#include <map>
#include <chrono>
#include <string>
#include <algorithm>

namespace EconomyManager
{
    struct PaidStatus {
        bool isPaid = false;
        float lastPaymentDay = 0.0f;
        std::chrono::steady_clock::time_point realTimePaidAt;
    };

    static std::map<RE::FormID, PaidStatus> paidFollowers;

    bool IsPotentialFollower(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsPlayerRef()) return false;
        
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return false;

        // 1. Explicit Follower Factions
        static auto* followerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
        if (followerFaction && a_actor->IsInFaction(followerFaction)) return true;

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        auto* nffFaction = dataHandler ? dataHandler->LookupForm<RE::TESFaction>(0x00000800, "nwsFollowerFramework.esp") : nullptr;
        if (nffFaction && a_actor->IsInFaction(nffFaction)) return true;

        // 2. Current Teammate Status
        if (a_actor->IsPlayerTeammate()) return true;

        // 3. Filter out Merchants/Vendors
        if (a_actor->GetActorRuntimeData().vendorFaction != nullptr) return false;

        // 4. Voice Type Filtering
        auto* base = a_actor->GetActorBase();
        if (base && base->voiceType) {
            std::string voiceName = base->voiceType->GetFormEditorID();
            std::transform(voiceName.begin(), voiceName.end(), voiceName.begin(), ::tolower);

            static const std::vector<std::string> allowedVoices = {
                "femaleeventoned", "femaleyoungeager", "femalecommander", "femalesultry", "femalecondescending",
                "maleyoungeager", "maleeventoned", "maledrunk", "malecommander", "malebrute", "maleslycynical",
                "custom", "kakthu", "isis", "follower"
            };

            bool voiceMatch = false;
            for (const auto& v : allowedVoices) {
                if (voiceName.find(v) != std::string::npos) {
                    voiceMatch = true;
                    break;
                }
            }

            if (voiceMatch) {
                if (base->IsUnique() && !a_actor->IsHostileToActor(player)) {
                    return true;
                }
            }
        }

        return false;
    }

    int32_t CalculateRecruitmentCost(RE::Actor* a_actor)
    {
        if (!a_actor) return Settings::RecruitmentBaseCost;
        int32_t level = a_actor->GetLevel();
        return Settings::RecruitmentBaseCost + (level * Settings::RecruitmentLevelMultiplier);
    }

    void SetPaid(RE::Actor* a_actor)
    {
        if (a_actor) {
            auto& status = paidFollowers[a_actor->formID];
            status.isPaid = true;
            status.realTimePaidAt = std::chrono::steady_clock::now();
        }
    }

    void ClearPaid(RE::Actor* a_actor)
    {
        if (a_actor) {
            paidFollowers.erase(a_actor->formID);
        }
    }

    bool HasBeenPaid(RE::Actor* a_actor)
    {
        if (!a_actor) return false;
        auto it = paidFollowers.find(a_actor->formID);
        if (it != paidFollowers.end()) {
            return it->second.isPaid;
        }
        return false;
    }

    bool IsInGracePeriod(RE::Actor* a_actor)
    {
        if (!a_actor) return false;
        auto it = paidFollowers.find(a_actor->formID);
        if (it != paidFollowers.end() && it->second.isPaid) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.realTimePaidAt).count();
            return elapsed < Settings::GracePeriodDuration;
        }
        return false;
    }

    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day)
    {
        if (a_actor) {
            paidFollowers[a_actor->formID].lastPaymentDay = a_day;
        }
    }

    float GetLastPaymentDay(RE::Actor* a_actor)
    {
        if (!a_actor) return 0.0f;
        auto it = paidFollowers.find(a_actor->formID);
        return (it != paidFollowers.end()) ? it->second.lastPaymentDay : 0.0f;
    }

    std::map<RE::FormID, bool> GetPaidMap()
    {
        std::map<RE::FormID, bool> result;
        for (auto const& [id, status] : paidFollowers) {
            result[id] = status.isPaid;
        }
        return result;
    }

    std::map<RE::FormID, float> GetPaymentDayMap()
    {
        std::map<RE::FormID, float> result;
        for (auto const& [id, status] : paidFollowers) {
            result[id] = status.lastPaymentDay;
        }
        return result;
    }

    void SetPaidFromLoad(RE::FormID a_id, bool a_paid)
    {
        paidFollowers[a_id].isPaid = a_paid;
        paidFollowers[a_id].realTimePaidAt = std::chrono::steady_clock::now();
    }

    void SetPaymentDayFromLoad(RE::FormID a_id, float a_day)
    {
        paidFollowers[a_id].lastPaymentDay = a_day;
    }
}
