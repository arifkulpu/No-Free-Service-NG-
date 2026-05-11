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
                    
                    if (speaker) {
                        // Daha önce ödeme yapıldı mı kontrol et
                        if (!EconomyManager::HasBeenPaid(speaker)) {
                            int32_t cost = EconomyManager::CalculateRecruitmentCost(speaker);
                            auto* player = RE::PlayerCharacter::GetSingleton();
                            int32_t playerGold = player ? player->GetGoldAmount() : 0;
                            
                            std::string status = (playerGold >= cost) ? "Karsilayabilirsin" : "Altin Yetersiz!";
                            RE::DebugNotification(std::format("[NFS v4] {} - Bedel: {} Altin ({})", 
                                speaker->GetName(), cost, status).c_str());
                        } else {
                            RE::DebugNotification(std::format("[NFS v4] {} zaten ödeme yapılmış.", speaker->GetName()).c_str());
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
