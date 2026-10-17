# TinyTown

**Kodla üretilen küçük bir dünya.**
*A tiny world built from code.*

TinyTown, C++ ile kodla üretilen küçük bir prosedürel 3B şehir ve bu şehirde
basit görevler yapan görünür ajanlardan oluşan küçük bir açık kaynak
deneyidir. Oyun motoru, zorunlu yapay zekâ modeli, API anahtarı veya hesap
gerektirmez.

**English:** [README.en.md](README.en.md)

![TinyTown v0.1 çalışırken: şehir, üç ajan ve inceleme paneli](docs/media/tinytown-v0.1-town.png)

*Ekran görüntüsü uygulamanın kendisi tarafından üretildi
(`--screenshot`), tasarım görseli değildir.*

## Durum: v0.1.0 yayımlanabilir

Bu sürümde ne olduğunu ve ne olmadığını açıkça yazıyoruz.

| Yetenek | Durum |
| --- | --- |
| CMake ile derleme (C++20, SDL2 + OpenGL) | Çalışıyor |
| Tohumdan prosedürel şehir | Çalışıyor |
| Gezilebilir kamera, çarpışma, dünya sınırları | Çalışıyor |
| Devriye / Teslimat / İnceleme yapan en az üç ajan | Çalışıyor |
| Binalardan geçmeyen ızgara yol bulma | Çalışıyor |
| Türkçe varsayılan arayüz + İngilizce seçeneği | Çalışıyor |
| Ajan inceleme paneli, ajan listesi, yardım katmanı | Çalışıyor |
| Duraklat, devam, dünyayı sıfırla, tek ajanı sıfırla | Çalışıyor |
| Penceresiz (headless) simülasyon testleri | 77 test geçiyor |
| Etkileşimsiz ekran görüntüsü (`--screenshot`) | Çalışıyor |
| Windows ve Linux indirilebilir paketleri | Çalışıyor |
| Ses, görüntü/video dışa aktarma, yapay zekâ | **Yok** (sonraki sürümler) |

## İndirme

Kaynak kodu derlemeniz gerekmez:

- **Windows x64:** `TinyTown-v0.1.0-windows-x64.zip` — arşivi çıkarın ve
  `tinytown.exe` dosyasına çift tıklayın. SDL2 kütüphanesi paketin içindedir.
- **Linux x64:** `TinyTown-v0.1.0-linux-x64.tar.gz` — arşivi çıkarıp
  `./tinytown` çalıştırın.

Paketler
[GitHub Releases](https://github.com/azmisahin-game/TinyTown/releases/latest)
sayfasında yayımlanır.

Linux paketi SDL2 ve OpenGL kütüphanelerini **içermez**; bunlar sistem
bağımlılığıdır:

```bash
sudo apt-get install -y libsdl2-2.0-0 libgl1     # Debian/Ubuntu
sudo dnf install -y SDL2 mesa-libGL              # Fedora
sudo pacman -S sdl2 mesa                         # Arch
```

Her iki pakette de `QUICKSTART_TR.txt`, `QUICKSTART_EN.txt`, `LICENSE`,
`THIRD-PARTY-NOTICES.md` ve `RUNTIME-REQUIREMENTS.txt` bulunur.

## Kaynaktan derleme

Gereksinimler:

- CMake 3.16 veya üzeri
- C++20 derleyicisi (GCC 14 ve MSVC 2022 ile doğrulandı)
- SDL2 geliştirme dosyaları, sürüm 2.32.x (zlib lisansı)
- OpenGL geliştirme kütüphanesi
- İsteğe bağlı: zlib. Varsa ekran görüntüleri PNG yazılır, yoksa PPM.

```bash
# Debian/Ubuntu
sudo apt-get install -y cmake g++ libsdl2-dev libgl1-mesa-dev zlib1g-dev

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/tinytown
```

### Windows'ta derleme

CI, resmî SDL2 geliştirme arşivini kullanır. Arşivin üst klasörü
`SDL2-<sürüm>` adındadır (`SDL2-<sürüm>-VC` değil) ve CMake paket dosyası
`<kök>\cmake\sdl2-config.cmake` yolundadır:

```powershell
# SDL2-devel-2.32.4-VC.zip arşivini C:\SDL2 klasörüne çıkarın
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
      -DSDL2_DIR="C:\SDL2\cmake"
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
copy C:\SDL2\lib\x64\SDL2.dll build\Release\
.\build\Release\tinytown.exe
```

### Derleme seçenekleri

| Seçenek | Varsayılan | Etkisi |
| --- | --- | --- |
| `TINYTOWN_BUILD_APP` | `ON` | Etkileşimli SDL2 uygulamasını derler |
| `TINYTOWN_BUILD_TESTS` | `ON` | Penceresiz test ikilisini derler |

Testler SDL, OpenGL veya pencere gerektirmez; her derleme makinesinde çalışır:

```bash
cmake -S . -B build -DTINYTOWN_BUILD_APP=OFF
cmake --build build --parallel
./build/tinytown_tests
```

## Çalıştırma

```bash
./build/tinytown                 # varsayılan tohum 1337, üç ajan
./build/tinytown --seed 42 --agents 5
./build/tinytown --lang en       # arayüzü İngilizce başlat
```

### Kontroller

| Girdi | İşlev |
| --- | --- |
| `W` `A` `S` `D` veya ok tuşları | Hareket |
| `Shift` | Hızlı koş |
| Fare | Bakış (önce `M` ile imleci alın) |
| `M` | İmleci al / bırak |
| `V` | Genel / sokak görünümü |
| `Tab` veya `1`–`9` | Ajan seç |
| Sol tık | İmlecin altındaki ajanı seç |
| `P` veya Boşluk | Duraklat / devam |
| `R` | Dünyayı tohuma sıfırla |
| `N` | Yalnızca seçili ajanı sıfırla |
| `L` | Arayüz dilini değiştir (TR / EN) |
| `H` veya `F1` | Yardım katmanını aç / kapa |
| `F2` | `captures/` içine ekran görüntüsü kaydet |
| `Esc` | İmleci bırak; bırakılmışken tekrar basınca çık |

Fare ile bakış isteğe bağlıdır. İmleç asla kilitlenmez ve odak kaybında tüm
basılı tuşlarla birlikte serbest bırakılır.

### Dil değiştirme

Arayüz varsayılan olarak **Türkçedir**. İki yol vardır:

1. Uygulama içinde `L` tuşuna basın. Anında değişir.
2. Başlangıçta seçin: `--lang en` veya `--lang tr`.

Türkçe karakterler (ç, ğ, ı, İ, ö, ş, ü) için bit eşlem yazı tipine glifler
eklendi. Metin UTF-8'den kod noktalarına çözülür; bayt bayt çizilmez. Türkçe
büyük harf katlama İngilizceden farklıdır: `i` harfi noktalı büyük `İ` olur,
noktasız `ı` harfi `I` olur.

### Komut satırı seçenekleri

```text
--seed <n>                 dünya tohumu (varsayılan 1337)
--agents <n>               ajan sayısı (varsayılan 3)
--width <px> --height <px> pencere boyutu (varsayılan 1280x720)
--screenshot <yol>         etkileşimsiz çalış, görüntü yaz ve çık
--screenshot-seconds <s>   yakalamadan önce geçen simülasyon saniyesi (3)
--yaw <radyan>             başlangıç kamera yönü
--pitch <radyan>           başlangıç kamera eğimi
--camera <x> <z>           başlangıç kamera konumu
--eye <metre>              başlangıç göz yüksekliği (büyük değer genel görünüm)
--lang <tr|en>             arayüz dili (varsayılan tr)
--print-agents             yakalamadan sonra ajan durumunu yazdır
--help                     bu mesajı göster
```

## Ajanlar gerçekte ne yapar

Her ajan üç davranıştan birini çalıştırır ve panel gerçek durumu gösterir:

- **Devriye** — kaldırım köşelerinden oluşan bir döngüde yürür.
- **Teslimat** — depodan paket alır, bir dükkânın önüne götürür ve döner.
  Taşınan paket ajanın üstünde çizilir, yani panel ile dünya çelişmez.
- **İnceleme** — bir noktaya gider ve orayı inceler.

Durumlar: `Bekliyor → Hedefe gidiyor → Görevde → Tamamlandı` veya
`Başarısız`. Tamamlanan ya da başarısız sonuç kısa süre ekranda kalır, sonra
ajan yeni görev alır. "Yol yok" ve "bilinmeyen hedef" olağan sonuçlardır: ajan
binanın içinden geçmek yerine nedeni kaydeder, başarısızlık sayacını artırır
ve yeni göreve geçer.

Tüm bunları pencere açmadan doğrulayabilirsiniz:

```bash
./build/tinytown --screenshot /tmp/town.png --screenshot-seconds 14 \
    --camera 6 28 --yaw 0.35 --pitch -0.40 --eye 14 --print-agents
```

```text
Wrote /tmp/town.png (1280x720)
--- agent state at capture (t=14.02s) ---
  #0 Ada    task=Patrol   state=Completed  target=Patrol point 1 at=(-19.1, -19.0) done=1 failed=0 carry=nothing
  #1 Bora   task=Deliver  state=Navigating target=Depot (pickup) at=(-9.4, 12.0) done=2 failed=0 carry=nothing
  #2 Ceren  task=Survey   state=Working    target=Park Plaza     at=(-11.7, -11.7) done=1 failed=0 carry=nothing
```

> Not: `--print-agents` çıktısı geliştirici günlüğüdür ve İngilizce kalır.
> Kullanıcıya gösterilen arayüz iki dillidir.

## Testler

```bash
./build/tinytown_tests
```

Test paketi; deterministik tohumla üretimi, yol bulmayı (ulaşılabilir,
ulaşılamaz, engelli başlangıç/hedef, sınır dışı, duvar etrafından dolaşma,
determinizm), ajan durum geçişlerini, büyük ve negatif `dt` davranışını,
kamera çerçeve hızı bağımsızlığını, oturum duraklatma/seçim/zaman kırpmayı,
yazı tipi kapsamını ve yerelleştirmeyi kapsar.

En güçlü kontrol şudur: her kullanıcı arayüzü metni, her iki dilde, yazı
tipinin çizebildiği karakterlerden oluştuğu doğrulanır. Bu, Türkçe metnin
sessizce bozuk karakterlere dönüşmesini engeller.

Tam kabul listesi ve son kaydedilen sonuçlar için
[docs/QUALITY-GATES.md](docs/QUALITY-GATES.md) dosyasına bakın.

## Platform desteği

| Platform | Yapılandırma | Derleme | Testler | Arayüz açıldı |
| --- | --- | --- | --- | --- |
| Linux (GCC 14, Debian 13) | Evet | Evet | Evet — 77 test | Evet, Xvfb + Mesa llvmpipe |
| Windows (MSVC 2022) | Evet (CI) | Evet (CI) | Evet (CI) — 77 test | **Hayır** |
| macOS | Hayır | Hayır | Hayır | Hayır |

"Xvfb + Mesa llvmpipe ile açıldı" şu demektir: gerçek ikili dosya gerçek bir
OpenGL bağlamı oluşturdu, kareleri çizdi ve gerçek bir PNG yazdı. Bu, bir
insanın masaüstünde arayüzü tıklayarak kullandığı iddiası **değildir**.

Windows'ta uygulama CI'da derlenir ve testler geçer, ancak GitHub runner'ında
kullanılabilir ekran veya GPU yoktur. Bu yüzden etkileşimli pencere ve OpenGL
çizimi Windows'ta **denenmedi**; CI yalnızca `tinytown.exe --help` ile süreç
başlangıcını, argüman ayrıştırmayı ve çıkış kodunu kontrol eder.

## v0.1.0 kapsamında olmayanlar

Projeyi olduğundan fazla göstermemek için açıkça yazıyoruz:

- Ses, WAV dışa aktarımı, MP4 veya animasyon karesi dizisi yoktur.
- LLM, öğrenen yapay zekâ, TTS, yapay zekâ görüntü üretimi yoktur. API
  anahtarı, ağ bağlantısı veya model indirmesi gerekmez.
- Tarayıcı / WebAssembly sürümü yoktur. Uygulama yerel bir SDL2 penceresi ve
  compatibility-profile immediate-mode OpenGL kullanır; bu hâliyle tarayıcıda
  çalışmaz.
- Uygulama içi düzenleyici, betik dili, eklenti sistemi yoktur.
- Otomatik güncelleme yoktur. Release indirmek, kurulu bir kopyayı güncellemez.

Ajanlar belirlenmiş kurallarla çalışan simülasyon ajanlarıdır. Karar vermek
için bir dil modeli kullanmazlar.

## Bağımlılıklar ve lisanslar

| Bağımlılık | Sürüm | Lisans | Neden gerekli |
| --- | --- | --- | --- |
| SDL2 | 2.32.4 | zlib | Pencere, klavye/fare girdisi ve OpenGL bağlamı. Bunu her platform için elle yazmak çok daha fazla kod ve risk demektir. |
| OpenGL | sistem (`libGL`) | sürücü / Mesa | Sahneyi çizmek için. Referans prototiple aynı yaklaşım: compatibility-profile immediate mode. |
| zlib | 1.3.1 | zlib | İsteğe bağlı. Ekran görüntüsünü PNG yazar. Yoksa PPM yazılır ve uygulama çalışmaya devam eder. |

Depoda gömülü ikili dosya, indirilmiş varlık veya model ağırlığı yoktur.
Şehirdeki her şekil çalışma anında kodla üretilir. Ayrıntılar:
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

## Depo düzeni

```text
src/core/      dünya üretimi, yol bulma, simülasyon, kamera  (grafik yok)
src/app/       ana döngü, oturum durumu, ekran görüntüsü
src/render/    OpenGL görüntüleyici (durumu okur, karar vermez)
src/ui/        bit eşlem yazı tipi, yerelleştirme, HUD katmanı
tests/         penceresiz testler (SDL/OpenGL gerekmez)
tools/         paketleme ve doğrulama betikleri
site/          GitHub Pages proje sayfası (HTML/CSS/JS, framework yok)
reference/     özgün Win32/OpenGL prototipi, tarih olarak
docs/          vizyon, kapsam, mimari, yol haritası, kalite kapıları
```

## Yol haritası

Sonraki adımlar ayrı ve ölçülebilir görevlerdir. Hiçbiri tamamlanmış gibi
gösterilmez.

**v0.2 — Üretim ve paylaşılabilir medya çıktıları (planlandı).**
Deterministik animasyon karesi dizisi (`--capture-sequence <dizin> --frames <n>
--fps <n>`), kodla sentezlenen ortam sesi ve ses düzeyi kontrolü, WAV dışa
aktarımı, isteğe bağlı FFmpeg ile MP4. Temel uygulama FFmpeg olmadan ve ses
aygıtı olmadan çalışmaya devam etmelidir. Üretilen ses prosedüreldir, yapay
zekâ TTS değildir.

**v0.3 — Daha geniş erişim ve tarayıcı değerlendirmesi (planlandı).**
Yerel Windows ve Linux paketlerinin yeniden doğrulanması; C++ çekirdeğin
platform başlıklarından bağımsızlığının değerlendirilmesi; WebGL 2 / GLES
uyumlu yeni bir görüntüleyici ve Emscripten derleme hedefi için fizibilite.
Çalışan bir tarayıcı prototipi yayımlanıp denenmeden "tarayıcı sürümü var"
denmeyecek.

Ayrıntılar ve kabul koşulları: [docs/ROADMAP.md](docs/ROADMAP.md).

## Proje neden var

Proje tek bir Win32/OpenGL C++ dosyası olarak başladı. O dosya
[`reference/win32-minicity/main.cpp`](reference/win32-minicity/README.md)
yolunda tarih olarak saklanır: doğrudan `<windows.h>` içerir ve yazıldığı
hâliyle Linux veya macOS'ta derlenemez. TinyTown onun amacını korur — kodla
çizilen, içinde yürüyebileceğiniz küçük bir şehir — ve taşınabilir bir kabuk
üzerine yeniden kurar.

Tasarım ilkesi:

> Önce çalışan en küçük bütün. Sonra kanıtlanmış ihtiyaç kadar karmaşıklık.

Başarı, satır sayısıyla değil; birinin depoyu klonlayıp birkaç komutla
derleyebilmesi, şehirde gezebilmesi, ajanları izleyebilmesi, birinin ne
yaptığını anlayabilmesi ve dünyayı sıfırlayabilmesiyle ölçülür.

## Belgeler

- [Proje vizyonu](docs/PROJECT-VISION.md)
- [Ürün kapsamı](docs/PRODUCT-SCOPE.md)
- [Mimari](docs/ARCHITECTURE.md)
- [Yol haritası](docs/ROADMAP.md)
- [Kalite kapıları ve son sonuçlar](docs/QUALITY-GATES.md)
- [Sürüm notları — v0.1.0](docs/RELEASE-NOTES-v0.1.0.md)
- [Üçüncü taraf bildirimleri](THIRD-PARTY-NOTICES.md)
- [English README](README.en.md)
- [Başlangıç prototipi](reference/win32-minicity/README.md)

## Lisans

MIT. Ayrıntılar: [LICENSE](LICENSE).
