#pragma once
#include <RE/Skyrim.h>
#include <stdint.h>
#include <map>

namespace EconomyManager
{
    int32_t CalculateRecruitmentCost(RE::Actor* a_actor);
    int32_t CalculateWeeklyWage(RE::Actor* a_actor);

    bool HasBeenPaid(RE::Actor* a_actor);
    void SetPaid(RE::Actor* a_actor);
    void ClearPaid(RE::Actor* a_actor);

    bool IsExempt(RE::Actor* a_actor);
    bool IsPotentialFollower(RE::Actor* a_actor);

    // Maaş Takibi
    void UpdateLastPaymentDay(RE::Actor* a_actor, float a_day);
    float GetLastPaymentDay(RE::Actor* a_actor);

    std::map<RE::FormID, bool>& GetPaidMap();
    std::map<RE::FormID, float>& GetPaymentDayMap();
}
