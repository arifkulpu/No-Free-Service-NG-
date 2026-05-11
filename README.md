# No Free Service (NG)

**No Free Service** is a lightweight SKSE plugin that grounds Skyrim's follower system in a realistic economic framework. No one risks their life for free anymore! You must pay a recruitment fee before a follower joins your cause.

---

## ✅ Features / Özellikler

### 1. Dynamic Pricing / Dinamik Fiyatlandırma
Fees are calculated based on the NPC's skills and their relationship with the player:
*   **Class-Based / Sınıf Bazlı:** Mages (Magicka-heavy classes) are **50% more expensive** due to material costs.
*   **Relationship Discounts / İlişki İndirimi:** Your bond with the NPC grants discounts:
    *   Friend / Dost: 10% off
    *   Confidant / Sırdaş: 25% off
    *   Ally / Müttefik: 50% off
    *   Lover / Sevgili: 75% off

### 2. Weekly Wage System / Haftalık Maaş Sistemi
Payment is no longer a one-time thing! You must pay your followers a weekly wage every **7 game days**.
*   The wage is **20%** of the initial recruitment cost.
*   If you lack the gold, the follower will cease their service and leave the party.

### 3. Automatic Fee Notification / Otomatik Ücret Bildirimi
When you start talking to a potential follower, a notification appears in the top-left showing their service fee, calculated in real-time based on their level, class, and relationship.

### 4. Save Support / Kayıt Desteği
Payment status and wage schedules are safely stored in your save file.

### 5. Stable Event-Based Architecture / Kararlı Mimari
Minimal background processing; checks only occur when game time passes or dialogue events trigger.

---

## 🛠️ Roadmap / Yol Haritası

1.  **MCM Support:** Configurable price multipliers and wage intervals.
2.  **Risk Scenarios:** Chance for NPCs to become hostile if unpaid instead of just leaving.

---

## 📥 Installation / Kurulum

1.  Ensure **Address Library for SKSE Plugins** is installed.
2.  Copy `NoFreeService.dll` to your `Data/SKSE/Plugins/` folder.
3.  Enjoy!

---

## 📜 Credits / Krediler

- **Developer:** Antigravity
- **Infrastructure:** CommonLibSSE-NG & SKSE Team

---

## ⚖️ License / Lisans

Copyright (c) 2026 Arif KULPU. All Rights Reserved. — Tüm Hakları Saklıdır. See [LICENSE](LICENSE.md) for details.
