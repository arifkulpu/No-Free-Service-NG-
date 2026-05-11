#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    // Fonksiyon Adresleri (SSE 1.6.1170 için Relocations)
    using SetRelationshipRank_t = void(RE::Actor*, RE::TESForm*, int32_t);
    static REL::Relocation<SetRelationshipRank_t> funcSetRank{ REL::ID(36544) };

    using SetPlayerTeammate_t = void(RE::Actor*, bool);
    static REL::Relocation<SetPlayerTeammate_t> funcSetTeammate{ REL::ID(36357) };

    using SetFactionRank_t = void(RE::Actor*, RE::TESFaction*, int8_t);
    static REL::Relocation<SetFactionRank_t> funcSetFactionRank{ REL::ID(36545) };

    void ForceDismiss(RE::Actor* actor)
    {
        if (!actor) return;
        auto* player = RE::PlayerCharacter::GetSingleton();

        if (funcSetRank.address() && player) funcSetRank(actor, player, 0);
        if (funcSetTeammate.address()) funcSetTeammate(actor, false);

        static auto* followerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
        if (followerFaction) {
            if (funcSetFactionRank.address()) funcSetFactionRank(actor, followerFaction, -1);
            actor->RemoveFromFaction(followerFaction);
        }

        using dismiss_t = void(RE::Actor*, bool, bool, bool);
        static REL::Relocation<dismiss_t> dismissFunc{ REL::ID(37351) };
        if (dismissFunc.address()) dismissFunc(actor, true, false, true);

        actor->EvaluatePackage(true, true);
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
                        auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                        int32_t cost = 500;
                        
                        if (funcSetRank.address()) funcSetRank(speaker, player, 0);

                        if (goldObj && player->GetItemCount(goldObj) >= cost) {
                            player->RemoveItem(goldObj, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                            speaker->AddObjectToContainer(goldObj, nullptr, cost, nullptr);
                            EconomyManager::SetPaid(speaker);
                            EconomyManager::UpdateLastPaymentDay(speaker, RE::Calendar::GetSingleton()->GetCurrentGameTime());
                            
                            if (funcSetRank.address()) funcSetRank(speaker, player, 3);
                            RE::DebugNotification(std::format("[NFS] {} hizmet bedeli alindi.", speaker->GetName()).c_str());
                        } else {
                            RE::DebugNotification(std::format("[NFS] {} hizmet bedeli (500 Altin) bekliyor.", speaker->GetName()).c_str());
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

        auto* processLists = RE::ProcessLists::GetSingleton();
        if (processLists) {
            static auto* currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
            for (auto& handle : processLists->highActorHandles) {
                auto ref = handle.get();
                auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
                
                if (actor && !EconomyManager::HasBeenPaid(actor) && !actor->IsPlayerRef()) {
                    bool isFollowing = actor->IsPlayerTeammate() || (currentFollowerFaction && actor->IsInFaction(currentFollowerFaction));
                    if (isFollowing) {
                        RE::DebugNotification(std::format("[NFS] {} ödeme yapmadan takip edemez!", actor->GetName()).c_str());
                        ForceDismiss(actor);
                    }
                }
            }
        }

        auto& paidMap = EconomyManager::GetPaidMap();
        for (auto const& [id, isPaid] : paidMap) {
            if (!isPaid) continue;
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (!actor) continue;

            static auto* currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
            bool isFollowing = actor->IsPlayerTeammate() || (currentFollowerFaction && actor->IsInFaction(currentFollowerFaction));
            if (!isFollowing) continue;

            float lastPay = EconomyManager::GetLastPaymentDay(actor);
            if (currentTime - lastPay >= 7.0f) {
                auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                int32_t wage = 150;
                if (player->GetItemCount(goldObj) >= wage) {
                    player->RemoveItem(goldObj, wage, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                    EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                } else {
                    EconomyManager::ClearPaid(actor);
                    ForceDismiss(actor);
                    RE::DebugNotification(std::format("[NFS] Maas odenemedigi icin {} ayrildi.", actor->GetName()).c_str());
                }
            }
        }
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
    }
}
