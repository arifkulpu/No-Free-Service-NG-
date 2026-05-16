#pragma once

#include <RE/Skyrim.h>

namespace RecruitmentHandler
{
    void Install();
    void Update();
    int32_t GetRank(RE::Actor* a, RE::Actor* b);
}
