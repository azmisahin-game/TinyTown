# TinyTown

**A tiny world built from code.**
*Kodla üretilen küçük bir dünya.*

TinyTown is a small open-source C++ experiment: a procedurally generated
miniature 3D town, and visible agents doing simple tasks inside it. It needs no
game engine, no mandatory AI model, no API key, and no account.

**Türkçe:** [README.md](README.md) (the primary README)

![TinyTown v0.1 running: the town, three agents, and the inspect panel](docs/media/tinytown-v0.1-town.png)

*Screenshot produced by the application itself (`--screenshot`), not a concept
image.*

## Status: v0.1.0 is publishable

We state plainly what exists and what does not.

| Capability | State |
| --- | --- |
| CMake build (C++20, SDL2 + OpenGL) | Working |
| Procedural town from a seed | Working |
| Walkable camera, collision, world bounds | Working |
| At least three agents doing Patrol / Deliver / Survey | Working |
| Grid pathfinding that never crosses a building | Working |
| Turkish interface by default, with English | Working |
| Agent inspect panel, agent list, help overlay | Working |
| Pause, resume, reset world, reset one agent | Working |
| Headless simulation tests | 77 passing |
| Offscreen screenshot (`--screenshot`) | Working |
| Windows and Linux download packages | Working |
| Audio, image/video export, AI | **Not present** (later releases) |

## Download

You do not need to compile the source:

- **Windows x64:** `TinyTown-v0.1.0-windows-x64.zip` — extract and double-click
  `tinytown.exe`. The SDL2 runtime is inside the package.
- **Linux x64:** `TinyTown-v0.1.0-linux-x64.tar.gz` — extract and run
  `./tinytown`.

Packages are published on
[GitHub Releases](https://github.com/azmisahin-game/TinyTown/releases/latest).

The Linux package does **not** bundle SDL2 or OpenGL; they are system
dependencies:

```bash
sudo apt-get install -y libsdl2-2.0-0 libgl1     # Debian/Ubuntu
sudo dnf install -y SDL2 mesa-libGL              # Fedora
sudo pacman -S sdl2 mesa                         # Arch
```

Both packages contain `QUICKSTART_TR.txt`, `QUICKSTART_EN.txt`, `LICENSE`,
`THIRD-PARTY-NOTICES.md` and `RUNTIME-REQUIREMENTS.txt`.

## Build from source

Requirements:

- CMake 3.16 or newer
- A C++20 compiler (verified with GCC 14 and MSVC 2022)
- SDL2 development files, version 2.32.x (zlib licence)
- An OpenGL development library
- Optional: zlib. With it, screenshots are written as PNG; without it, as PPM.

```bash
# Debian/Ubuntu
sudo apt-get install -y cmake g++ libsdl2-dev libgl1-mesa-dev zlib1g-dev

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/tinytown
```

### Building on Windows

CI uses the official SDL2 development archive. Its top-level folder is named
`SDL2-<version>` (not `SDL2-<version>-VC`), and the CMake package config lives
at `<root>\cmake\sdl2-config.cmake`:

```powershell
# Extract SDL2-devel-2.32.4-VC.zip to C:\SDL2
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
      -DSDL2_DIR="C:\SDL2\cmake"
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
copy C:\SDL2\lib\x64\SDL2.dll build\Release\
.\build\Release\tinytown.exe
```

### Build options

| Option | Default | Effect |
| --- | --- | --- |
| `TINYTOWN_BUILD_APP` | `ON` | Build the interactive SDL2 application |
| `TINYTOWN_BUILD_TESTS` | `ON` | Build the headless test binary |

The tests need no SDL, OpenGL, or window, so they run on any build machine:

```bash
cmake -S . -B build -DTINYTOWN_BUILD_APP=OFF
cmake --build build --parallel
./build/tinytown_tests
```

## Run

```bash
./build/tinytown                 # default seed 1337, three agents
./build/tinytown --seed 42 --agents 5
./build/tinytown --lang en       # start with the English interface
```

### Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` or arrow keys | Move |
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
| `H` or `F1` | Toggle the help overlay |
| `F2` | Save a screenshot into `captures/` |
| `Esc` | Release the mouse; press again to quit |

Mouse look is optional. The cursor is never locked, and it is released together
with all held keys when the window loses focus.

### Changing the language

The interface is **Turkish by default**. There are two ways to change it:

1. Press `L` in the application. It changes immediately.
2. Choose at start-up: `--lang en` or `--lang tr`.

Glyphs for the Turkish characters (ç, ğ, ı, İ, ö, ş, ü) were added to the bitmap
font. Text is decoded from UTF-8 into codepoints and is never drawn byte by
byte. Turkish case folding differs from English: `i` becomes dotted capital
`İ`, and dotless `ı` becomes `I`.

### Command-line options

```text
--seed <n>                 world seed (default 1337)
--agents <n>               number of agents (default 3)
--width <px> --height <px> window size (default 1280x720)
--screenshot <path>        run without interaction, write an image, and exit
--screenshot-seconds <s>   simulated seconds before the capture (default 3)
--yaw <radians>            initial camera yaw
--pitch <radians>          initial camera pitch
--camera <x> <z>           initial camera ground position
--eye <metres>             initial eye height (a high value starts overview mode)
--lang <tr|en>             interface language (default tr)
--print-agents             print agent state after a capture
--help                     show this message
```

## What the agents actually do

Each agent runs one of three behaviours, and the panel reports the truthful
current state:

- **Patrol** — walks a loop of sidewalk corners.
- **Deliver** — picks up a package at the depot, carries it to a storefront,
  and returns. The carried package is drawn above the agent, so the panel and
  the world never disagree.
- **Survey** — travels to a point and inspects it.

States: `Idle → Navigating → Working → Completed` or `Failed`. A completed or
failed result stays on screen briefly, then the agent takes a new task. "No
route" and "unknown target" are ordinary outcomes: instead of walking through a
building, the agent records the reason, increments its failure counter, and
moves on.

Verify all of this without opening a window:

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

> Note: `--print-agents` output is a developer log and stays in English. The
> user-facing interface is bilingual.

## Tests

```bash
./build/tinytown_tests
```

The suite covers deterministic seeded generation, pathfinding (reachable,
unreachable, blocked start/goal, out of bounds, detour around a wall,
determinism), agent state transitions, large and negative `dt`, camera
frame-rate independence, session pause/selection/time clamping, font coverage,
and localization.

The strongest check is this: every user-visible string, in both languages, is
verified to consist only of characters the bitmap font can draw. That is what
stops Turkish text from silently turning into broken characters.

See [docs/QUALITY-GATES.md](docs/QUALITY-GATES.md) for the full acceptance list
and the last recorded results.

## Platform support

| Platform | Configured | Compiled | Tests run | Interface launched |
| --- | --- | --- | --- | --- |
| Linux (GCC 14, Debian 13) | Yes | Yes | Yes — 77 tests | Yes, Xvfb + Mesa llvmpipe |
| Windows (MSVC 2022) | Yes (CI) | Yes (CI) | Yes (CI) — 77 tests | **No** |
| macOS | No | No | No | No |

"Launched with Xvfb + Mesa llvmpipe" means the real binary created a real
OpenGL context, drew frames, and wrote a real PNG. It is **not** a claim that a
human used the interface on a desktop.

On Windows the application builds and the tests pass in CI, but a GitHub runner
has no usable display or GPU. The interactive window and its OpenGL rendering
were therefore **not** exercised on Windows; CI only checks process start-up,
argument parsing and the exit code via `tinytown.exe --help`.

## Not included in v0.1.0

To avoid overstating the project:

- No audio, no WAV export, no MP4, no animation frame sequence.
- No LLM, no learning AI, no TTS, no AI image generation. No API key, network
  connection, or model download is required.
- No browser / WebAssembly build. The application is a native SDL2 window using
  compatibility-profile immediate-mode OpenGL, so it does not run in a browser
  as it stands.
- No in-app editor, scripting language, or plugin system.
- No auto-updater. Downloading a release does not update an installed copy.

The agents are simulation agents running fixed rules. They do not use a
language model to make decisions.

## Dependencies and licences

| Dependency | Version | Licence | Why it is needed |
| --- | --- | --- | --- |
| SDL2 | 2.32.4 | zlib | Window, keyboard/mouse input, and the OpenGL context. Writing this by hand per platform is far more code and more risk. |
| OpenGL | system (`libGL`) | driver / Mesa | Drawing the scene. Same approach as the reference prototype: compatibility-profile immediate mode. |
| zlib | 1.3.1 | zlib | Optional. Writes screenshots as PNG. Without it PPM is written and the application keeps working. |

The repository contains no vendored binaries, no downloaded assets, and no model
weights. Every shape in the town is generated from code at runtime. Details:
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

## Repository layout

```text
src/core/      world generation, pathfinding, simulation, camera  (no graphics)
src/app/       main loop, session state, screenshot writing
src/render/    OpenGL renderer (reads state, never decides it)
src/ui/        bitmap font, localization, HUD overlay
tests/         headless tests (no SDL/OpenGL needed)
tools/         packaging and verification scripts
site/          GitHub Pages project site (HTML/CSS/JS, no framework)
reference/     the original Win32/OpenGL prototype, kept as history
docs/          vision, scope, architecture, roadmap, quality gates
```

## Roadmap

The next steps are separate, measurable tasks. None is shown as complete.

**v0.2 — Capturable and shareable media output (planned).**
Deterministic animation frame sequences (`--capture-sequence <dir> --frames <n>
--fps <n>`), code-synthesized ambient audio with volume control, WAV export,
and optional MP4 via FFmpeg. The base application must keep working without
FFmpeg and without an audio device. The generated audio is procedural, not AI
TTS.

**v0.3 — Wider reach and browser evaluation (planned).**
Re-verify the native Windows and Linux packages; assess how independent the C++
core is from platform headers; and evaluate feasibility for a WebGL 2 / GLES
renderer plus an Emscripten build target. No browser build will be claimed
until a working prototype is published and tested.

Full details and acceptance criteria: [docs/ROADMAP.md](docs/ROADMAP.md).

## Why the project exists

The project began as a single Win32/OpenGL C++ file. That file is kept as
history at
[`reference/win32-minicity/main.cpp`](reference/win32-minicity/README.md): it
includes `<windows.h>` directly and cannot be compiled on Linux or macOS as
written. TinyTown keeps its intent — a small town you can walk through, drawn
entirely from code — and rebuilds it on a portable shell.

The design principle:

> The smallest whole thing that works, then only the complexity a proven need
> requires.

Success is measured not by line count, but by whether someone can clone the
repository, build it with a few commands, walk around the town, watch the
agents, understand what one of them is doing, and reset the world.

## Documentation

- [Project vision](docs/PROJECT-VISION.md)
- [Product scope](docs/PRODUCT-SCOPE.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)
- [Quality gates and last results](docs/QUALITY-GATES.md)
- [Release notes — v0.1.0](docs/RELEASE-NOTES-v0.1.0.md)
- [Third-party notices](THIRD-PARTY-NOTICES.md)
- [Türkçe README](README.md)
- [Reference prototype](reference/win32-minicity/README.md)

## Licence

MIT. See [LICENSE](LICENSE).
