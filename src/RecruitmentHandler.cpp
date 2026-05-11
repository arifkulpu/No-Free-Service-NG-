#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    // Bildirim gönderilmiş NPC'leri takip et
    static std::set<RE::FormID> s_notifiedActors;
    // Ödeme bekleyen NPC'ler
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
            if (!a_event) return RE::BSEventNotifyControl::kContinue;
 
            if (a_event->menuName == RE::DialogueMenu::MENU_NAME) {
                if (a_event->opening) {
                    auto* topicMgr = RE::MenuTopicManager::GetSingleton();
                    if (topicMgr && topicMgr->speaker) {
                        auto* ref = topicMgr->speaker.get().get();
                        auto* speaker = ref ? ref->As<RE::Actor>() : nullptr;
                        
                        if (speaker) {
                            RE::FormID speakerID = speaker->formID;
                            logger::info("Dialogue opened with: {} ({:X})", speaker->GetName(), speakerID);

                            SKSE::GetTaskInterface()->AddTask([speakerID]() {
                                auto* target = RE::TESForm::LookupByID<RE::Actor>(speakerID);
                                if (!target) return;
 
                                if (EconomyManager::HasBeenPaid(target)) {
                                    logger::info("Notification skipped: {} already paid.", target->GetName());
                                    return;
                                }
 
                                if (!EconomyManager::IsPotentialFollower(target)) {
                                    logger::info("Notification skipped: {} is not a potential follower.", target->GetName());
                                    return;
                                }
 
                                // Bildirim daha önce gösterilmediyse göster
                                if (s_notifiedActors.find(speakerID) == s_notifiedActors.end()) {
                                    s_notifiedActors.insert(speakerID);
                                    int32_t cost = EconomyManager::CalculateRecruitmentCost(target);
                                    auto* player = RE::PlayerCharacter::GetSingleton();
                                    int32_t gold = player ? player->GetGoldAmount() : 0;

                                    std::string status = (gold >= cost) ? "Karsilayabilirsin" : "Altin Yetersiz!";
                                    RE::DebugNotification(std::format("[NFS] {} - Bedel: {} Altin ({})", 
                                        target->GetName(), cost, status).c_str());
                                    logger::info("Price notification shown for {}: {} gold", target->GetName(), cost);
                                }
                            });
                        }
                    }
                } else {
                    // Diyalog kapandığında bildirim setini temizleme (tekrar konuşunca görüksün diye)
                    // Ancak şimdilik sadece log basalım
                    logger::info("Dialogue menu closed.");
                }
            }
 
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    static void HandlePayment(RE::Actor* actor)
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!actor || !player) return;
 
        logger::info("Processing payment for: {}", actor->GetName());
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
                logger::info("Successfully paid {} for {}", cost, actor->GetName());
            }
        } else {
            RE::DebugNotification(std::format("[NFS] Altin yetersiz! {} ayriliyor.", actor->GetName()).c_str());
            
            using func_t = void(RE::Actor*, bool, bool, bool);
            static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
            if (dismissFunc.address()) {
                dismissFunc(actor, true, false, true);
            }
            actor->EvaluatePackage(true, true);
            logger::warn("Insufficient gold: {} dismissed.", actor->GetName());
        }
 
        s_notifiedActors.erase(actor->formID);
        s_pendingPayment.erase(actor->formID);
    }
 
    void Update()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;
 
        auto* calendar = RE::Calendar::GetSingleton();
        auto* processLists = RE::ProcessLists::GetSingleton();
        if (!calendar || !processLists) return;
 
        // 1) Yeni takipçi kontrolü
        for (auto& handle : processLists->highActorHandles) {
            auto actorPtr = handle.get();
            auto* actor = actorPtr ? actorPtr.get() : nullptr;
            
            if (actor && !actor->IsDisabled() && actor->IsPlayerTeammate()) {
                if (!EconomyManager::HasBeenPaid(actor) && s_pendingPayment.find(actor->formID) == s_pendingPayment.end()) {
                    RE::FormID id = actor->formID;
                    s_pendingPayment.insert(id);
                    logger::info("New teammate detected: {}. Scheduling payment check.", actor->GetName());

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
 
        // 2) Haftalık maaş kontrolü
        static float lastWageCheck = 0.0f;
        float currentTime = calendar->GetCurrentGameTime();
        if (currentTime - lastWageCheck < 0.05f) return; // Daha az sıklıkla kontrol (optimizasyon)
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
                    RE::DebugNotification(std::format("[NFS] Haftalik maas: {} Altin ({}).", wage, actor->GetName()).c_str());
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
            logger::info("RecruitmentHandler: Version 6.0 with deep logging installed.");
        }
    }
}
