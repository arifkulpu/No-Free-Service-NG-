#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <RE/A/Actor.h>
#include <map>

namespace EconomyManager
{
    static std::map<RE::FormID, bool> paidFollowers;
    static std::map<RE::FormID, float> paymentDays;

    bool IsPotentialFollower(RE::Actor* a_actor)
    {
        // NPC objesine dokunmadan sadece varlığını kontrol et
        if (!a_actor) return false;

        // Diyalog menüsü açıksa "true" dön (En güvenli yol)
        auto* ui = RE::UI::GetSingleton();
        if (ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) return true;

        return false;
    }

    int32_t CalculateRecruitmentCost(RE::Actor* a_actor)
    {
        // Seviye kontrolünü bile kaldırıyoruz, sabit fiyat!
        return 500;
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

    int32_t CalculateWeeklyWage(RE::Actor* a_actor) { return 100; }
    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day) { if (a_actor) paymentDays[a_actor->formID] = a_day; }
    float GetLastPaymentDay(RE::Actor* a_actor) { if (a_actor && paymentDays.count(a_actor->formID)) return paymentDays[a_actor->formID]; return 0.0f; }
    std::map<RE::FormID, bool>& GetPaidMap() { return paidFollowers; }
    std::map<RE::FormID, float>& GetPaymentDayMap() { return paymentDays; }
}
