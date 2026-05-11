#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace SerializationManager
{
    void Register();
    
    bool IsNPCInPaidList(RE::FormID a_formID);
    void AddNPCToPaidList(RE::FormID a_formID);

    // Internal handlers
    void SaveCallback(SKSE::SerializationInterface* a_intfc);
    void LoadCallback(SKSE::SerializationInterface* a_intfc);
    void RevertCallback(SKSE::SerializationInterface* a_intfc);
}
