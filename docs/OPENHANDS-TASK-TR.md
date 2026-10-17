# OpenHands için ilk görev — TinyTown v0.1

Bu görevi TinyTown deposunun kök dizininde çalıştır. Önce `AGENTS.md` ve `docs/` altındaki ürün belgelerini oku. `reference/win32-minicity/main.cpp` dosyası, mevcut fikrin tarihî prototipidir; olduğu gibi çapraz platformluymuş gibi sunma.

## Amaç

TinyTown için yalnızca öneri veya boş iskelet değil, mümkün olan en küçük **çalışan ve test edilebilir dikey dilimi** üret. Sonuç, C++ ile oluşturulmuş stil sahibi küçük bir 3B dünya ve bu dünyada basit görevler yapan görünür ajanlar olsun. Ürün ilk sürümde bir AI modeli, oyun motoru veya bulut hizmeti olmadan çalışmalı.

## Çalışma biçimi

1. Depoyu, derleme araçlarını ve referans kodunu incele. Mevcut durumda ne olduğunu kısa not et.
2. Çok platformlu masaüstü için en küçük uygulanabilir yolu seç. CMake kullan. SDL gibi bir bağımlılık seçersen sürümünü, lisansını ve neden gerekli olduğunu belirt. Mevcut Win32 kodunun doğrudan Linux/macOS'a taşınamayacağını hesaba kat.
3. Çok katmanlı bir oyun motoru yazma. Önce pencereyi açan, şehir çizen ve kullanıcıyı hareket ettiren bir uygulamayı tamamla.
4. Ardından seed ile üretilebilen küçük şehir ve en az üç ajan ekle. Ajanların devriye/hedefe gitme/paket teslimi gibi basit, deterministik görevleri olsun. Engelleri dikkate alan küçük bir waypoint veya grid yolu yeterli.
5. Bir ajan seçildiğinde görevini, durumunu ve hedefini gösteren yalın bir panel ekle. Yardım/kontrol bilgileri de görünür olsun.
6. Grafik penceresi açmadan çalışabilen çekirdek simülasyon testleri ekle: deterministik seed, görev durumu geçişleri, geçerli ve geçersiz hedef/yol, büyük `dt` durumu.
7. Gerçek derleme/test komutları belli olduktan sonra Windows ve Linux CI ekle. Platform başına gerçek sonuçları raporla. CI derlemesini GUI'nin o platformda elle çalıştığına kanıt sayma.
8. README ve mimari belgelerini gerçek uygulamaya göre güncelle. Doğrulanmış uygulama ekran görüntüsünü üretmek mümkünse ekle.

## Kısıtlar

- Unity, Unreal, Godot veya başka bir oyun motoru ekleme.
- İlk sürüme React, Node/npm, Python çalışma zamanı, LLM, TTS/AI model ağırlığı, CUDA veya API anahtarı gereksinimi ekleme.
- Yapay zekâ görüntü üretimi, doğal konuşma/TTS ve MP4 dışa aktarmayı ilk göreve sığdırmaya çalışma. Bunlar daha sonraki, ayrı kilometre taşlarıdır.
- Gerçekten kullanmadığın ses, görsel veya model dosyalarını indirme.
- Çalışmayan kod parçalarını tamamlandı diye bırakma. Sahte test, sahte ekran görüntüsü veya uydurulmuş build çıktısı üretme.
- Gereksiz yeniden yazım yapma; önce en küçük çalışır sürümü koru.
- Ağ erişimi, sistem paketi veya araç eksikliği varsa uygulanabilir en iyi ilerlemeyi yap ve engeli açıkça raporla; kullanıcıdan tekrar zaten belgelenmiş bilgileri isteme.

## Kabul ölçütü

Görev sonunda kullanıcı şunları yapabilmeli:

- belgelenmiş komutlarla projeyi temiz bir build dizininde derlemek;
- uygulamayı açıp şehirde hareket etmek;
- en az üç ajanın görevlerini gözlemlemek;
- bir ajanı seçip ne yaptığını anlamak;
- simülasyonu bilinen bir başlangıç durumuna sıfırlamak.

## Son rapor

Cevabını şu başlıklarla bitir: `Implemented`, `Files changed`, `Commands executed`, `Test results`, `Platforms not verified`, `Known limitations`, `Next smallest step`. Çalıştırmadığın hiçbir testi geçmiş gibi yazma. İlk sürüm tamamlanamıyorsa bile derlenip açılan en küçük uygulamayı teslim et ve eksik kalanları açıkça belirt.
