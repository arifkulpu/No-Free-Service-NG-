# No Free Service (NG)

Skyrim SE/AE için geliştirilmiş, takipçi (follower) işe alımını daha gerçekçi ve ekonomik hale getiren bir SKSE64 eklentisidir. Artık kimse "bir iyilik" karşılığında sizinle ölüme gelmeyecek!

An SKSE64 plugin for Skyrim SE/AE that makes follower recruitment more realistic and economically challenging. No one will follow you to their death just for "a favor" anymore!

---

## 🇹🇷 Türkçe Açıklama

### 🚀 Özellikler

*   **Dinamik İşe Alım Ücreti:** NPC'lerin seviyelerine göre hesaplanan adil bir başlangıç bedeli.
    *   *Formül:* `Temel Ücret (500) + (NPC Seviyesi * Çarpan (50))`
*   **Haftalık Ödeme Sistemi:** Takipçileriniz artık sadece bir kere değil, haftalık olarak maaş ister.
*   **Otomatik Menü Enjeksiyonu:** Potansiyel bir takipçiyle konuşmaya başladığınız anda diyalog penceresi kapanır ve ödeme menüsü otomatik olarak açılır.
*   **⏱️ Gerçek Zamanlı Koruma ve Süre Aşımı (Yeni!)**:
    *   **İşe Alım Penceresi:** Ödeme yapıldıktan sonra yoldaşı işe almak için **30 saniyelik** gerçek zamanlı bir pencereniz olur. Bu sürede işe alınmazsa yapılan ödeme zaman aşımına uğrar ve yoldaş tekrar para ister.
    *   **Kazara Kovulma Koruması:** Bir yoldaşı kovduğunuzda, **30 saniyelik** bir koruma (tolerans) süresi başlar. Bu süre zarfında yoldaşı ücretsiz olarak tekrar ekibe katabilirsiniz. Kovulma anından itibaren 30 saniye geçtikten sonra yoldaş tekrar işe alınmak için tam ödeme talep eder.
*   **AE 1.6.1170 Desteği:** En güncel Skyrim sürümüyle tam uyumlu, kararlı ve performanslı.
*   **Geniş Mod Uyumluluğu:** 3DNPC, Beyond Skyrim: Bruma ve diğer özel modlu takipçilerle kusursuz çalışır.
*   **Arayüz Dostu:** `SkyrimSoulsRE` ve `Status Indicator Framework` gibi popüler arayüz modlarıyla çakışmayı önlemek için thread-safe `UITask` tabanlı çalışır.
*   **Sorunsuz Kayıt (Save/Load) Sistemi:** Yoldaşların ödeme ve zaman aşımı durumları oyun kaydedildiğinde korunur ve yüklendiğinde kaldığı yerden devam eder.

### ⚙️ Yapılandırma (INI)

Ayarları `Data/SKSE/Plugins/NoFreeService.ini` dosyasından dilediğiniz gibi değiştirebilirsiniz:

```ini
[General]
RecruitmentBaseCost=500       ; Temel işe alım bedeli (Altın)
RecruitmentLevelMultiplier=50 ; Seviye başına eklenen altın miktarı
WeeklyWage=150                ; Haftalık maaş miktarı (Altın)
GracePeriodDuration=30        ; Ödeme geçerliliği ve kovulma koruma süresi (Saniye)
```

### 📥 Kurulum

1.  **Address Library for SKSE Plugins** modunun yüklü olduğundan emin olun.
2.  `NoFreeService.dll` dosyasını `Data/SKSE/Plugins/` klasörüne kopyalayın.

---

## 🇬🇧 English Description

### 🚀 Features

*   **Dynamic Recruitment Cost:** A fair initial recruitment cost calculated dynamically based on the NPC's level.
    *   *Formula:* `Base Cost (500) + (NPC Level * Multiplier (50))`
*   **Weekly Wage System:** Followers aren't paid just once; they demand a weekly wage to keep serving you.
*   **Automatic Menu Injection:** Triggers the recruitment/payment interface automatically the moment you engage in dialogue with a potential follower.
*   **⏱️ Real-Time Protection & Expiration (New!)**:
    *   **Recruitment Window:** Once you pay a follower, you have a **30-second** real-time window to recruit them. If not recruited within this time, the payment expires, and they will ask for money again.
    *   **Accidental Dismissal Protection:** When you dismiss a follower, a **30-second** grace period begins. You can re-hire them for free during this duration. Once 30 seconds pass, they will demand full payment again to be hired.
*   **AE 1.6.1170 Support:** Fully optimized and native support for the latest Skyrim Anniversary Edition runtime.
*   **Wide Mod Compatibility:** Works flawlessly with custom follower mods like 3DNPC, Beyond Skyrim: Bruma, and others.
*   **UI Friendly:** Thread-safe `UITask` implementation ensures full compatibility with UI-modifying plugins like `SkyrimSoulsRE` and `Status Indicator Framework`.
*   **Seamless Persistence:** Follower payment states and real-time timers are fully serialized, making them safe across saves (`Save`/`Load` operations).

### ⚙️ Configuration (INI)

You can customize the plugin settings in `Data/SKSE/Plugins/NoFreeService.ini`:

```ini
[General]
RecruitmentBaseCost=500       ; Base recruitment cost (Gold)
RecruitmentLevelMultiplier=50 ; Additional cost per NPC level
WeeklyWage=150                ; Weekly wage amount (Gold)
GracePeriodDuration=30        ; Payment validity and dismissal grace period (Seconds)
```

### 📥 Installation

1.  Ensure you have **Address Library for SKSE Plugins** installed.
2.  Copy `NoFreeService.dll` to your `Data/SKSE/Plugins/` directory.

---

## 📄 Lisans / License

Bu proje **Arif KULPU**'ya ait özel mülk telif haklarıyla korunmaktadır. Detaylar için [LICENSE.md](file:///c:/Users/pc/Desktop/projeler/No%20Free%20Service/LICENSE.md) dosyasına göz atabilirsiniz.

This project is protected by proprietary copyright owned by **Arif KULPU**. For details, please refer to the [LICENSE.md](file:///c:/Users/pc/Desktop/projeler/No%20Free%20Service/LICENSE.md) file.
