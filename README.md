# No Free Service (Skyrim SE Plugin)

Skyrim'de "Bedava hizmet yok!" felsefesiyle yoldaşlık sistemini ekonomiye bağlayan bir SKSE eklentisidir. Artık en yakın dostunuz bile olsa, hizmeti için bir bedel ödemeniz gerekecek.

## 🚀 Özellikler

- **İnteraktif İşe Alım Menüsü:** Bir yoldaş adayıyla konuşmaya başladığınız anda otomatik olarak hizmet bedeli menüsü açılır.
- **Hizmet Bedeli Hesaplama:** NPC'nin seviyesine göre dinamik olarak hesaplanan işe alım ücreti.
- **Haftalık Maaş Sistemi:** Takipçileriniz her hafta maaş talep eder. Ödenmezse hizmeti bırakırlar.
- **Zorunlu Nötrleştirme:** Ödeme yapılmadığı sürece potansiyel yoldaşlar (modla eklenenler dahil) size karşı "Nötr" kalır.
- **Akıllı Filtreleme:** Tüccarlar ve ilgisiz NPC'ler filtrelenir; sadece gerçek yoldaş adaylarından para istenir.
- **Koruma Süresi (Grace Period):** Ödeme sonrası belirli bir süre boyunca kayıt korunur.
- **INI Konfigürasyon Desteği:** Tüm fiyatları ve süreleri `NoFreeService.ini` üzerinden ayarlayabilirsiniz.

## ⚙️ Yapılandırma (INI)

Modun ayarlarını `Data/SKSE/Plugins/NoFreeService.ini` dosyasından değiştirebilirsiniz:

```ini
[General]
RecruitmentBaseCost=200       ; Temel işe alım ücreti
RecruitmentLevelMultiplier=20 ; Seviye başına eklenen ücret
WeeklyWage=150                ; Haftalık ödenecek maaş
GracePeriodDuration=30        ; Ödeme koruma süresi (saniye)
```

## 🛠️ Teknik Detaylar

- **Framework:** CommonLibSSE-NG (1.6.1170+ uyumlu).
- **Zamanlama:** Gerçek zamanlı polling ve oyun saati entegrasyonu.

## 📥 Kurulum

1.  **Address Library for SKSE Plugins** yüklü olduğundan emin olun.
2.  `NoFreeService.dll` dosyasını `Data/SKSE/Plugins/` klasörüne kopyalayın.

---

## 📜 Krediler

- **Geliştiriciler:** Arif KULPU
- **Altyapı:** CommonLibSSE-NG & SKSE Team

---

## ⚖️ Lisans

Copyright (c) 2026 Arif KULPU. Tüm Hakları Saklıdır.
