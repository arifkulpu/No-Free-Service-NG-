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

                    std::string fName = factionInfo.faction->GetFullName();
                    if (!fName.empty()) {
                        std::transform(fName.begin(), fName.end(), fName.begin(), ::tolower);
                        if (fName.find("follower") != std::string::npos || fName.find("hireling") != std::string::npos) return true;
                    }
                }
            }
        }

        // RelationshipRank bu sürümde Actor üyesi olmadığı için atlanıyor
        auto* ui = RE::UI::GetSingleton();
        if (ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) return true;

        return false;
    }

    int32_t CalculateRecruitmentCost(RE::Actor* a_actor)
    {
        if (!a_actor) return 500;
        int32_t level = static_cast<int32_t>(a_actor->GetLevel());
        if (level <= 0) level = 1;
        float baseCost = (level * 100.0f) + 500.0f;
        float classMultiplier = 1.0f;
        try {
            auto* npcBase = a_actor->GetActorBase();
            auto* npc = npcBase ? npcBase->As<RE::TESNPC>() : nullptr;
            if (npc && npc->npcClass) {
                uintptr_t addr = reinterpret_cast<uintptr_t>(npc->npcClass);
                if (addr > 0x1000000 && addr < 0x00007FFFFFFFFFFF) { 
                    if (npc->npcClass->GetFormType() == RE::FormType::Class) {
                        uint8_t magicka = npc->npcClass->data.attributeWeights.magicka;
                        uint8_t health = npc->npcClass->data.attributeWeights.health;
                        uint8_t stamina = npc->npcClass->data.attributeWeights.stamina;
                        if (magicka > health && magicka > stamina) classMultiplier = 1.5f;
                    }
                }
            }
        } catch (...) {}
        return static_cast<int32_t>(baseCost * classMultiplier);
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
