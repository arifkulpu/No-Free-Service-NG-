#include "PCH.h"
#include "Settings.h"
#include "RecruitmentHandler.h"
#include <RE/Skyrim.h>
#include <map>
#include <vector>

namespace EconomyManager
{
    struct PaidStatus {
        bool isPaid = false;
        float lastPaymentDay = 0.0f;
        std::chrono::steady_clock::time_point realTimePaidAt;
        bool wasTeammate = false;
        std::chrono::steady_clock::time_point dismissedAt;
        bool hasDismissedTime = false;
        bool notified30 = false;
        bool notified20 = false;
        bool notified10 = false;
    };

    static std::map<RE::FormID, PaidStatus> paidFollowers;

    static bool IsHousecarl(RE::Actor* a_actor) {
        if (!a_actor) return false;
        auto* base = a_actor->GetActorBase();
        if (!base) return false;
        auto* npc = base->As<RE::TESNPC>();
        if (npc && npc->npcClass) {
            std::string className = npc->npcClass->GetFormEditorID();
            if (className.find("Housecarl") != std::string::npos) return true;
        }
        return false;
    }

    bool IsPotentialFollower(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsDead() || a_actor->IsPlayerRef()) return false;
        if (a_actor->IsGuard() || a_actor->IsChild()) return false;
        
        auto* base = a_actor->GetActorBase();
        if (!base) return false;

        // Özel Karakter Filtreleri
        if (a_actor->GetActorRuntimeData().vendorFaction != nullptr) return false;
        if (IsHousecarl(a_actor)) return false;

        // 1. Standart Takipçi Faction'ları (Modlu takipçilerin %99'u buna dahildir)
        static auto* potentialFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
        static auto* currentFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x00017433);
        if (potentialFaction && a_actor->IsInFaction(potentialFaction)) return true;
        if (currentFaction && a_actor->IsInFaction(currentFaction)) return true;

        // 2. İlişki Kontrolü (Daha önce konuştuğumuz "sorunlu kodu" burada güvenli hale getirdik)
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player && RecruitmentHandler::GetRank(a_actor, player) > 0) return true;

        // 3. Mod Karakterleri için Özel İstisna: 
        // Eğer karakter bir moddan geliyorsa VE "Unique" ise (modlu takipçilerin çoğu unique'dir)
        uint8_t modIdx = (base->formID >> 24) & 0xFF;
        if (modIdx > 0x04 && base->IsUnique()) {
             // Burada Armion gibi karakterleri süzmek için ek bir "Essential" veya "VoiceType" kontrolü gerekebilir 
             // ama Unique olması genellikle takipçi adayları için yeterlidir.
             return true; 
        }

        return false;
    }

    int32_t CalculateRecruitmentCost(RE::Actor* a_actor) {
        if (!a_actor) return Settings::RecruitmentBaseCost;
        return Settings::RecruitmentBaseCost + (a_actor->GetLevel() * Settings::RecruitmentLevelMultiplier);
    }

    void SetPaid(RE::Actor* a_actor) {
        if (a_actor) {
            logger::info("SetPaid: Actor {:X} ({}) marked as paid. Cost: {}", 
                a_actor->formID, a_actor->GetName(), CalculateRecruitmentCost(a_actor));
            auto& status = paidFollowers[a_actor->formID];
            status.isPaid = true;
            status.realTimePaidAt = std::chrono::steady_clock::now();
            status.wasTeammate = a_actor->IsPlayerTeammate();
            status.hasDismissedTime = false;
            status.notified30 = false;
            status.notified20 = false;
            status.notified10 = false;
        }
    }

    void ClearPaid(RE::Actor* a_actor) {
        if (a_actor) paidFollowers.erase(a_actor->formID);
    }

    bool HasBeenPaid(RE::Actor* a_actor) {
        if (!a_actor) return false;
        auto it = paidFollowers.find(a_actor->formID);
        return (it != paidFollowers.end() && it->second.isPaid);
    }

    bool IsInGracePeriod(RE::Actor* a_actor) {
        return HasBeenPaid(a_actor);
    }

    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day) {
        if (a_actor) paidFollowers[a_actor->formID].lastPaymentDay = a_day;
    }

    float GetLastPaymentDay(RE::Actor* a_actor) {
        if (!a_actor) return 0.0f;
        auto it = paidFollowers.find(a_actor->formID);
        return (it != paidFollowers.end()) ? it->second.lastPaymentDay : 0.0f;
    }

    std::map<RE::FormID, bool> GetPaidMap() {
        std::map<RE::FormID, bool> result;
        for (auto const& [id, status] : paidFollowers) result[id] = status.isPaid;
        return result;
    }

    std::map<RE::FormID, float> GetPaymentDayMap() {
        std::map<RE::FormID, float> result;
        for (auto const& [id, status] : paidFollowers) result[id] = status.lastPaymentDay;
        return result;
    }

    void SetPaidFromLoad(RE::FormID a_id, bool a_paid) {
        auto& status = paidFollowers[a_id];
        status.isPaid = a_paid;
        status.realTimePaidAt = std::chrono::steady_clock::now();
        status.wasTeammate = false;
        status.hasDismissedTime = false;
        status.notified30 = false;
        status.notified20 = false;
        status.notified10 = false;
    }

    void SetPaymentDayFromLoad(RE::FormID a_id, float a_day) {
        paidFollowers[a_id].lastPaymentDay = a_day;
    }

    void UpdateFollowerPaymentStates() {
        auto now = std::chrono::steady_clock::now();
        for (auto& [id, status] : paidFollowers) {
            if (!status.isPaid) {
                continue;
            }

            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            logger::info("UpdateFollowerPaymentStates: Checking actor ID {:X}, found pointer: {}", id, (void*)actor);
            if (actor) {
                bool isTeammate = actor->IsPlayerTeammate();
                if (isTeammate) {
                    if (!status.wasTeammate) {
                        logger::info("UpdateFollowerPaymentStates: Actor {:X} ({}) detected as player teammate.", id, actor->GetName());
                    }
                    status.wasTeammate = true;
                    status.hasDismissedTime = false;
                } else {
                    if (status.wasTeammate) {
                        logger::info("UpdateFollowerPaymentStates: Actor {:X} ({}) was teammate but is now dismissed. Starting grace period timer.", id, actor->GetName());
                        status.wasTeammate = false;
                        status.dismissedAt = now;
                        status.hasDismissedTime = true;
                        status.notified30 = false;
                        status.notified20 = false;
                        status.notified10 = false;
                    }
                }

                // Expiration and Notification checks (only runs if actor is valid and not a teammate)
                if (!isTeammate) {
                    long long elapsed = 0;
                    if (status.hasDismissedTime) {
                        elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - status.dismissedAt).count();
                    } else {
                        elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - status.realTimePaidAt).count();
                    }

                    long long remaining = Settings::GracePeriodDuration - elapsed;

                    if (remaining <= 30 && remaining > 20 && !status.notified30) {
                        char buf[256];
                        if (status.hasDismissedTime) {
                            snprintf(buf, sizeof(buf), "[%s] Free re-hire window closes in 30 seconds!", actor->GetName());
                        } else {
                            snprintf(buf, sizeof(buf), "[%s] Recruitment window closes in 30 seconds!", actor->GetName());
                        }
                        RE::DebugNotification(buf);
                        status.notified30 = true;
                    } else if (remaining <= 20 && remaining > 10 && !status.notified20) {
                        char buf[256];
                        snprintf(buf, sizeof(buf), "[%s] 20 seconds remaining!", actor->GetName());
                        RE::DebugNotification(buf);
                        status.notified20 = true;
                    } else if (remaining <= 10 && remaining > 0 && !status.notified10) {
                        char buf[256];
                        snprintf(buf, sizeof(buf), "[%s] 10 seconds remaining!", actor->GetName());
                        RE::DebugNotification(buf);
                        status.notified10 = true;
                    }

                    if (remaining <= 0) {
                        if (status.hasDismissedTime) {
                            logger::info("UpdateFollowerPaymentStates: Actor {:X} ({}) dismissal grace period expired. Clearing paid status.", id, actor->GetName());
                        } else {
                            logger::info("UpdateFollowerPaymentStates: Actor {:X} ({}) payment validity expired (never recruited). Clearing paid status.", id, actor->GetName());
                        }
                        status.isPaid = false;
                        status.hasDismissedTime = false;
                    }
                }
            }
        }
    }
}
