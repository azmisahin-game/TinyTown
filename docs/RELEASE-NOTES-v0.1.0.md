# TinyTown v0.1.0

Küçük, prosedürel bir 3B şehir ve bu şehirde basit görevler yapan görünür
ajanlar. Oyun motoru yok, zorunlu yapay zekâ modeli yok, API anahtarı yok.

*TinyTown v0.1.0 — a tiny procedural 3D town with visible agents doing simple
tasks. No game engine, no mandatory AI model, no API key.*

---

## Türkçe

### Bu sürümde ne var

- **Prosedürel şehir.** Tohumdan üretilen yol ızgarası, kaldırımlar, farklı
  yükseklik ve renklerde binalar, ışıklı pencereler, park, depo avlusu, sokak
  lambaları, ağaçlar, banklar ve sandıklar. Aynı tohum her zaman aynı şehri
  üretir.
- **Gezilebilir kamera.** Birinci şahıs hareket, hızlı koşma, çarpışma, dünya
  sınırları ve `V` ile genel görünüm.
- **Üç deterministik ajan davranışı.** Devriye, Teslimat ve İnceleme. Ajanlar
  kural tabanlı görev durum makineleridir.
- **Yol bulma.** A* ile ızgara üzerinde yol bulma; binaların içinden geçmez,
  ulaşılamayan hedefte açıkça "yol yok" der.
- **Türkçe varsayılan arayüz.** `L` tuşuyla İngilizceye geçilir. Türkçe
  karakterler (ç, ğ, ı, İ, ö, ş, ü) doğru görüntülenir.
- **İnceleme paneli.** Seçili ajanın görevi, durumu, hedefi, konumu, taşıdığı
  paket ve sonuç metni. Ayrıca ajan listesi, durum satırı ve yardım katmanı.
- **Kontroller.** Duraklat, devam et, dünyayı tohuma sıfırla, yalnızca seçili
  ajanı sıfırla, ekran görüntüsü kaydet.
- **Ekran görüntüsü.** `--screenshot` ile etkileşimsiz çalışıp gerçek PNG
  yazar. zlib yoksa PPM yazar.

### İndirme ve çalıştırma

**Windows x64** — `TinyTown-v0.1.0-windows-x64.zip`

1. ZIP dosyasını bir klasöre çıkarın.
2. `tinytown.exe` dosyasına çift tıklayın.

SDL2 çalışma kütüphanesi (`SDL2.dll`) paketin içindedir. Başka bir şey
kurmanız gerekmez.

**Linux x64** — `TinyTown-v0.1.0-linux-x64.tar.gz`

```bash
tar -xzf TinyTown-v0.1.0-linux-x64.tar.gz
cd TinyTown-v0.1.0-linux-x64
./tinytown
```

Linux paketi SDL2 ve OpenGL kütüphanelerini **içermez**; bunlar sistem
bağımlılığıdır:

```bash
sudo apt-get install -y libsdl2-2.0-0 libgl1     # Debian/Ubuntu
sudo dnf install -y SDL2 mesa-libGL              # Fedora
sudo pacman -S sdl2 mesa                         # Arch
```

Her iki pakette de `QUICKSTART_TR.txt`, `QUICKSTART_EN.txt`, `LICENSE`,
`THIRD-PARTY-NOTICES.md` ve `RUNTIME-REQUIREMENTS.txt` bulunur.

### Kontroller

| Girdi | İşlev |
| --- | --- |
| `W` `A` `S` `D` / ok tuşları | Hareket |
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
| `H` veya `F1` | Yardım katmanı |
| `F2` | `captures/` içine ekran görüntüsü kaydet |
| `Esc` | İmleci bırak, tekrar basınca çık |

### Dürüst kapsam

Bu sürümde **yoktur**:

- LLM, öğrenen yapay zekâ, TTS veya yapay zekâ görüntü üretimi.
- Ses, WAV dışa aktarımı, MP4 veya animasyon karesi dizisi.
- Tarayıcı / WebAssembly sürümü. Uygulama yerel SDL2 penceresi ve
  compatibility-profile immediate-mode OpenGL kullanır.
- Otomatik güncelleme. Release indirmek, kurulu uygulamayı güncellemez.

Ajanlar belirlenmiş kurallarla çalışan simülasyon ajanlarıdır. Karar vermek
için bir dil modeli kullanmazlar.

### Doğrulama

- Linux: temiz CMake yapılandırması, derleme, 77 headless test geçti, Xvfb ve
  Mesa llvmpipe ile gerçek ekran görüntüsü üretildi.
- Windows: MSVC 2022 ile derleme ve aynı 77 test CI'da geçti.
- Windows masaüstü GUI/OpenGL etkileşimi CI runner'ında **test edilmedi**;
  runner'ın kullanılabilir ekranı veya GPU'su yok. Paketlenen `tinytown.exe`
  yalnızca `--help` ile başlatılarak süreç başlangıcı doğrulandı.

---

## English

### What is in this release

- **Procedural town.** A road grid, sidewalks, buildings of varied height and
  colour, lit windows, a park, a depot yard, street lamps, trees, benches and
  crates, all generated from a seed. The same seed always produces the same
  town.
- **Walkable camera.** First-person movement, sprint, collision, world bounds,
  and an overview mode on `V`.
- **Three deterministic agent behaviours.** Patrol, Deliver and Survey. The
  agents are rule-based task state machines.
- **Pathfinding.** A* on the walkability grid. Agents never cross a building,
  and an unreachable target reports "no route" explicitly.
- **Turkish by default.** Press `L` for English. Turkish characters (ç, ğ, ı,
  İ, ö, ş, ü) render correctly.
- **Inspect panel.** The selected agent's task, state, target, position,
  carried package and result text, plus an agent list, a status line and a help
  overlay.
- **Controls.** Pause, resume, reset the world to its seed, reset only the
  selected agent, and save a screenshot.
- **Screenshots.** `--screenshot` runs without interaction and writes a real
  PNG, or a PPM file when zlib is unavailable.

### Download and run

**Windows x64** — `TinyTown-v0.1.0-windows-x64.zip`

1. Extract the ZIP to a folder.
2. Double-click `tinytown.exe`.

The SDL2 runtime (`SDL2.dll`) is included in the package. Nothing else needs to
be installed.

**Linux x64** — `TinyTown-v0.1.0-linux-x64.tar.gz`

```bash
tar -xzf TinyTown-v0.1.0-linux-x64.tar.gz
cd TinyTown-v0.1.0-linux-x64
./tinytown
```

The Linux package does **not** bundle SDL2 or OpenGL; those are system
dependencies:

```bash
sudo apt-get install -y libsdl2-2.0-0 libgl1     # Debian/Ubuntu
sudo dnf install -y SDL2 mesa-libGL              # Fedora
sudo pacman -S sdl2 mesa                         # Arch
```

Both packages contain `QUICKSTART_TR.txt`, `QUICKSTART_EN.txt`, `LICENSE`,
`THIRD-PARTY-NOTICES.md` and `RUNTIME-REQUIREMENTS.txt`.

### Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` / arrow keys | Move |
| `Shift` | Sprint |
| Mouse | Look (press `M` first to capture the cursor) |
| `M` | Capture / release mouse |
| `V` | Overview / street view |
| `Tab` or `1`–`9` | Select agent |
| Left click | Select the agent under the cursor |
| `P` or Space | Pause / resume |
| `R` | Reset world to seed |
| `N` | Reset only the selected agent |
| `L` | Switch interface language (TR / EN) |
| `H` or `F1` | Help overlay |
| `F2` | Save a screenshot into `captures/` |
| `Esc` | Release the mouse, then quit |

### Honest scope

This release does **not** include:

- An LLM, learning AI, TTS, or AI image generation.
- Audio, WAV export, MP4, or animation frame sequences.
- A browser / WebAssembly build. The application is a native SDL2 window using
  compatibility-profile immediate-mode OpenGL.
- An auto-updater. Downloading a release does not update an installed copy.

The agents are simulation agents running fixed rules. They do not use a
language model to make decisions.

### Verification

- Linux: clean CMake configure, build, 77 headless tests passing, and a real
  screenshot rendered under Xvfb with Mesa llvmpipe.
- Windows: build with MSVC 2022 and the same 77 tests passing in CI.
- The Windows desktop GUI and its OpenGL rendering were **not** tested on a CI
  runner, which has no usable display or GPU. The packaged `tinytown.exe` was
  only started with `--help` to verify process start-up.

---

## Roadmap

The next milestones are separate, measurable tasks. They are not part of
v0.1.0 and are not claimed as done.

- **v0.2 — Capturable output.** Deterministic animation frame sequences
  (`--capture-sequence`), code-synthesized ambient audio with mute and volume,
  WAV export, and an optional FFmpeg integration for MP4. The base application
  must keep working with no audio device and no FFmpeg.
- **v0.3 — Wider reach.** Re-verify native Windows and Linux packaging, and
  assess a browser build. The current renderer uses compatibility-profile
  immediate-mode OpenGL, so it will not run in a browser as-is; a WebGL 2 /
  GLES renderer and an Emscripten target would be needed. No browser build is
  claimed until a real one is deployed and tested.

See `docs/ROADMAP.md` for the full plan and acceptance criteria.
