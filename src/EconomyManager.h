#pragma once
#include <RE/Skyrim.h>
#include <map>

namespace EconomyManager
{
    bool IsPotentialFollower(RE::Actor* a_actor);
    int32_t CalculateRecruitmentCost(RE::Actor* a_actor);
    
    void SetPaid(RE::Actor* a_actor);
    void ClearPaid(RE::Actor* a_actor);
    bool HasBeenPaid(RE::Actor* a_actor);
    bool IsInGracePeriod(RE::Actor* a_actor);
    
    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day);
    float GetLastPaymentDay(RE::Actor* a_actor);
    
    std::map<RE::FormID, bool> GetPaidMap();
    std::map<RE::FormID, float> GetPaymentDayMap();

    void SetPaidFromLoad(RE::FormID a_id, bool a_paid);
    void SetPaymentDayFromLoad(RE::FormID a_id, float a_day);
}
