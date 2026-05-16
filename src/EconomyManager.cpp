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
            auto& status = paidFollowers[a_actor->formID];
            status.isPaid = true;
            status.realTimePaidAt = std::chrono::steady_clock::now();
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
        paidFollowers[a_id].isPaid = a_paid;
        paidFollowers[a_id].realTimePaidAt = std::chrono::steady_clock::now();
    }

    void SetPaymentDayFromLoad(RE::FormID a_id, float a_day) {
        paidFollowers[a_id].lastPaymentDay = a_day;
    }
}
