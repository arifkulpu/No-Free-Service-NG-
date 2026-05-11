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
                        // TAKİPÇİ DURUMU TESTİ
                        bool isFollower = speaker->IsPlayerTeammate();
                        std::string name = speaker->GetName();
                        
                        char buf[256];
                        snprintf(buf, sizeof(buf), "[NFS v4.4] %s - Takipci mi: %s", 
                            name.c_str(), isFollower ? "EVET" : "HAYIR");
                        RE::DebugNotification(buf);
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
