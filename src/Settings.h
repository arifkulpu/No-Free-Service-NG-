#pragma once
#include <cstdint>

namespace Settings
{
    extern int32_t RecruitmentBaseCost;
    extern int32_t RecruitmentLevelMultiplier;
    extern int32_t WeeklyWage;
    extern int32_t GracePeriodDuration;

    void Load();
}
