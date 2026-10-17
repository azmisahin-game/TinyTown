# Quality gates and acceptance checklist

Use this before calling any milestone complete. The results below are the
observed state of this repository, not intentions.

## Build and tests

- [x] CMake configure succeeds from a clean build directory.
- [x] Native build succeeds on each platform claimed in the README.
      Linux verified. Windows is CI-only and **not yet observed passing**, so the
      README claims it as untested.
- [x] Unit tests cover deterministic world generation and agent/task/path logic.
- [x] CI runs the documented build and test commands.
      The workflow is committed. No run has been observed, so it is not treated
      as evidence.
- [x] The application is launched manually on at least one graphics-capable
      environment for each release candidate.
      Launched under Xvfb with Mesa llvmpipe (real GL context, real frames, real
      PNG). Not a human clicking through a desktop GUI.
- [x] No compiler warnings introduced by the change are ignored without
      explanation. A clean `-Wall -Wextra` build reports zero warnings.

## Runtime behaviour

- [x] Window resize and close work.
      Resize re-queries the drawable size and updates the viewport and
      projection. Close events stop the loop.
- [x] Keyboard/mouse input works after losing and regaining focus.
      Focus loss clears all held keys and releases the mouse capture
      (`Session::onFocusLost`, covered by a test).
- [x] Camera movement is frame-rate independent and cannot escape valid world
      bounds. Both are unit-tested.
- [x] Agents do not pass through obstacles; unreachable routes fail safely.
      Verified by tests that walk agents for thousands of steps and assert every
      position is walkable, plus explicit blocked start/goal cases.
- [x] Reset returns the simulation to a known seed/state.
      `Simulation::reset(seed, agents)` is tested to restore an identical
      fingerprint, and `R` does the same from the interface.
- [x] Selecting an agent shows truthful state, target, and task result.
      The panel reads directly from `Agent` fields; `--print-agents` prints the
      same values, so the panel and the log can be compared.
- [x] No network, model download, API key, or background service is required for
      v0.1.
- [x] Mute/volume works if audio exists; the app remains usable without audio.
      No audio exists in v0.1, so there is nothing to mute.

## Documentation and public release

- [x] Build prerequisites and exact commands are written down.
- [x] Supported, tested, and untested platforms are clearly separated.
- [x] README does not advertise unimplemented AI, TTS, or video export.
- [x] A screenshot comes from the actual app, not a generated concept image.
      `docs/media/tinytown-v0.1-town.png` was produced by
      `--screenshot` from the real build.
- [x] Any bundled assets or third-party dependencies have licence attribution.
      SDL2 (zlib), OpenGL (driver/Mesa), zlib (zlib). No assets are bundled.
- [ ] Release archive works on a clean machine or its runtime requirements are
      stated. Not attempted; there is no release archive yet.
- [ ] If GitHub Pages is enabled, it deploys a real, current site. No Pages site
      exists yet.

## Last recorded results

Environment: Debian GNU/Linux 13, GCC 14.2.0, CMake 3.31.6, SDL2 2.32.4,
Mesa 25.0.7 (llvmpipe), zlib 1.3.1.

```text
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
-- Found OpenGL: /usr/lib/x86_64-linux-gnu/libOpenGL.so
-- TinyTown: PNG screenshot support enabled (zlib 1.3.1)
-- Configuring done
-- Generating done
-- Build files have been written to: .../build

$ cmake --build build --parallel
[100%] Built target tinytown          (no warnings)

$ ctest --test-dir build --output-on-failure
1/1 Test #1: tinytown_core_tests ..............   Passed
100% tests passed, 0 tests failed out of 1

$ ./build/tinytown_tests
56 passed, 0 failed, 0 assertion failure(s)
```

Test coverage by suite:

| Suite | Tests | Covers |
| --- | --- | --- |
| World | 8 | seed determinism, structure, walkable landmarks/spawn, blocked footprints, collision |
| Pathfinding | 9 | reachable/unreachable, blocked start/goal, out-of-bounds, wall detour, determinism |
| Simulation | 14 | reset determinism, task/state transitions, large and negative `dt`, obstacle-free motion, invalid targets |
| Camera | 7 | frame-rate independence, bounds, pitch clamp, diagonal speed |
| Session | 11 | pause, selection clamping, frame-time clamp, step budget, focus loss |
| Font | 7 | glyph coverage, row width, digit distinctness, case folding |

## Final report format for coding agents

1. **Implemented:** user-visible capabilities now present.
2. **Files changed:** key paths and purpose.
3. **Commands executed:** exact commands, not paraphrases.
4. **Results:** pass/fail plus meaningful output.
5. **Not verified:** platform/toolchain checks that could not run.
6. **Known limitations:** no optimism disguised as completion.
7. **Next smallest step:** one concrete follow-up task.

