#include "PCH.h"
#include "SerializationManager.h"
#include "EconomyManager.h"
#include <SKSE/SKSE.h>

namespace SerializationManager
{
    static constexpr uint32_t SerializationID = 'NFSP';
    static constexpr uint32_t PaidMapType = 'PAID';
    static constexpr uint32_t PaymentDayType = 'PDAY';

    void SaveCallback(SKSE::SerializationInterface* a_intfc)
    {
        if (a_intfc->OpenRecord(PaidMapType, 0)) {
            auto paidMap = EconomyManager::GetPaidMap();
            size_t size = paidMap.size();
            a_intfc->WriteRecordData(size);
            for (auto const& [id, paid] : paidMap) {
                RE::FormID formID = id;
                a_intfc->WriteRecordData(formID);
                a_intfc->WriteRecordData(paid);
            }
        }

        if (a_intfc->OpenRecord(PaymentDayType, 0)) {
            auto dayMap = EconomyManager::GetPaymentDayMap();
            size_t size = dayMap.size();
            a_intfc->WriteRecordData(size);
            for (auto const& [id, dayValue] : dayMap) {
                RE::FormID formID = id;
                float day = dayValue;
                a_intfc->WriteRecordData(formID);
                a_intfc->WriteRecordData(day);
            }
        }
    }

    void LoadCallback(SKSE::SerializationInterface* a_intfc)
    {
        uint32_t type;
        uint32_t version;
        uint32_t length;

        while (a_intfc->GetNextRecordInfo(type, version, length)) {
            if (type == PaidMapType) {
                size_t size;
                a_intfc->ReadRecordData(size);
                for (size_t i = 0; i < size; ++i) {
                    RE::FormID oldFormID;
                    a_intfc->ReadRecordData(oldFormID);
                    
                    bool paid;
                    a_intfc->ReadRecordData(paid);

                    RE::FormID newFormID;
                    if (a_intfc->ResolveFormID(oldFormID, newFormID)) {
                        EconomyManager::SetPaidFromLoad(newFormID, paid);
                    }
                }
            } else if (type == PaymentDayType) {
                size_t size;
                a_intfc->ReadRecordData(size);
                for (size_t i = 0; i < size; ++i) {
                    RE::FormID oldFormID;
                    a_intfc->ReadRecordData(oldFormID);
                    
                    float day;
                    a_intfc->ReadRecordData(day);

                    RE::FormID newFormID;
                    if (a_intfc->ResolveFormID(oldFormID, newFormID)) {
                        EconomyManager::SetPaymentDayFromLoad(newFormID, day);
                    }
                }
            }
        }
    }

    void Register()
    {
        auto* serialization = SKSE::GetSerializationInterface();
        if (!serialization) return;
        
        serialization->SetUniqueID(SerializationID);
        serialization->SetSaveCallback(SaveCallback);
        serialization->SetLoadCallback(LoadCallback);
        logger::info("SerializationManager: Registered for save/load.");
    }
}
