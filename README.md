# No Free Service (Skyrim SE Plugin)

An SKSE plugin that ties the follower system to the economy with the philosophy "There is no free service!". Even your closest friends will now demand a price for their services.

---

## 🇺🇸 English Description

### 🚀 Features

- **Interactive Recruitment Menu:** A recruitment cost menu automatically opens the moment you start talking to a potential follower.
- **Dynamic Recruitment Cost:** Costs are calculated dynamically based on the NPC's level (Base: 500 Gold + 50 Gold per level).
- **Weekly Wage System:** Followers demand a weekly wage (Default: 150 Gold). If unpaid, they will leave your service.
- **Mandatory Neutralization:** Potential followers (including those from mods) remain "Neutral" until paid; they won't become allies for free.
- **Smart Filtering:** Merchants, shopkeepers, and common citizens are automatically filtered out; only real follower candidates are charged.
- **Grace Period:** Payment status is preserved for 30 seconds after payment, preventing re-triggering the menu if you exit dialogue.
- **INI Configuration Support:** All prices and durations can be adjusted via `NoFreeService.ini`.

### ⚙️ Configuration (INI)

You can modify settings in `Data/SKSE/Plugins/NoFreeService.ini`:

```ini
[General]
RecruitmentBaseCost=500       ; Base recruitment cost
RecruitmentLevelMultiplier=50 ; Additional cost per level
WeeklyWage=150                ; Weekly wage amount
GracePeriodDuration=30        ; Protection duration after payment (seconds)
```

### 📥 Installation

1.  Ensure **Address Library for SKSE Plugins** is installed.
2.  Copy `NoFreeService.dll` to your `Data/SKSE/Plugins/` folder.

---

## 🇹🇷 Türkçe Açıklama

### 🚀 Özellikler

- **İnteraktif İşe Alım Menüsü:** Bir yoldaş adayıyla konuşmaya başladığınız anda otomatik olarak hizmet bedeli menüsü açılır.
- **Dinamik İşe Alım Ücreti:** NPC'nin seviyesine göre dinamik hesaplama (Temel: 500 Altın + Seviye Başı 50 Altın).
- **Haftalık Maaş Sistemi:** Takipçileriniz haftalık maaş talep eder (Varsayılan: 150 Altın). Ödenmezse ayrılırlar.
- **Zorunlu Nötrleştirme:** Ödeme yapılmadığı sürece potansiyel yoldaşlar size karşı "Nötr" kalır, bedavaya müttefik olmazlar.
- **Akıllı Filtreleme:** Tüccarlar ve sıradan kasabalılar filtrelenir; sadece gerçek yoldaş adaylarından para istenir.
- **Koruma Süresi (Grace Period):** Ödeme sonrası 30 saniye boyunca kayıt korunur.
- **INI Konfigürasyon Desteği:** Tüm fiyatları `NoFreeService.ini` üzerinden ayarlayabilirsiniz.

### ⚙️ Yapılandırma (INI)

`Data/SKSE/Plugins/NoFreeService.ini` dosyasından ayarları değiştirebilirsiniz:

```ini
[General]
RecruitmentBaseCost=500       ; Temel işe alım ücreti
RecruitmentLevelMultiplier=50 ; Seviye başına eklenen ücret
WeeklyWage=150                ; Haftalık ödenecek maaş
GracePeriodDuration=30        ; Ödeme koruma süresi (saniye)
```

### 📥 Kurulum

1.  **Address Library for SKSE Plugins** yüklü olduğundan emin olun.
2.  `NoFreeService.dll` dosyasını `Data/SKSE/Plugins/` klasörüne kopyalayın.

---

## 📜 Credits / Krediler

- **Developers / Geliştiriciler:** Arif KULPU & Antigravity (Google DeepMind Team)
- **Infrastructure / Altyapı:** CommonLibSSE-NG & SKSE Team

---

## ⚖️ License / Lisans

Copyright (c) 2026 Arif KULPU. All Rights Reserved. — Tüm Hakları Saklıdır.
