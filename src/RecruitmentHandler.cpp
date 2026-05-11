#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    static std::set<RE::FormID> s_notifiedActors;
 
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
                // Diyalog açılınca bildirim göster ama NPC üzerinden isim ALMA (Çökme riski!)
                RE::DebugNotification("[NFS] Diyalog Algilandi - Hizmet Bedeli: 500 Altin");
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    void Update()
    {
        // Çökme kaynağını bulmak için arka plan döngüsünü tamamen boşaltıyoruz
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
        }
    }
}
