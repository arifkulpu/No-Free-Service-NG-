#include "PCH.h"
#include "EconomyManager.h"
#include "Settings.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <vector>
#include <RE/I/IMessageBoxCallback.h>
#include <RE/M/MessageBoxData.h>
#include <RE/U/UIMessageQueue.h>
#include <RE/I/InterfaceStrings.h>
#include <RE/T/TESBoundObject.h>
#include <RE/C/Calendar.h>
#include <chrono>
 
namespace RE
{
	MessageBoxData::~MessageBoxData() {}
}

namespace RecruitmentHandler
{
    static RE::FormID showingMenuFor = 0;

    using SetRelationshipRank_t = void(RE::Actor*, RE::TESForm*, int32_t);
    static REL::Relocation<SetRelationshipRank_t> funcSetRank{ REL::ID(36544) };

    RE::TESFaction* GetNFFFollowerFaction() {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler) {
            return dataHandler->LookupForm<RE::TESFaction>(0x00000800, "nwsFollowerFramework.esp");
        }
        return nullptr;
    }

    void ForceDismiss(RE::Actor* actor)
    {
        if (!actor) return;
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        logger::info("ForceDismiss: Neutralizing {}.", actor->GetName());

        try {
            actor->GetActorRuntimeData().boolBits.reset(RE::Actor::BOOL_BITS::kPlayerTeammate);
        } catch (...) {}

        if (funcSetRank.address()) {
            try {
                funcSetRank(actor, player, 0);
            } catch (...) {}
        }

        static auto* followerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
        if (followerFaction) actor->RemoveFromFaction(followerFaction);

        auto* nffFaction = GetNFFFollowerFaction();
        if (nffFaction) actor->RemoveFromFaction(nffFaction);

        actor->EvaluatePackage(true, false);
    }

    void CloseDialogue()
    {
        auto ui = RE::UI::GetSingleton();
        if (ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
            auto msgQ = RE::UIMessageQueue::GetSingleton();
            if (msgQ) msgQ->AddMessage(RE::DialogueMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }

    class RecruitmentMenuCallback : public RE::IMessageBoxCallback
    {
    public:
        RE::FormID speakerID;
        int32_t cost;

        RecruitmentMenuCallback(RE::Actor* a_speaker, int32_t a_cost) :
            speakerID(a_speaker ? a_speaker->formID : 0), cost(a_cost) {}

        void Run(Message a_msg) override
        {
            uint32_t response = static_cast<uint32_t>(a_msg);
            auto* player = RE::PlayerCharacter::GetSingleton();
            auto* speaker = RE::TESForm::LookupByID<RE::Actor>(speakerID);
            
            RecruitmentHandler::showingMenuFor = 0;

            if (!player || !speaker) return;

            if (response == 0) {  // Pay
                auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                if (goldObj && player->GetItemCount(goldObj) >= cost) {
                    player->RemoveItem(goldObj, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, speaker);
                    EconomyManager::SetPaid(speaker);
                    EconomyManager::UpdateLastPaymentDay(speaker, RE::Calendar::GetSingleton()->GetCurrentGameTime());
                    
                    if (funcSetRank.address()) funcSetRank(speaker, player, 3);
                    
                    RE::DebugNotification(std::format("Payment of {} gold received.", cost).c_str());

                    // RE-OPEN DIALOGUE AUTOMATICALLY
                    speaker->ActivateRef(player, 0, nullptr, 1, false);
                } else {
                    RE::DebugNotification("You do not have enough gold!");
                }
            }
        }
    };

    void ShowRecruitmentMenu(RE::Actor* a_speaker, int32_t a_cost)
    {
        auto* data = new RE::MessageBoxData();
        data->bodyText = std::format("{}\nRecruitment cost: {} Gold", a_speaker->GetName(), a_cost).c_str();
        data->buttonText.push_back("Pay");
        data->buttonText.push_back("Cancel");
        data->callback = RE::BSTSmartPointer<RE::IMessageBoxCallback>(new RecruitmentMenuCallback(a_speaker, a_cost));
        data->type = 0;
        data->menuDepth = 10;
        data->cancelOptionIndex = 1;
        data->isCancellable = true;

        data->QueueMessage();
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
                        if (EconomyManager::IsPotentialFollower(speaker)) {
                            RecruitmentHandler::CloseDialogue();
                            
                            SKSE::GetTaskInterface()->AddTask([speaker]() {
                                if (RecruitmentHandler::showingMenuFor != speaker->formID) {
                                    RecruitmentHandler::showingMenuFor = speaker->formID;
                                    int32_t cost = EconomyManager::CalculateRecruitmentCost(speaker);
                                    RecruitmentHandler::ShowRecruitmentMenu(speaker, cost);
                                }
                            });
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
        if (!player) return;

        static auto lastCheck = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCheck).count();
        if (elapsed < 1000) return;
        lastCheck = now;

        auto* processLists = RE::ProcessLists::GetSingleton();
        if (processLists) {
            for (auto& handle : processLists->highActorHandles) {
                auto ref = handle.get();
                auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
                if (actor && !actor->IsPlayerRef()) {
                    if (EconomyManager::IsPotentialFollower(actor) && !EconomyManager::HasBeenPaid(actor)) {
                        // Force Neutral state for unpaid modded allies
                        if (funcSetRank.address()) funcSetRank(actor, player, 0);
                        
                        // Safety: Check if they are accidentally recruited
                        if (actor->IsPlayerTeammate()) {
                            RecruitmentHandler::ForceDismiss(actor);
                        }
                    }
                }
            }
        }

        // Wage logic
        auto paidMap = EconomyManager::GetPaidMap();
        std::vector<RE::FormID> activePaidIDs;
        for (auto const& [id, isPaid] : paidMap) if (isPaid) activePaidIDs.push_back(id);

        for (auto id : activePaidIDs) {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (!actor) continue;

            static auto* currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
            auto* nffFaction = GetNFFFollowerFaction();
            bool isFollowing = actor->IsPlayerTeammate() || 
                               (currentFollowerFaction && actor->IsInFaction(currentFollowerFaction)) ||
                               (nffFaction && actor->IsInFaction(nffFaction));

            if (!isFollowing) {
                // Only clear paid status if NOT in dialogue AND not in a 30s grace period
                bool inDialogue = RE::UI::GetSingleton()->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
                if (!inDialogue && !EconomyManager::IsInGracePeriod(actor)) {
                    EconomyManager::ClearPaid(actor);
                }
                continue;
            }

            float lastPay = EconomyManager::GetLastPaymentDay(actor);
            float currentTime = RE::Calendar::GetSingleton()->GetCurrentGameTime();
            if (currentTime - lastPay >= 7.0f) {
                auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                int32_t wage = Settings::WeeklyWage;
                if (player->GetItemCount(goldObj) >= wage) {
                    player->RemoveItem(goldObj, wage, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                    EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                } else {
                    EconomyManager::ClearPaid(actor);
                    ForceDismiss(actor);
                    RE::DebugNotification(std::format("{} left because their wages couldn't be paid.", actor->GetName()).c_str());
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
