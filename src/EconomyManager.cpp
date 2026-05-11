#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <RE/A/Actor.h>
#include <map>
#include <algorithm>

namespace EconomyManager
{
    static std::map<RE::FormID, bool> paidFollowers;
    static std::map<RE::FormID, float> paymentDays;

    bool IsPotentialFollower(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsDisabled()) return false;

        // 1. Standart Takipçi Faction'ları
        static const RE::FormID followerFactions[] = {
            0x0005C84E, // CurrentFollowerFaction
            0x000CB7DF, // WIFollowerFaction
            0x0005C84C, // PotentialFollowerFaction
            0x000B2D6D  // PotentialHirelingFaction
        };

        for (auto id : followerFactions) {
            auto* faction = RE::TESForm::LookupByID<RE::TESFaction>(id);
            if (faction && a_actor->IsInFaction(faction)) {
                logger::info("IsPotentialFollower: {} is in follower faction {:X}", a_actor->GetName(), id);
                return true;
            }
        }

        if (a_actor->IsPlayerTeammate()) {
            logger::info("IsPotentialFollower: {} is a PlayerTeammate", a_actor->GetName());
            return true;
        }

        logger::info("IsPotentialFollower: {} does not match any follower criteria.", a_actor->GetName());
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
                auto* npcClass = npc->npcClass;
                uintptr_t addr = reinterpret_cast<uintptr_t>(npcClass);

                // ULTRA SAFETY: 0x1000000 altındaki adresler bozuktur.
                if (addr > 0x1000000 && addr < 0x00007FFFFFFFFFFF) { 
                    if (npcClass->GetFormType() == RE::FormType::Class) {
                        uint8_t magicka = npcClass->data.attributeWeights.magicka;
                        uint8_t health = npcClass->data.attributeWeights.health;
                        uint8_t stamina = npcClass->data.attributeWeights.stamina;

                        if (magicka > health && magicka > stamina) {
                            classMultiplier = 1.5f;
                        }
                    }
                } else {
                    logger::warn("Suspicious npcClass pointer detected: {:X} for NPC: {}", addr, a_actor->GetName());
                }
            }
        } catch (...) {
            // Hata durumunda varsayılanla devam
        }

        float relationshipMultiplier = 1.0f;
        // GetRelationshipRank bu sürümde Actor üyesi değilse geçici olarak devre dışı
        return static_cast<int32_t>(baseCost * classMultiplier * relationshipMultiplier);
    }

    bool HasBeenPaid(RE::Actor* a_actor) { 
        return a_actor && paidFollowers.count(a_actor->formID) && paidFollowers[a_actor->formID]; 
    }
    
    void SetPaid(RE::Actor* a_actor) { 
        if (a_actor) {
            paidFollowers[a_actor->formID] = true;
            logger::info("Marked as paid: {} ({:X})", a_actor->GetName(), a_actor->formID);
        }
    }
    
    void ClearPaid(RE::Actor* a_actor) { 
        if (a_actor) {
            paidFollowers[a_actor->formID] = false;
            paymentDays.erase(a_actor->formID);
            logger::info("Payment cleared: {}", a_actor->GetName());
        }
    }

    int32_t CalculateWeeklyWage(RE::Actor* a_actor) { 
        return static_cast<int32_t>(CalculateRecruitmentCost(a_actor) * 0.20f); 
    }

    bool IsExempt(RE::Actor* a_actor) { return false; }

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
