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

        // 1. Zaten takım arkadaşıysa kesinlikle takipçidir
        if (a_actor->IsPlayerTeammate()) return true;

        // 2. Standart ve Genişletilmiş Takipçi Faction ID'leri
        static const RE::FormID followerFactions[] = {
            0x0005C84E, // CurrentFollowerFaction
            0x000CB7DF, // WIFollowerFaction
            0x0005C84C, // PotentialFollowerFaction
            0x000B2D6D, // PotentialHirelingFaction
            0x00019C8E  // PlayerFollowerFaction
        };

        for (auto id : followerFactions) {
            auto* faction = RE::TESForm::LookupByID<RE::TESFaction>(id);
            if (faction && a_actor->IsInFaction(faction)) return true;
        }

        // 3. Modlu Takipçiler İçin İsim Taraması (Esnek Kontrol)
        // NPC'nin dahil olduğu tüm faction'ları gez ve isminde "Follower" geçen var mı bak
        auto* npcBase = a_actor->GetActorBase();
        auto* npc = npcBase ? npcBase->As<RE::TESNPC>() : nullptr;
        if (npc) {
            for (auto& factionInfo : npc->factions) {
                if (factionInfo.faction) {
                    std::string fName = factionInfo.faction->GetFullName();
                    std::transform(fName.begin(), fName.end(), fName.begin(), ::tolower);
                    if (fName.find("follower") != std::string::npos || fName.find("hireling") != std::string::npos) {
                        logger::info("IsPotentialFollower: {} matched via faction name: {}", a_actor->GetName(), fName);
                        return true;
                    }
                }
            }
        }

        // 4. İlişki Seviyesi Kontrolü (Fallback)
        // Eğer oyuncuyla dostluğu varsa takipçi olma ihtimali yüksektir
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player && a_actor->GetRelationshipRank(player) > 0) {
            return true;
        }

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

    bool HasBeenPaid(RE::Actor* a_actor) { 
        return a_actor && paidFollowers.count(a_actor->formID) && paidFollowers[a_actor->formID]; 
    }
    
    void SetPaid(RE::Actor* a_actor) { 
        if (a_actor) paidFollowers[a_actor->formID] = true; 
    }
    
    void ClearPaid(RE::Actor* a_actor) { 
        if (a_actor) {
            paidFollowers[a_actor->formID] = false;
            paymentDays.erase(a_actor->formID);
        }
    }

    int32_t CalculateWeeklyWage(RE::Actor* a_actor) { 
        return static_cast<int32_t>(CalculateRecruitmentCost(a_actor) * 0.20f); 
    }

    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day) {
        if (a_actor) paymentDays[a_actor->formID] = a_day;
    }

    float GetLastPaymentDay(RE::Actor* a_actor) { 
        if (a_actor && paymentDays.count(a_actor->formID)) return paymentDays[a_actor->formID];
        return 0.0f;
    }

    std::map<RE::FormID, bool>& GetPaidMap() { return paidFollowers; }
    std::map<RE::FormID, float>& GetPaymentDayMap() { return paymentDays; }
}
