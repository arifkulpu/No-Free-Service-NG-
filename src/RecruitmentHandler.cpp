#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>

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

        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (!a_event) return RE::BSEventNotifyControl::kContinue;

            if (a_event->menuName == RE::DialogueMenu::MENU_NAME) {
                if (a_event->opening) {
                    auto* player = RE::PlayerCharacter::GetSingleton();
                    auto* topicMgr = RE::MenuTopicManager::GetSingleton();
                    if (!player || !topicMgr) return RE::BSEventNotifyControl::kContinue;

                    // Speaker'ı bulmak için yöntemler deniyoruz
                    RE::Actor* speaker = nullptr;
                    if (topicMgr->speaker) {
                        auto* ref = topicMgr->speaker.get().get();
                        if (ref) {
                            speaker = ref->As<RE::Actor>();
                        }
                    }

                    if (speaker) {
                        RE::FormID speakerID = speaker->formID;
                        SKSE::GetTaskInterface()->AddTask([speakerID]() {
                            auto* target = RE::TESForm::LookupByID<RE::Actor>(speakerID);
                            if (target && EconomyManager::IsPotentialFollower(target)) {
                                if (!EconomyManager::HasBeenPaid(target)) {
                                    int32_t cost = EconomyManager::CalculateRecruitmentCost(target);
                                    RE::DebugNotification(std::format("{} hizmet bedeli: {} Altin.", target->GetName(), cost).c_str());
                                    GetSingleton()->m_pendingSpeakerID = speakerID;
                                }
                            }
                        });
                    }
                } else {
                    RE::FormID savedID = m_pendingSpeakerID.exchange(0);
                    if (savedID != 0) {
                        // Diyalog tamamen kapandıktan sonra kontrol et (1 kare sonra)
                        SKSE::GetTaskInterface()->AddTask([savedID]() {
                            auto* target = RE::TESForm::LookupByID<RE::Actor>(savedID);
                            // Eğer artık teammate olmuşsa ödeme al
                            if (target && target->IsPlayerTeammate()) {
                                if (!EconomyManager::HasBeenPaid(target)) {
                                    HandlePayment(savedID);
                                }
                            }
                        });
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        std::atomic<RE::FormID> m_pendingSpeakerID = 0;

        static void HandlePayment(RE::FormID actorID)
        {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(actorID);
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!actor || !player) return;

            int32_t cost = EconomyManager::CalculateRecruitmentCost(actor);
            int32_t playerGold = player->GetGoldAmount();

            if (playerGold >= cost) {
                auto* gold = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                if (gold) {
                    // Oyuncudan parayı kes
                    player->RemoveItem(gold, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                    
                    // Parayı doğrudan NPC'nin envanterine ekle
                    actor->AddObjectToContainer(gold, nullptr, cost, nullptr);
                    
                    EconomyManager::SetPaid(actor);
                    EconomyManager::UpdateLastPaymentDay(actor, RE::Calendar::GetSingleton()->GetCurrentGameTime());

                    auto* goldSound = RE::TESForm::LookupByID<RE::TESSound>(0x0003E7D3);
                    if (goldSound) {
                        using func_t = void(RE::TESSound*, RE::TESObjectREFR*);
                        static REL::Relocation<func_t> playFunc{ RELOCATION_ID(17056, 17441) };
                        if (playFunc.address()) playFunc(goldSound, player);
                    }

                    RE::DebugNotification(std::format("{} Altin odendi. {} gruba katildi.", cost, actor->GetName()).c_str());
                    logger::info("Payment of {} gold collected and given to {}", cost, actor->GetName());
                }
            } else {
                RE::DebugNotification(std::format("Yetersiz Altin! {} en az {} altin bekliyor.", actor->GetName(), cost).c_str());

                using func_t = void(RE::Actor*, bool, bool, bool);
                static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
                if (dismissFunc.address()) {
                    dismissFunc(actor, true, false, true);
                }
                actor->EvaluatePackage(true, true);
                logger::info("Insufficient gold, removed {} from party", actor->GetName());
            }
        }
    };

    void Update() 
    {
        static float lastCheckTime = 0.0f;
        auto* calendar = RE::Calendar::GetSingleton();
        if (!calendar) return;

        float currentTime = calendar->GetCurrentGameTime();
        // Her 0.01 oyun gününde bir (yaklaşık 15 gerçek dakika varsayılan timescale ile) kontrol et
        if (currentTime - lastCheckTime < 0.01f) return; 
        lastCheckTime = currentTime;

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        std::vector<RE::FormID> toDismiss;
        auto& paidMap = EconomyManager::GetPaidMap();

        for (auto const& [formID, isPaid] : paidMap) {
            if (!isPaid) continue;

            auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            // Sadece aktif ve takım arkadaşı olanları kontrol et
            if (actor && !actor->IsDisabled() && actor->IsPlayerTeammate()) {
                float lastPayment = EconomyManager::GetLastPaymentDay(actor);
                
                // Eğer ilk ödeme günü henüz atanmamışsa (eski kayıtlar için)
                if (lastPayment <= 0.0f) {
                    EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                    continue;
                }

                if (currentTime - lastPayment >= 7.0f) {
                    int32_t wage = EconomyManager::CalculateWeeklyWage(actor);
                    int32_t playerGold = player->GetGoldAmount();

                    if (playerGold >= wage) {
                        auto* gold = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
                        if (gold) {
                            player->RemoveItem(gold, wage, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                            actor->AddObjectToContainer(gold, nullptr, wage, nullptr);
                            
                            EconomyManager::UpdateLastPaymentDay(actor, currentTime);
                            RE::DebugNotification(std::format("{} haftalik maasi ({} Altin) odendi.", actor->GetName(), wage).c_str());
                            logger::info("Weekly wage of {} paid to {}", wage, actor->GetName());
                        }
                    } else {
                        RE::DebugNotification(std::format("Altin yetersiz! {} maasini alamadigi icin ayriliyor.", actor->GetName()).c_str());
                        toDismiss.push_back(formID);
                    }
                }
            }
        }

        for (auto id : toDismiss) {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (actor) {
                EconomyManager::ClearPaid(actor);
                using func_t = void(RE::Actor*, bool, bool, bool);
                static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
                if (dismissFunc.address()) {
                    dismissFunc(actor, true, false, true);
                }
                actor->EvaluatePackage(true, true);
                logger::info("Follower {} dismissed due to unpaid wages", actor->GetName());
            }
        }
    }

    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
            logger::info("RecruitmentHandler: Event-Based System v5.0 installed.");
        }
    }
}
