#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    static auto* currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);

    void HandleDismiss(RE::Actor* actor)
    {
        if (!actor) return;
        
        // 1. Orijinal Kovma Fonksiyonu
        using func_t = void(RE::Actor*, bool, bool, bool);
        static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
        if (dismissFunc.address()) {
            dismissFunc(actor, true, false, true);
        }

        // 2. Faction Derecesini -1 yap ve çıkar (En kesin yöntem)
        if (currentFollowerFaction) {
            actor->SetFactionRank(currentFollowerFaction, -1);
            actor->RemoveFromFaction(currentFollowerFaction);
        }

        // 3. AI Güncelle
        actor->EvaluatePackage(true, true);
    }

    bool IsCandidate(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsPlayerRef()) return false;
        if (a_actor->IsGuard()) return false;
        auto* race = a_actor->GetRace();
        if (race && race->formID == 0x00013197) return false;
        return true; 
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
                    
                    if (speaker && player && IsCandidate(speaker) && !EconomyManager::HasBeenPaid(speaker)) {
                        auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                        int32_t cost = 500;
                        
                        if (goldObj && player->GetItemCount(goldObj) >= cost) {
                            player->RemoveItem(goldObj, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                            speaker->AddObjectToContainer(goldObj, nullptr, cost, nullptr);
                            
                            EconomyManager::SetPaid(speaker);
                            EconomyManager::UpdateLastPaymentDay(speaker, RE::Calendar::GetSingleton()->GetCurrentGameTime());
                            
                            RE::DebugNotification(std::format("[NFS] {} hizmet bedeli alindi.", speaker->GetName()).c_str());
                        } else {
                            RE::DebugNotification(std::format("[NFS] {} için 500 Altin gerekiyor!", speaker->GetName()).c_str());
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
        if (currentTime - lastCheck < 0.5f) return; 
        lastCheck = currentTime;

        auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
        
        auto* processLists = RE::ProcessLists::GetSingleton();
        if (processLists) {
            for (auto& handle : processLists->highActorHandles) {
                auto ref = handle.get();
                auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
                
                if (actor && !EconomyManager::HasBeenPaid(actor)) {
                    // Takipçi mi? (Hem IsPlayerTeammate hem Faction kontrolü)
                    bool isFollowing = actor->IsPlayerTeammate() || (currentFollowerFaction && actor->IsInFaction(currentFollowerFaction));
                    
                    if (isFollowing) {
                        RE::DebugNotification(std::format("[NFS] {} ödeme yapmadan seni takip edemez!", actor->GetName()).c_str());
                        HandleDismiss(actor);
                    }
                }
            }
        }

        auto& paidMap = EconomyManager::GetPaidMap();
        std::vector<RE::FormID> toDismiss;
        for (auto const& [id, isPaid] : paidMap) {
            if (!isPaid) continue;
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (!actor) continue;

            bool isFollowing = actor->IsPlayerTeammate() || (currentFollowerFaction && actor->IsInFaction(currentFollowerFaction));
            if (!isFollowing) continue;

            float lastPay = EconomyManager::GetLastPaymentDay(actor);
            if (currentTime - lastPay >= 7.0f) {
                int32_t wage = 150;
                if (player->GetItemCount(goldObj) >= wage) {
                    player->RemoveItem(goldObj, wage, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                    EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                    RE::DebugNotification(std::format("[NFS] Haftalik maas ödendi: {}.", actor->GetName()).c_str());
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
                RE::DebugNotification(std::format("[NFS] Maas odenemedigi icin {} ayrildi.", actor->GetName()).c_str());
            }
        }
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
    }
}
