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
        if (!a_actor || a_actor->IsDisabled() || !a_actor->Is3DLoaded()) return false;

        // 1. Standart Takipçi Faction'ları
        static const RE::FormID followerFactions[] = {
            0x0005C84E, // CurrentFollowerFaction
            0x000CB7DF, // WIFollowerFaction
            0x0005C84C, // PotentialFollowerFaction
            0x000B2D6D  // PotentialHirelingFaction
        };

        for (auto id : followerFactions) {
            auto* faction = RE::TESForm::LookupByID<RE::TESFaction>(id);
            if (faction && a_actor->IsInFaction(faction)) return true;
        }

        // 2. Modlu Takipçiler İçin Esnek Kontrol: 
        // Eğer NPC zaten teammate ise veya "Follower" isminde bir keyword/faction içeriyorsa
        if (a_actor->IsPlayerTeammate()) return true;

        return false;
    }

    int32_t CalculateRecruitmentCost(RE::Actor* a_actor)
    {
        if (!a_actor) return 500;

        // Temel Ücret: (Seviye * 100) + 500
        int32_t level = static_cast<int32_t>(a_actor->GetLevel());
        if (level <= 0) level = 1;
        float baseCost = (level * 100.0f) + 500.0f;

        // Sınıf Bazlı Çarpan
        float classMultiplier = 1.0f;
        auto* actorBase = a_actor->GetActorBase();
        auto* npcClass = actorBase ? actorBase->npcClass : nullptr;
        
        if (npcClass) {
            // Eğer NPC'nin sınıfı Magicka ağırlıklıysa büyücü kabul et
            uint8_t magickaWeight = npcClass->data.attributeWeights.magicka;
            uint8_t healthWeight = npcClass->data.attributeWeights.health;
            uint8_t staminaWeight = npcClass->data.attributeWeights.stamina;

            if (magickaWeight > healthWeight && magickaWeight > staminaWeight) {
                classMultiplier = 1.5f; // Büyücüler %50 daha pahalı
            }
        }

        // İlişki İndirimi
        float relationshipMultiplier = 1.0f;
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player && a_actor) {
            // TODO: CommonLibSSE-NG sürümünde GetRelationshipRank fonksiyonu bulunamadı.
            // Doğru fonksiyon ismini bulana kadar indirim devre dışı.
            int32_t rank = 0; 
            if (rank > 0) {
                if (rank == 1) relationshipMultiplier = 0.90f;      // Dost: %10 indirim
                else if (rank == 2) relationshipMultiplier = 0.75f; // Sırdaş: %25 indirim
                else if (rank == 3) relationshipMultiplier = 0.50f; // Müttefik: %50 indirim
                else if (rank >= 4) relationshipMultiplier = 0.25f; // Sevgili: %75 indirim
            }
        }

        return static_cast<int32_t>(baseCost * classMultiplier * relationshipMultiplier);
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
        // Haftalık maaş, işe alım bedelinin %20'si kadar olsun
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
