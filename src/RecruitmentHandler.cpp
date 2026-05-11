#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    class MenuWatcher : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuWatcher* GetSingleton()
        {
            static MenuWatcher singleton;
            return &singleton;
        }
 
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                              RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (a_event && a_event->menuName == RE::DialogueMenu::MENU_NAME && a_event->opening) {
                auto* topicMgr = RE::MenuTopicManager::GetSingleton();
                if (topicMgr && topicMgr->speaker) {
                    auto ref = topicMgr->speaker.get();
                    auto* speaker = ref ? ref->As<RE::Actor>() : nullptr;
                    auto* player = RE::PlayerCharacter::GetSingleton();
                    
                    if (speaker && player && !EconomyManager::HasBeenPaid(speaker)) {
                        auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                        int32_t cost = 500; // Şimdilik sabit fiyat
                        
                        if (goldObj && player->GetItemCount(goldObj) >= cost) {
                            // PARAYI DÜŞÜR (CRITICAL TEST)
                            player->RemoveItem(goldObj, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                            speaker->AddObjectToContainer(goldObj, nullptr, cost, nullptr);
                            
                            EconomyManager::SetPaid(speaker);
                            EconomyManager::UpdateLastPaymentDay(speaker, RE::Calendar::GetSingleton()->GetCurrentGameTime());
                            
                            char buf[256];
                            snprintf(buf, sizeof(buf), "[NFS v5] %s icin %d Altin odendi.", speaker->GetName(), cost);
                            RE::DebugNotification(buf);
                        } else {
                            RE::DebugNotification("[NFS v5] Altin yetersiz, hizmet sunulmuyor.");
                        }
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    void Update() {}
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
        }
    }
}
