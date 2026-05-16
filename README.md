# No Free Service (NG)

Skyrim SE/AE için geliştirilmiş, takipçi (follower) işe alımını daha gerçekçi ve ekonomik hale getiren bir SKSE eklentisidir. Artık kimse "bir iyilik" karşılığında sizinle ölüme gelmeyecek.

## 🚀 Özellikler

- **Dinamik İşe Alım Ücreti:** NPC'lerin seviyesine göre hesaplanan adil bir başlangıç bedeli.
  - Formül: `Temel Ücret (500) + (NPC Seviyesi * Çarpan (50))`
- **Haftalık Ödeme Sistemi:** Takipçileriniz artık sadece bir kere değil, haftalık olarak maaş ister.
- **Otomatik Menü Enjeksiyonu:** Potansiyel bir takipçiyle konuşmaya başladığınız anda menü otomatik olarak açılır.
- **AE 1.6.1170 Desteği:** En güncel Skyrim sürümüyle tam uyumlu ve stabil.
- **Mod Uyumluluğu:** 3DNPC, Beyond Skyrim: Bruma ve diğer modlu takipçilerle sorunsuz çalışır.
- **UI Dostu:** `SkyrimSoulsRE` ve `Status Indicator Framework` gibi popüler arayüz modlarıyla uyumluluk için UITask tabanlı çalışır.

## ⚙️ Yapılandırma (INI)

Ayarları `Data/SKSE/Plugins/NoFreeService.ini` dosyasından değiştirebilirsiniz:

```ini
[General]
RecruitmentBaseCost=500       ; Temel işe alım bedeli
RecruitmentLevelMultiplier=50 ; Seviye başı ek maliyet
WeeklyWage=150                ; Haftalık maaş miktarı
GracePeriodDuration=30        ; Ödeme sonrası koruma süresi (saniye)
```

## 🛠️ Teknik Detaylar

- **Dil:** C++ (SKSE64)
- **Kütüphane:** CommonLibSSE-NG
- **Sürüm:** 1.0.0 (Stabil)

## 📥 Kurulum

1. **Address Library for SKSE Plugins** yüklü olduğundan emin olun.
2. `NoFreeService.dll` dosyasını `Data/SKSE/Plugins/` klasörüne kopyalayın.

---

## 🇬🇧 English Summary

**No Free Service** is an SKSE plugin that adds a recruitment cost and weekly wage system for followers in Skyrim.

- **Dynamic Cost:** Calculated based on NPC level.
- **Automatic Menu:** Triggers when starting dialogue with potential followers.
- **Version Support:** Fully compatible with Skyrim AE 1.6.1170.
- **Compatibility:** Designed to work alongside major UI and follower mods.

### Installation
Copy `NoFreeService.dll` to your `Data/SKSE/Plugins/` directory. Requires Address Library.
