#include "PCH.h"
#include "EconomyManager.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <atomic>
#include <set>
 
namespace RecruitmentHandler
{
    // Bildirim gönderilmiş NPC'leri takip et (tekrar tekrar göstermesin)
    static std::set<RE::FormID> s_notifiedActors;
    // Ödeme bekleyen (yeni teammate olan) NPC'ler
    static std::set<RE::FormID> s_pendingPayment;
 
    // -----------------------------------------------------------------------
    // Diyalog açılınca: fiyat bildirimini göster
    // -----------------------------------------------------------------------
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
 
            // Sadece diyalog menüsünü dinle
            if (a_event->menuName != RE::DialogueMenu::MENU_NAME)
                return RE::BSEventNotifyControl::kContinue;
 
            if (!a_event->opening) return RE::BSEventNotifyControl::kContinue;
 
            // Speaker'ı bul
            auto* topicMgr = RE::MenuTopicManager::GetSingleton();
            if (!topicMgr || !topicMgr->speaker) return RE::BSEventNotifyControl::kContinue;
 
            auto* ref = topicMgr->speaker.get().get();
            if (!ref) return RE::BSEventNotifyControl::kContinue;
 
            auto* speaker = ref->As<RE::Actor>();
            if (!speaker) return RE::BSEventNotifyControl::kContinue;
 
            RE::FormID speakerID = speaker->formID;
 
            SKSE::GetTaskInterface()->AddTask([speakerID]() {
                auto* target = RE::TESForm::LookupByID<RE::Actor>(speakerID);
                if (!target) return;
 
                // Zaten ödeme yapılmışsa gösterme
                if (EconomyManager::HasBeenPaid(target)) return;
 
                // Potansiyel takipçi değilse gösterme
                if (!EconomyManager::IsPotentialFollower(target)) return;
 
                // Aynı NPC'ye tekrar tekrar bildirim gitmesin
                if (s_notifiedActors.count(speakerID)) return;
                s_notifiedActors.insert(speakerID);
 
                int32_t cost = EconomyManager::CalculateRecruitmentCost(target);
                auto* player = RE::PlayerCharacter::GetSingleton();
                int32_t playerGold = player ? player->GetGoldAmount() : 0;
 
                if (playerGold >= cost) {
                    RE::DebugNotification(
                        std::format("[NFS] {} - Hizmet Bedeli: {} Altin (Karsilayabilirsin)",
                            target->GetName(), cost).c_str());
                } else {
                    RE::DebugNotification(
                        std::format("[NFS] {} - Hizmet Bedeli: {} Altin (Altin yetersiz!)",
                            target->GetName(), cost).c_str());
                }
            });
 
            return RE::BSEventNotifyControl::kContinue;
        }
    };
 
    // -----------------------------------------------------------------------
    // Ödeme işlemi
    // -----------------------------------------------------------------------
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
            }
 
            EconomyManager::SetPaid(actor);
            EconomyManager::UpdateLastPaymentDay(actor,
                RE::Calendar::GetSingleton()->GetCurrentGameTime());
 
            // Altın sesi çal
            auto* goldSound = RE::TESForm::LookupByID<RE::TESSound>(0x0003E7D3);
            if (goldSound) {
                using func_t = void(RE::TESSound*, RE::TESObjectREFR*);
                static REL::Relocation<func_t> playFunc{ RELOCATION_ID(17056, 17441) };
                if (playFunc.address()) playFunc(goldSound, player);
            }
 
            RE::DebugNotification(
                std::format("[NFS] {} Altin odendi. {} gruba katildi.", cost, actor->GetName()).c_str());
            logger::info("Payment {} gold collected from player, given to {}", cost, actor->GetName());
 
        } else {
            // Para yok → takipçiyi kov
            RE::DebugNotification(
                std::format("[NFS] Altin yetersiz! {} grubundan ayriliyor.", actor->GetName()).c_str());
 
            using func_t = void(RE::Actor*, bool, bool, bool);
            static REL::Relocation<func_t> dismissFunc{ REL::ID(37351) };
            if (dismissFunc.address()) {
                dismissFunc(actor, true, false, true);
            }
            actor->EvaluatePackage(true, true);
 
            logger::info("Insufficient gold - {} dismissed from party", actor->GetName());
        }
 
        // Bildirim setinden temizle (tekrar konuşabilsin)
        s_notifiedActors.erase(actor->formID);
        s_pendingPayment.erase(actor->formID);
    }
 
    // -----------------------------------------------------------------------
    // Her frame çağrılır:
    // 1) Yeni teammate olan ödenmemiş NPC'leri yakala → ödeme al
    // 2) Haftalık maaş kontrolü
    // -----------------------------------------------------------------------
    void Update()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;
 
        auto* calendar = RE::Calendar::GetSingleton();
        if (!calendar) return;
 
        // Tüm takım arkadaşlarını dolaş
        auto* processLists = RE::ProcessLists::GetSingleton();
        if (!processLists) return;
 
        // --- 1) Yeni işe alınan ödenmemiş takipçi kontrolü ---
        for (auto& handle : processLists->highActorHandles) {
            auto actorPtr = handle.get();
            if (!actorPtr) continue;
            auto* actor = actorPtr.get();
            if (!actor) continue;
            if (actor->IsDisabled()) continue;
            if (!actor->IsPlayerTeammate()) continue;
 
            RE::FormID id = actor->formID;
 
            // Ödeme zaten yapılmış → atla
            if (EconomyManager::HasBeenPaid(actor)) continue;
 
            // Zaten pending listesinde → atla (bir kere işle)
            if (s_pendingPayment.count(id)) continue;
 
            // Yeni teammate! Ödeme kuyruğuna al
            s_pendingPayment.insert(id);
 
            // 1 frame sonra işle (NFF'in kendi state'ini oturtsun diye küçük gecikme)
            SKSE::GetTaskInterface()->AddTask([id]() {
                auto* target = RE::TESForm::LookupByID<RE::Actor>(id);
                if (target && target->IsPlayerTeammate() && !EconomyManager::HasBeenPaid(target)) {
                    HandlePayment(target);
                } else {
                    s_pendingPayment.erase(id);
                }
            });
        }
 
        // --- 2) Haftalık maaş kontrolü (her 0.01 oyun gününde bir) ---
        static float lastWageCheck = 0.0f;
        float currentTime = calendar->GetCurrentGameTime();
        if (currentTime - lastWageCheck < 0.01f) return;
        lastWageCheck = currentTime;
 
        std::vector<RE::FormID> toDismiss;
        auto& paidMap = EconomyManager::GetPaidMap();
 
        for (auto const& [formID, isPaid] : paidMap) {
            if (!isPaid) continue;
 
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (!actor || actor->IsDisabled() || !actor->IsPlayerTeammate()) continue;
 
            float lastPayment = EconomyManager::GetLastPaymentDay(actor);
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
                        RE::DebugNotification(
                            std::format("[NFS] {} haftalik maasi ({} Altin) odendi.", actor->GetName(), wage).c_str());
                        logger::info("Weekly wage {} paid to {}", wage, actor->GetName());
                    }
                } else {
                    RE::DebugNotification(
                        std::format("[NFS] Altin yetersiz! {} maasini alamadi, ayriliyor.", actor->GetName()).c_str());
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
                logger::info("Follower {} dismissed - unpaid wages", actor->GetName());
            }
        }
    }
 
    void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->GetEventSource<RE::MenuOpenCloseEvent>()->AddEventSink(MenuWatcher::GetSingleton());
            logger::info("RecruitmentHandler: NFF-compatible system installed.");
        }
    }
}
