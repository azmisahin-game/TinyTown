# Architecture

This document describes the implementation that exists in `src/`, not a plan for
empty layers. It is kept short on purpose.

## What is actually there

```text
src/core/        world generation, pathfinding, simulation, camera
src/app/         main loop, session state, screenshot writing
src/render/      OpenGL renderer
src/ui/          bitmap font, HUD overlay
tests/           headless tests (no SDL, no OpenGL)
```

`tinytown_core` is a static library containing `src/core/` and `src/app/session.cpp`.
It has **no** SDL, OpenGL, or platform headers, and it is what the test binary
links against. The renderer and the SDL shell link the core library in addition
to their own dependencies.

```text
        SDL2 window / input            (src/app/main.cpp)
                  │
                  ▼
        Session: pause, selection,     (src/app/session.cpp — tested)
        fixed-step accumulator
                  │
      ┌───────────┴───────────┐
      ▼                       ▼
  Renderer (GL)           Simulation core            (tested)
  src/render/             src/core/
      │                       │
      │            ┌──────────┴──────────┐
      │            ▼                     ▼
      │      World generation      Pathfinding + agents
      │      src/core/world.cpp    src/core/pathfinding.cpp
      │                            src/core/simulation.cpp
      └───────► reads world + agent state ◄──────┘
```

## Dependency rules that the code actually follows

- `src/core/` includes only the standard library and its own headers. No SDL, no
  OpenGL, no `<windows.h>`, no file I/O.
- The renderer reads world and agent state and draws it. It never mutates the
  simulation and never decides whether a task succeeded.
- `Session` owns pause, agent selection, and the fixed-step accumulator, and it
  contains no SDL types. That is why pause, selection clamping, focus-loss
  clearing, and frame-time clamping are unit-tested rather than hand-checked.
- `main.cpp` is the only file that knows about SDL and OpenGL together. It
  translates events into `Session` calls and then draws.

## Platform shell

- **SDL2 2.32.4** (zlib licence) provides the window, input, and the OpenGL
  context. It was chosen over hand-written per-OS window code because the
  smallest correct cross-platform implementation is one library, not three.
- The window requests an **OpenGL compatibility profile**. The renderer uses
  immediate-mode drawing (`glBegin`/`glVertex3f`), the same technique as the
  reference prototype. This needs no shader compilation and no asset pipeline,
  which is the smallest path to the v0.1 look. It is a deliberate trade-off: a
  core-profile or WebGL renderer would be needed before a browser target, and
  that work is not in v0.1.
- A hidden `SDL_WINDOW_HIDDEN` window still yields a real, renderable OpenGL
  context. That is how `--screenshot` works on a machine with no desktop.

## Simulation contracts (all covered by tests)

- `Simulation::update(dt)` clamps `dt` to `SimConfig::maxStepDt`, so one long
  frame can never teleport agents across the town.
- World generation is deterministic: the same 64-bit seed produces a
  byte-identical walkability grid and identical building/prop placement. The RNG
  is a hand-written SplitMix64 in `src/core/random.hpp` because the standard
  library does not guarantee identical values across compilers and platforms.
- Agents have explicit states: `Idle`, `Navigating`, `Working`, `Completed`,
  `Failed`. Completion is decided from world facts (arrival distance, work
  timer), never from UI text.
- Pathfinding is A* on the walkability grid, 8-connected, without corner
  cutting, followed by a line-of-sight smoothing pass. "No route" is returned
  explicitly, and `findPath` refuses a blocked or out-of-bounds start or goal.
- Collision uses the same grid the pathfinder uses. The grid is expanded by the
  agent radius during generation, so a centre-point test is the correct
  clearance check and movement cannot disagree with the planned route.

## World representation

- One `World` struct holds the seed, bounds, the walkability grid, buildings,
  props, blocks, named landmarks, patrol points, and the spawn point.
- Landmarks are snapped to walkable ground after the grid is finalised, so a
  generated building can never make a destination unreachable by accident.
- Lamps and trees block their own grid cell only. The sidewalk is wide enough
  that the walking lane beside a lamp stays clear, which keeps corridors open
  while still making props solid.

## Rendering and UI

- Art direction: a soft miniature diorama. Muted palette, per-face directional
  shading, flat shading, and light linear fog that starts beyond the town so the
  world stays crisp and only the far ground fades.
- Windows on buildings are drawn as thin boxes on all four faces, with a stable
  hash choosing which are lit, so the pattern is identical every frame.
- The HUD is drawn with an orthographic pass over the scene, using a 5x7 bitmap
  font defined as readable ASCII art in `src/ui/font5x7.cpp`. There is no font
  file and no text library. Glyph shape is unit-tested.

## Screenshots

`glReadPixels` reads the framebuffer; `src/app/image_io.cpp` flips it and writes
either PNG (via zlib, a minimal hand-written encoder) or PPM. No image library is
linked. If zlib is absent at configure time, PNG is refused with a clear message
and PPM is used instead.

## Later output pipeline (not implemented)

Kept as design only, per the roadmap:

- Still capture already works. Deterministic animation capture would advance the
  simulation with a fixed step and write a frame sequence.
- Audio would be a separately rendered WAV stream with a defined timing contract.
- MP4 would be an optional external FFmpeg process, with missing-encoder
  behaviour tested and licensing checked. The app must not crash without it.
- AI adapters would be isolated: model output is a proposed action that must pass
  the same validation the deterministic tasks pass.

## Cross-platform honesty

A shared core is the goal, not proof that every API is portable. Record
separately whether each platform was configured, compiled, tested, and actually
launched. In this repository Linux was configured, compiled, tested, and launched
under Xvfb with software GL. Windows is configured, compiled and tested by CI on
a hosted runner (77/77 tests, zero warnings), but that runner has no usable
display, so the interactive GUI and its OpenGL rendering are still unverified on
Windows. macOS has not been touched.

Two Windows-specific details are worth remembering, because both caused real
failures:

- The official `SDL2-devel-<version>-VC.zip` extracts to a folder named
  `SDL2-<version>` and places its CMake package config in `<root>/cmake`. The
  build passes `SDL2_DIR=<root>/cmake`.
- Because the program defines `SDL_MAIN_HANDLED` and supplies its own `main`, it
  must call `SDL_SetMainReady()` before `SDL_Init()`.


