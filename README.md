# TinyTown

**A tiny world built from code.**  
*Küçük bir dünya; oyun motoru olmadan, kodla üretilir.*

TinyTown, C++ ile prosedürel bir 3B mini dünya oluşturan, bu dünyadaki basit otonom ajanları gözlemlenebilir kılan ve ilerleyen sürümlerde sahneleri paylaşılabilir görsel/işitsel çıktılara dönüştürmeyi hedefleyen küçük bir açık kaynak deneyidir.

Bu proje bir “oyun motoruna karşı savaş” projesi değildir. Amacı, küçük ama gerçek bir ürünün ne kadarını doğrudan kodla, az ve gerekçeli bağımlılıkla yapabileceğimizi göstermek.

## Neden var?

Bir fikir; onlarca araç, model, servis ve yapılandırma gerektirmeden önce çalışan bir çekirdeğe dönüşebilmeli. TinyTown, bunu görünür bir örnekle göstermek istiyor:

- Kodla üretilmiş, stil sahibi bir mini şehir.
- Dünyada belirli hedefleri gerçekleştiren, davranışları anlaşılabilir ajanlar.
- Harici görsel/ses varlıklarına ve bir LLM'e bağımlı olmayan temel deneyim.
- Zaman içinde sahne yakalama, animasyon ve ses dışa aktarma gibi kullanılabilir çıktılar.
- Önce yerel masaüstü; mimari elverdiğinde aynı temel simülasyonu web/WebAssembly'e taşıma hedefi.

## Önemli bir ayrım

C++ kendi başına bir yapay zekâ görüntü modeli veya doğal konuşan bir TTS modeli değildir. TinyTown'ın ilk aşamadaki görselleri **prosedürel olarak** üretilecektir: geometri, renk, ışık, animasyon ve shader'lar kodla oluşturulur. İlk sesler de konuşma değil, kodla sentezlenen ortam sesleridir. Gerçek AI görüntü üretimi, doğal TTS veya LLM kullanımı ancak daha sonra, isteğe bağlı ve ayrı entegrasyonlar olarak değerlendirilebilir.

Bu ayrım projenin iddiasını daha güçlü kılar: temel uygulama bir model indirilmeden açılır; dış modeller eklendiğinde de ana dünya ve simülasyon onlara bağımlı olmaz.

## İlk hedef

**v0.1 — Çalışan ve test edilebilir dünya**

1. Windows ve Linux'ta derlenebilen bir C++ uygulaması.
2. Referans prototipten türetilmiş, gezilebilir küçük bir şehir.
3. Tekrarlanabilir (seed ile üretilebilen) dünya ve temel kamera kontrolleri.
4. En az üç basit ajanın devriye, hedefe gitme veya paket teslimi gibi görevleri.
5. Seçili ajanın hedefini ve durumunu gösteren yalın bir arayüz.
6. Derleme/test adımları belgelenmiş; doğrulanmış CI.

macOS ve tarayıcı/WebAssembly hedefleri sonraki kapılardır; ilk sürümün gerçekte test edilmediği platformlarda çalıştığı iddia edilmeyecektir.

## Hızlı bağlam

- [Proje vizyonu](docs/PROJECT-VISION.md)
- [Ürün kapsamı ve ilk sürüm](docs/PRODUCT-SCOPE.md)
- [Mimari ilkeler](docs/ARCHITECTURE.md)
- [Yol haritası](docs/ROADMAP.md)
- [Kalite ve kabul ölçütleri](docs/QUALITY-GATES.md)
- [OpenHands / coding agent görevi — Türkçe](docs/OPENHANDS-TASK-TR.md)
- [Coding-agent task — English](docs/OPENHANDS-TASK-EN.md)
- [Başlangıç prototipi](reference/win32-minicity/README.md)

## Durum

Bu depo henüz ürünün ilk çalışan sürümünü içermiyor. Bu başlangıç paketi ürün yönünü, ilk kapsamı ve kodlama ajanlarının uyması gereken sınırları tanımlar. İlk uygulama görevi tamamlandığında bu bölüm gerçek build/test sonuçları ve ekran görüntüleriyle güncellenmelidir.

## Tasarım ilkesi

> Önce çalışan en küçük bütün. Sonra kanıtlanmış ihtiyaç kadar karmaşıklık.

TinyTown'ın başarısı satır sayısıyla değil; birinin depoyu klonlayıp uygulamayı gerçekten çalıştırabilmesi, dünyada bir şey yapabilmesi ve elde edilen çıktıyı tekrar kullanabilmesiyle ölçülür.
