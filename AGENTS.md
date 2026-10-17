# Instructions for coding agents

Read this file and `docs/PROJECT-VISION.md`, `docs/PRODUCT-SCOPE.md`, and `docs/QUALITY-GATES.md` before changing code.

## Purpose

TinyTown is a compact C++ procedural-world and agent-simulation product. It should be useful and pleasant to run, not just an architecture demonstration. Its central proof is that a small, thoughtfully built application can produce a visual world, simulate behaviour, and later export media without requiring a game engine or a mandatory AI model.

## Non-negotiable principles

1. **Build a working vertical slice.** Do not spend the first milestone creating a framework, plugin system, editor, or generic engine.
2. **No game engine in v0.1.** Do not add Unity, Unreal, Godot, or a similar engine.
3. **No mandatory AI service/model.** The first version must run without API keys, Python, downloaded model weights, CUDA, a server account, or a network connection after build/runtime files are present.
4. **Keep dependencies small and justified.** Before adding a dependency, state which concrete problem it solves and why a smaller system API or existing dependency is insufficient. Pin versions and document licenses.
5. **Preserve platform-independent simulation logic.** World generation, agent state, task logic, and path decisions should not depend on Windows-only APIs or graphics calls.
6. **Do not over-abstract.** Create modules/interfaces only where they reduce current complexity, enable a real target (e.g. native/web), or make an important behaviour testable.
7. **Be honest about validation.** Never say “works on Windows/Linux/macOS/web” unless an actual build or test ran on that target. Report missing toolchains and unrun tests explicitly.
8. **No unrelated rewrite.** Keep the first milestone narrow and protect a working build once it exists.
9. **Document reality, not intentions as facts.** README feature claims must correspond to implemented, verified behaviour. Mark planned features as planned.
10. **Do not silently add large AI/video dependencies.** Real AI image generation, speech synthesis, LLMs, or MP4 encoding belong behind optional later integrations.

## Engineering defaults

- Target C++20 unless a documented toolchain constraint justifies otherwise.
- Use CMake for reproducible native builds.
- Prefer one well-supported cross-platform window/input/audio layer plus a small renderer over custom platform code for every OS. SDL is a candidate, not a religion: verify the current API and choose the minimum viable dependency set.
- Keep core simulation testable without opening a graphics window.
- Use deterministic seeds for procedural world generation and agent schedules where possible.
- Make time-step handling explicit; cap large frame deltas so returning from a paused/background window does not teleport the simulation.
- Fail gracefully if optional output tooling is missing.
- Keyboard and mouse controls must be documented and must not depend on mouse cursor warping that traps the user unexpectedly.
- Respect close events, focus loss, resize, reduced motion when applicable, and audio mute/volume.

## Required agent workflow

1. Inspect the current checkout and reference prototype before editing.
2. Summarize existing state and choose the smallest implementation path; do not ask the maintainer to repeat context already in these files.
3. Implement a complete end-to-end slice rather than stubs for every future feature.
4. Add tests for deterministic core behaviour and create CI only when the build commands are real.
5. Run all feasible configure/build/test commands. Capture exact results.
6. Update README/docs/screenshots based on what exists.
7. Finish with: files changed, actual commands run, pass/fail results, platforms not tested, known limitations, and the next smallest task.

## Language and user-facing copy

Technical identifiers and source comments may be English. The project README and user-visible instructions should be clear; keep the Turkish description while a concise English project summary can be included for discoverability.

## Repository-specific knowledge (v0.1)

Build commands that are known to work:

```bash
# Debian/Ubuntu dependencies
sudo apt-get install -y cmake g++ libsdl2-dev libgl1-mesa-dev zlib1g-dev

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/tinytown_tests            # 56 tests, no SDL/GL needed
```

Headless graphics on a machine with no desktop (this is how screenshots and CI
smoke runs work — Xvfb gives a real X display and Mesa llvmpipe gives a real
software OpenGL context):

```bash
sudo apt-get install -y xvfb libgl1-mesa-dri
xvfb-run -a -s "-screen 0 1280x720x24" ./build/tinytown \
    --screenshot /tmp/town.png --screenshot-seconds 14 \
    --camera 6 28 --yaw 0.35 --pitch -0.40 --eye 14 --print-agents
```

`--print-agents` prints each agent's task, state, target, position and counters.
Use it whenever a change touches simulation behaviour, so the claims in a report
can be checked against real output rather than inferred from the picture.

Layout rules worth preserving:

- `src/core/` and `src/app/session.cpp` are the `tinytown_core` library. They must
  stay free of SDL, OpenGL and platform headers so the tests stay headless.
- `src/render/` and `src/app/main.cpp` are the only places that touch GL/SDL.
- The renderer reads simulation state and must never decide task outcomes.
- Determinism is a product contract: world generation uses the hand-written RNG
  in `src/core/random.hpp`, not `<random>`, because the standard library does not
  guarantee identical values across compilers. Do not swap it without re-checking
  the determinism tests.
- Agent movement and the pathfinder share the same walkability grid. The grid is
  expanded by the agent radius at generation time, so collision is a centre-point
  test. Changing one without the other will let agents clip through buildings.
- `docs/media/tinytown-v0.1-town.png` must be regenerated with `--screenshot`
  whenever the look changes. Never add a hand-made or concept image.

Windows build facts that cost real debugging time:

- The official `SDL2-devel-<version>-VC.zip` extracts to a top-level folder named
  `SDL2-<version>`, **not** `SDL2-<version>-VC`, and its CMake package config is
  at `<root>/cmake/sdl2-config.cmake`, **not** `<root>/lib/cmake/SDL2`. Pass
  `-DSDL2_DIR=<root>/cmake`.
- CMake records a value passed with `-D` as cache type `UNINITIALIZED`, not
  `PATH`. Match `^SDL2_DIR:` rather than `^SDL2_DIR:PATH=` when reading
  `CMakeCache.txt`.
- `src/app/main.cpp` defines `SDL_MAIN_HANDLED` and supplies its own `main`, so it
  must call `SDL_SetMainReady()` before `SDL_Init()`. Removing that call breaks
  the Windows build at run time, not compile time.
- `find_package(SDL2)` accepts any SDL2 it can find. The Windows CI job reads
  `SDL2_DIR` back from `CMakeCache.txt` and fails if it is not the SDK the job
  installed, so a job cannot pass while linking an unrelated SDL2.
- Include what you use: MSVC does not supply `std::size` without `<iterator>` or
  `std::ptrdiff_t` without `<cstddef>`, even though GCC does transitively.
- PowerShell validation is possible without Windows: install PowerShell 7 for
  Linux and parse-check or run the workflow scripts with
  `DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1`.

Last verified CI run for both jobs (Linux and Windows, 56/56 tests each):
<https://github.com/azmisahin-game/TinyTown/actions/runs/39009303637>



