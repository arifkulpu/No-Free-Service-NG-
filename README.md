# No Free Service (NG)

**No Free Service**, Skyrim'deki takipçi sistemini gerçekçi bir ekonomik temele oturtan hafif bir SKSE eklentisidir. Artık kimse canını bedavaya tehlikeye atmıyor! Bir takipçiyi grubuna dahil etmeden önce hizmet bedelini ödemek zorundasın.

**No Free Service** is a lightweight SKSE plugin that grounds Skyrim's follower system in a realistic economic framework. No one risks their life for free anymore! You must pay a recruitment fee before a follower joins your cause.

---

## ✅ Features / Özellikler

### 1. Dynamic Pricing / Dinamik Fiyatlandırma
Fees are calculated based on the NPC's skills and their relationship with the player:
*   **Class-Based / Sınıf Bazlı:** Büyücüler (Magicka ağırlıklı sınıflar) malzeme masrafları nedeniyle **%50 daha pahalıdır**. / Mages (Magicka-heavy classes) are **50% more expensive** due to material costs.
*   **Relationship Discounts / İlişki İndirimi:** NPC ile aranızdaki ilişki geliştikçe indirim kazanırsınız. / Your bond with the NPC grants discounts.
    *   Friend / Dost: 10% off / %10 indirim
    *   Confidant / Sırdaş: 25% off / %25 indirim
    *   Ally / Müttefik: 50% off / %50 indirim
    *   Lover / Sevgili: 75% off / %75 indirim

### 2. Weekly Wage System / Haftalık Maaş Sistemi
Payment is no longer a one-time thing! You must pay your followers a weekly wage every **7 game days**.
*   Artık ödeme tek seferlik değil! Her **7 oyun içi günde bir** takipçinize maaş ödemelisiniz.
*   The wage is **20%** of the initial recruitment cost. / Maaş, işe alım bedelinin **%20'si** kadardır.
*   If you lack the gold, the follower will leave the party. / Eğer altınınız yetersizse, takipçi gruptan ayrılır.

### 3. Automatic Fee Notification / Otomatik Ücret Bildirimi
Bir takipçi adayıyla konuşmaya başladığında sol üstte hizmet bedeli bildirimi belirir. Ücret o anki şartlara göre hesaplanır. / When you start talking to a potential follower, a notification appears in the top-left showing their service fee.

### 4. Save Support / Kayıt Desteği
Payment status and wage schedules are safely stored in your save file. / Ödeme durumları ve maaş günleri oyun kaydına güvenli bir şekilde işlenir.

### 5. Stable Architecture / Kararlı Mimari
Minimal background processing for maximum stability. / Maksimum stabilite için minimum arka plan işlemi.

---

## 🛠️ Roadmap / Yol Haritası

1.  **MCM Support:** Configurable price multipliers and wage intervals. / Fiyat çarpanı ve maaş günü aralığı ayarlanabilecek.
2.  **Risk Scenarios:** Chance for NPCs to become hostile if unpaid. / Ödeme yapılamadığında NPC'nin düşman olma ihtimali.

---

## 📥 Installation / Kurulum

1.  Ensure **Address Library for SKSE Plugins** is installed. / **Address Library** yüklü olduğundan emin olun.
2.  Copy `NoFreeService.dll` to your `Data/SKSE/Plugins/` folder. / Dosyayı Plugins klasörüne kopyalayın.
3.  Enjoy! / Keyfini çıkarın!

---

## 📜 Credits / Krediler

- **Developers / Geliştiriciler:** Arif KULPU & Antigravity
- **Infrastructure / Altyapı:** CommonLibSSE-NG & SKSE Team

---

## ⚖️ License / Lisans

Copyright (c) 2026 Arif KULPU. All Rights Reserved. — Tüm Hakları Saklıdır. See [LICENSE](LICENSE.md) for details.
