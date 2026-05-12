# No Free Service (Skyrim SE Plugin)

Skyrim'de "Bedava hizmet yok!" felsefesiyle yoldaşlık sistemini ekonomiye bağlayan bir SKSE eklentisidir. Artık en yakın dostunuz bile olsa, hizmeti için bir bedel ödemeniz gerekecek.

## 🚀 Özellikler

- **İnteraktif İşe Alım Menüsü:** Bir yoldaş adayıyla konuşmaya başladığınız anda otomatik olarak hizmet bedeli menüsü açılır.
- **Hizmet Bedeli Hesaplama:** NPC'nin seviyesine göre dinamik olarak hesaplanan işe alım ücreti (Min: 200 Altın + Seviye Başı 20 Altın).
- **Haftalık Maaş Sistemi:** Takipçileriniz her hafta 150 Altın maaş talep eder. Ödenmezse hizmeti bırakırlar.
- **Zorunlu Nötrleştirme:** Ödeme yapılmadığı sürece potansiyel yoldaşlar (modla eklenenler dahil) size karşı "Nötr" kalır, müttefik (Ally) olmazlar.
- **Akıllı Filtreleme:** Tüccarlar, dükkan sahipleri ve sıradan kasabalılar otomatik olarak filtrelenir; sadece gerçek yoldaş adaylarından para istenir.
- **NFF Desteği:** Netherin Follower Framework ve diğer büyük yoldaş modlarıyla tam uyumludur.
- **Koruma Süresi (Grace Period):** Ödeme yaptıktan sonra 30 saniye boyunca kayıt korunur, diyalogdan çıksanız bile tekrar para istenmez.

## 🛠️ Teknik Detaylar

- **Framework:** CommonLibSSE-NG (1.6.1170+ uyumlu).
- **Bellek Yönetimi:** `EXCEPTION_ACCESS_VIOLATION` hatalarını önlemek için güvenli bellek erişim protokolleri.
- **Zamanlama:** Gerçek zamanlı polling (diyalog sırasında frame-perfect) ve oyun saati entegrasyonu.

## 📥 Kurulum

1.  **Address Library for SKSE Plugins** yüklü olduğundan emin olun.
2.  `NoFreeService.dll` dosyasını `Data/SKSE/Plugins/` klasörüne kopyalayın.
3.  Konuşmaya başlayın ve bedelini ödeyin!

---

## 📜 Krediler

- **Geliştiriciler:** Arif KULPU
- **Altyapı:** CommonLibSSE-NG & SKSE Team

---

## ⚖️ Lisans

Copyright (c) 2026 Arif KULPU. Tüm Hakları Saklıdır.
