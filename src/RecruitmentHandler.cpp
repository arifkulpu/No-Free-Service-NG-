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
            // v2: Sürüm takibi için yazı değiştirildi
            if (a_event && a_event->menuName == RE::DialogueMenu::MENU_NAME && a_event->opening) {
                RE::DebugNotification("[NFS v2] Acil Durum Modu - Diyalog Algilandi");
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    void Update()
    {
        // Stabilite testi için boş
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
        }
    }
}
