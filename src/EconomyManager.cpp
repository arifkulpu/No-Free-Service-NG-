#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <RE/A/Actor.h>
#include <map>
#include <algorithm>
#include <string>

namespace EconomyManager
{
    static std::map<RE::FormID, bool> paidFollowers;
    static std::map<RE::FormID, float> paymentDays;

    bool IsPotentialFollower(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsDisabled()) return false;
        if (a_actor->IsPlayerTeammate()) return true;

        auto* npcBase = a_actor->GetActorBase();
        auto* npc = npcBase ? npcBase->As<RE::TESNPC>() : nullptr;
        
        if (npc) {
            static const std::string keywords[] = { "Follower", "Hireling", "Ally" };
            for (const auto& kwStr : keywords) {
                if (a_actor->HasKeywordString(kwStr)) return true;
            }

            for (auto& factionInfo : npc->factions) {
                if (factionInfo.faction) {
                    RE::FormID fID = factionInfo.faction->formID;
                    if (fID == 0x0005C84E || fID == 0x000CB7DF || fID == 0x0005C84C || 
                        fID == 0x000B2D6D || fID == 0x00019C8E) return true;
                }
            }
        }

        auto* ui = RE::UI::GetSingleton();
        if (ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) return true;

        return false;
    }

    int32_t CalculateRecruitmentCost(RE::Actor* a_actor)
    {
        if (!a_actor) return 500;
        
        // Sadece Seviye Bazlı Hesaplama (En Güvenli Yol)
        int32_t level = static_cast<int32_t>(a_actor->GetLevel());
        if (level <= 0) level = 1;
        
        // (Seviye * 100) + 500 Gold
        int32_t cost = (level * 100) + 500;
        
        // Üst limit koyalım (Örn: Max 10.000 Gold)
        if (cost > 10000) cost = 10000;
        
        return cost;
    }

    bool HasBeenPaid(RE::Actor* a_actor) { return a_actor && paidFollowers.count(a_actor->formID) && paidFollowers[a_actor->formID]; }
    void SetPaid(RE::Actor* a_actor) { if (a_actor) paidFollowers[a_actor->formID] = true; }
    void ClearPaid(RE::Actor* a_actor) { if (a_actor) { paidFollowers[a_actor->formID] = false; paymentDays.erase(a_actor->formID); } }
    int32_t CalculateWeeklyWage(RE::Actor* a_actor) { return static_cast<int32_t>(CalculateRecruitmentCost(a_actor) * 0.20f); }
    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day) { if (a_actor) paymentDays[a_actor->formID] = a_day; }
    float GetLastPaymentDay(RE::Actor* a_actor) { if (a_actor && paymentDays.count(a_actor->formID)) return paymentDays[a_actor->formID]; return 0.0f; }
    std::map<RE::FormID, bool>& GetPaidMap() { return paidFollowers; }
    std::map<RE::FormID, float>& GetPaymentDayMap() { return paymentDays; }
}
