#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    static std::set<RE::FormID> s_pendingPayment;
 
    bool IsActuallyPotentialFollower(RE::Actor* a_actor)
    {
        if (!a_actor) return false;
        
        // Faction ID'leri (Vanilla)
        static auto* potentialFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84D);
        static auto* currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
        
        if (potentialFollowerFaction && a_actor->IsInFaction(potentialFollowerFaction)) return true;
        if (currentFollowerFaction && a_actor->IsInFaction(currentFollowerFaction)) return true;
        if (a_actor->IsPlayerTeammate()) return true;
        
        return false;
    }

    void HandleDismiss(RE::Actor* actor)
    {
        if (!actor) return;
        using func_t = void(RE::Actor*, bool, bool, bool);
        static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
        if (dismissFunc.address()) {
            dismissFunc(actor, true, false, true);
            actor->EvaluatePackage(true, true);
            RE::DebugNotification(std::format("[NFS] {} hizmeti sonlandirdi.", actor->GetName()).c_str());
        }
    }

    class MenuWatcher : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuWatcher* GetSingleton() { static MenuWatcher singleton; return &singleton; }
 
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (a_event && a_event->menuName == RE::DialogueMenu::MENU_NAME && a_event->opening) {
                auto* topicMgr = RE::MenuTopicManager::GetSingleton();
                if (topicMgr && topicMgr->speaker) {
                    auto ref = topicMgr->speaker.get();
                    auto* speaker = ref ? ref->As<RE::Actor>() : nullptr;
                    auto* player = RE::PlayerCharacter::GetSingleton();
                    
                    if (speaker && player && !EconomyManager::HasBeenPaid(speaker)) {
                        if (IsActuallyPotentialFollower(speaker)) {
                            auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                            int32_t cost = 500;
                            
                            if (goldObj && player->GetItemCount(goldObj) >= cost) {
                                player->RemoveItem(goldObj, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                                speaker->AddObjectToContainer(goldObj, nullptr, cost, nullptr);
                                EconomyManager::SetPaid(speaker);
                                EconomyManager::UpdateLastPaymentDay(speaker, RE::Calendar::GetSingleton()->GetCurrentGameTime());
                                RE::DebugNotification(std::format("[NFS] {} icin {} Altin odendi.", speaker->GetName(), cost).c_str());
                            } else {
                                RE::DebugNotification("[NFS] Takipçi için yeterli altınınız yok.");
                            }
                        }
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    void Update()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* calendar = RE::Calendar::GetSingleton();
        if (!player || !calendar) return;

        static float lastCheck = 0.0f;
        float currentTime = calendar->GetCurrentGameTime();
        if (currentTime - lastCheck < 0.1f) return; // Performans için
        lastCheck = currentTime;

        auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
        if (!goldObj) return;

        // Haftalık Maaş Kontrolü
        auto& paidMap = EconomyManager::GetPaidMap();
        std::vector<RE::FormID> toDismiss;

        for (auto const& [id, isPaid] : paidMap) {
            if (!isPaid) continue;
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (!actor || actor->IsDisabled() || !actor->IsPlayerTeammate()) continue;

            float lastPay = EconomyManager::GetLastPaymentDay(actor);
            if (currentTime - lastPay >= 7.0f) {
                int32_t wage = 150; // Haftalık maaş
                if (player->GetItemCount(goldObj) >= wage) {
                    player->RemoveItem(goldObj, wage, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                    EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                    RE::DebugNotification(std::format("[NFS] {} icin haftalik {} Altin odendi.", actor->GetName(), wage).c_str());
                } else {
                    toDismiss.push_back(id);
                }
            }
        }

        for (auto id : toDismiss) {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (actor) {
                EconomyManager::ClearPaid(actor);
                HandleDismiss(actor);
            }
        }
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
    }
}
