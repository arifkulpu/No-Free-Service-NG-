#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    static std::set<RE::FormID> s_notifiedActors;
    static std::set<RE::FormID> s_pendingPayment;
 
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
                        RE::FormID speakerID = speaker->formID;
                        SKSE::GetTaskInterface()->AddTask([speakerID]() {
                            auto* target = RE::TESForm::LookupByID<RE::Actor>(speakerID);
                            if (target && !EconomyManager::HasBeenPaid(target) && EconomyManager::IsPotentialFollower(target)) {
                                if (s_notifiedActors.find(speakerID) == s_notifiedActors.end()) {
                                    s_notifiedActors.insert(speakerID);
                                    int32_t cost = EconomyManager::CalculateRecruitmentCost(target);
                                    auto* player = RE::PlayerCharacter::GetSingleton();
                                    int32_t gold = player ? player->GetGoldAmount() : 0;
                                    std::string status = (gold >= cost) ? "Karsilayabilirsin" : "Altin Yetersiz!";
                                    RE::DebugNotification(std::format("[NFS] {} - Bedel: {} Altin ({})", 
                                        target->GetName(), cost, status).c_str());
                                }
                            }
                        });
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    static void HandlePayment(RE::Actor* actor)
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!actor || !player) return;
 
        int32_t cost = EconomyManager::CalculateRecruitmentCost(actor);
        int32_t playerGold = player->GetGoldAmount();
 
        if (playerGold >= cost) {
            auto* gold = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
            if (gold) {
                player->RemoveItem(gold, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                actor->AddObjectToContainer(gold, nullptr, cost, nullptr);
                EconomyManager::SetPaid(actor);
                EconomyManager::UpdateLastPaymentDay(actor, RE::Calendar::GetSingleton()->GetCurrentGameTime());
 
                auto* goldSound = RE::TESForm::LookupByID<RE::TESSound>(0x0003E7D3);
                if (goldSound) {
                    using func_t = void(RE::TESSound*, RE::TESObjectREFR*);
                    static REL::Relocation<func_t> playFunc{ RELOCATION_ID(17056, 17441) };
                    if (playFunc.address()) playFunc(goldSound, player);
                }
                RE::DebugNotification(std::format("[NFS] {} Altin odendi. {} gruba katildi.", cost, actor->GetName()).c_str());
            }
        } else {
            RE::DebugNotification(std::format("[NFS] Altin yetersiz! {} ayriliyor.", actor->GetName()).c_str());
            using func_t = void(RE::Actor*, bool, bool, bool);
            static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
            if (dismissFunc.address()) dismissFunc(actor, true, false, true);
            actor->EvaluatePackage(true, true);
        }
        s_notifiedActors.erase(actor->formID);
        s_pendingPayment.erase(actor->formID);
    }
 
    void Update()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;
 
        auto* crosshair = RE::CrosshairPickData::GetSingleton();
        if (crosshair) {
            auto ref = crosshair->target[0].get();
            auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
            if (actor && !actor->IsDisabled() && actor->IsPlayerTeammate()) {
                if (!EconomyManager::HasBeenPaid(actor) && s_pendingPayment.find(actor->formID) == s_pendingPayment.end()) {
                    RE::FormID id = actor->formID;
                    s_pendingPayment.insert(id);
                    SKSE::GetTaskInterface()->AddTask([id]() {
                        auto* target = RE::TESForm::LookupByID<RE::Actor>(id);
                        if (target && target->IsPlayerTeammate() && !EconomyManager::HasBeenPaid(target)) {
                            HandlePayment(target);
                        } else {
                            s_pendingPayment.erase(id);
                        }
                    });
                }
            }
        }
 
        auto* calendar = RE::Calendar::GetSingleton();
        if (!calendar) return;
        static float lastWageCheck = 0.0f;
        float currentTime = calendar->GetCurrentGameTime();
        if (currentTime - lastWageCheck < 0.05f) return;
        lastWageCheck = currentTime;
 
        std::vector<RE::FormID> toDismiss;
        auto& paidMap = EconomyManager::GetPaidMap();
        for (auto const& [formID, isPaid] : paidMap) {
            if (!isPaid) continue;
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (!actor || actor->IsDisabled() || !actor->IsPlayerTeammate()) continue;
            float lastPayment = EconomyManager::GetLastPaymentDay(actor);
            if (lastPayment > 0.0f && (currentTime - lastPayment >= 7.0f)) {
                int32_t wage = EconomyManager::CalculateWeeklyWage(actor);
                if (player->GetGoldAmount() >= wage) {
                    auto* gold = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                    player->RemoveItem(gold, wage, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                    actor->AddObjectToContainer(gold, nullptr, wage, nullptr);
                    EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                } else {
                    toDismiss.push_back(formID);
                }
            }
        }
 
        for (auto id : toDismiss) {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (actor) {
                EconomyManager::ClearPaid(actor);
                using func_t = void(RE::Actor*, bool, bool, bool);
                static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
                if (dismissFunc.address()) dismissFunc(actor, true, false, true);
                actor->EvaluatePackage(true, true);
            }
        }
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
        }
    }
}
