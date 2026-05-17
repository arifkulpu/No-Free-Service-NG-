#include "PCH.h"
#include "EconomyManager.h"
#include "Settings.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

// AE 1.6.1170 için MessageBoxData linker düzeltmesi (Tam vtable temizliği için)
namespace RE { MessageBoxData::~MessageBoxData() { buttonText.clear(); } }

namespace RecruitmentHandler
{
    static RE::FormID showingMenuFor = 0;

    // AE 1.6.1170 (v1.6.x) için DOĞRU ID'ler
    // GetRelationshipRank: 37537 (Eski: 36543)
    // SetRelationshipRank: 37538 (Eski: 36544)
    int32_t GetRank(RE::Actor* a, RE::Actor* b) {
        if (!a || !b) return 0;
        using func_t = int32_t(RE::Actor*, RE::Actor*);
        static REL::Relocation<func_t> func{ REL::ID(37537) };
        if (func.address()) return func(a, b);
        return 0;
    }

    void SetRank(RE::Actor* a, RE::TESForm* b, int32_t rank) {
        if (!a || !b) return;
        using func_t = void(RE::Actor*, RE::TESForm*, int32_t);
        static REL::Relocation<func_t> func{ REL::ID(37538) };
        if (func.address()) func(a, b, rank);
    }

    static RE::Actor* FindCurrentSpeaker()
    {
        auto* topicMgr = RE::MenuTopicManager::GetSingleton();
        if (topicMgr && topicMgr->speaker) {
            auto ref = topicMgr->speaker.get();
            if (ref) return ref->As<RE::Actor>();
        }
        return nullptr;
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
        RecruitmentMenuCallback(RE::Actor* a_speaker, int32_t a_cost) : speakerID(a_speaker ? a_speaker->formID : 0), cost(a_cost) {}
        
        void Run(Message a_msg) override
        {
            uint32_t response = static_cast<uint32_t>(a_msg);
            auto* player = RE::PlayerCharacter::GetSingleton();
            auto* speaker = RE::TESForm::LookupByID<RE::Actor>(speakerID);
            RecruitmentHandler::showingMenuFor = 0;
            
            if (player && speaker && response == 0) {
                auto* goldObj = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                if (goldObj && player->GetItemCount(goldObj) >= cost) {
                    player->RemoveItem(goldObj, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, speaker);
                    EconomyManager::SetPaid(speaker);
                    EconomyManager::UpdateLastPaymentDay(speaker, RE::Calendar::GetSingleton()->GetCurrentGameTime());
                    SetRank(speaker, player, 3);
                    RE::DebugNotification(std::format("Payment of {} gold received.", cost).c_str());
                    // Ödemeden sonra otomatik olarak diyalogu tekrar aç
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
        data->QueueMessage();
    }

    class MenuWatcher : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuWatcher* GetSingleton() { static MenuWatcher singleton; return &singleton; }
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (a_event && a_event->menuName == RE::DialogueMenu::MENU_NAME && a_event->opening) {
                auto* speaker = FindCurrentSpeaker();
                if (speaker) {
                    RE::FormID speakerID = speaker->formID;
                    SKSE::GetTaskInterface()->AddUITask([speakerID]() {
                        auto* actor = RE::TESForm::LookupByID<RE::Actor>(speakerID);
                        if (actor) {
                            bool isTeammate = actor->IsPlayerTeammate();
                            bool hasBeenPaid = EconomyManager::HasBeenPaid(actor);
                            bool isPotential = EconomyManager::IsPotentialFollower(actor);
                            
                            logger::info("Dialogue Opened - Actor {:X} ({}): isTeammate: {}, hasPaid: {}, isPotential: {}, showingMenuFor: {:X}", 
                                speakerID, actor->GetName(), isTeammate, hasBeenPaid, isPotential, RecruitmentHandler::showingMenuFor);

                            if (!isTeammate && !hasBeenPaid) {
                                if (isPotential) {
                                    if (RecruitmentHandler::showingMenuFor != speakerID) {
                                        logger::info("Showing payment menu for {}", actor->GetName());
                                        RecruitmentHandler::showingMenuFor = speakerID;
                                        RecruitmentHandler::CloseDialogue();
                                        int32_t cost = EconomyManager::CalculateRecruitmentCost(actor);
                                        RecruitmentHandler::ShowRecruitmentMenu(actor, cost);
                                    } else {
                                        logger::info("Menu is already being shown for {}", actor->GetName());
                                    }
                                } else {
                                    logger::info("Actor {} is not a potential follower", actor->GetName());
                                }
                            }
                        }
                    });
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
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastCheck).count() < 2000) return;
        lastCheck = std::chrono::steady_clock::now();

        // Update real-time payment states (grace period and dismissal expirations)
        EconomyManager::UpdateFollowerPaymentStates();
    }
 
    void Install() {
        auto* ui = RE::UI::GetSingleton();
        if (ui) ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
    }
}
