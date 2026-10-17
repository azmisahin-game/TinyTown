# Quality gates and acceptance checklist

Use this before calling any milestone complete. The results below are the
observed state of this repository, not intentions.

## Build and tests

- [x] CMake configure succeeds from a clean build directory.
- [x] Native build succeeds on each platform claimed in the README.
      Linux verified locally. Windows verified by CI (configure, build, ctest):
      run [39009303637](https://github.com/azmisahin-game/TinyTown/actions/runs/39009303637)
      passed with 56/56 tests and zero MSVC warnings.
- [x] Unit tests cover deterministic world generation and agent/task/path logic.
- [x] CI runs the documented build and test commands.
      Both jobs are green as of run `39009303637`. Earlier failures and their
      causes are recorded under "CI history" below.
- [x] The application is launched manually on at least one graphics-capable
      environment for each release candidate.
      Launched under Xvfb with Mesa llvmpipe (real GL context, real frames, real
      PNG). Not a human clicking through a desktop GUI.
- [x] No compiler warnings introduced by the change are ignored without
      explanation. A clean `-Wall -Wextra` build reports zero warnings.

## CI history

| Run | Commit | Linux | Windows | Windows failure point |
| --- | --- | --- | --- | --- |
| [39007442092](https://github.com/azmisahin-game/TinyTown/actions/runs/39007442092) | `831e948` | Passed (47 s) | **Failed** | `Download SDL2 development libraries` |
| [39008987314](https://github.com/azmisahin-game/TinyTown/actions/runs/39008987314) | `00597ca` | Passed | **Failed** | `Confirm the intended SDL2 SDK was resolved` |
| [39009303637](https://github.com/azmisahin-game/TinyTown/actions/runs/39009303637) | `aeef50a` | **Passed** | **Passed** | — |

The final run is the evidence that both jobs are green. Step-level results for
`39009303637`:

Linux — all 10 steps passed: install dependencies, toolchain report, configure,
build, ctest, direct test-binary run, offscreen render, artifact upload. The
offscreen step reported
`/tmp/ci-smoke.png: PNG image data, 1280 x 720, 8-bit/color RGB, non-interlaced`.

Windows — all 12 steps passed: MSVC setup, SDL2 cache restore, SDK verify or
install, configure, resolved-SDK assertion, build, ctest, direct test-binary run,
DLL staging, `--help` start-up check. Observed output:

```text
SDL2 SDK installed and validated at C:\SDL2
CMake resolved SDL2_DIR=C:\SDL2\cmake
Confirmed: CMake used the SDL2 SDK installed by this job
1/1 Test #1: tinytown_core_tests ..............   Passed    0.04 sec
100% tests passed, 0 tests failed out of 1
56 passed, 0 failed, 0 assertion failure(s)
Staged C:\SDL2\lib\x64\SDL2.dll next to the executable
tinytown.exe --help exited with code 0
```

Zero MSVC warnings (`warning C####`) and zero compiler errors were present in
that log.

### First failure: the SDL2 download step

The first Windows run failed at the SDK step, before configure/build/test. The
logged error was:

```text
Move-Item: D:\a\_temp\...ps1:5
Line |
   5 |  Move-Item C:\SDL2-2.32.4-VC C:\SDL2
     |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path 'C:\SDL2-2.32.4-VC' because it does not exist.
```

Cause: the official `SDL2-devel-2.32.4-VC.zip` extracts to a top-level folder
named `SDL2-2.32.4`, not `SDL2-2.32.4-VC`. The workflow assumed the folder name
and moved a path that never existed. The same archive also puts its CMake
package config in `<root>\cmake\sdl2-config.cmake`, not `<root>\lib\cmake\SDL2`,
so the `SDL2_DIR` passed to CMake was wrong as well.

Both URLs (`https://www.libsdl.org/release/...` and the GitHub release asset)
serve a byte-identical archive (SHA-256
`28681dbef9c31a2bb4af6cbda90fdabc27c7415d65b9393da83d5abd12c4a265`, 6,183,659
bytes), so the download URL itself was never the problem.

### Second failure: the resolved-SDK assertion

After the SDK step was fixed, the job reached configure but failed at the new
assertion step with `SDL2_DIR was not recorded in CMakeCache.txt`, even though
configure logged `SDL2 found via SDL2_DIR=C:\SDL2\cmake`.

Cause: the step matched `^SDL2_DIR:PATH=`, but CMake records a value passed with
`-D` as type `UNINITIALIZED`:

```text
$ cmake -S . -B build -DSDL2_DIR=/usr/lib/x86_64-linux-gnu/cmake/SDL2
$ grep SDL2_DIR build/CMakeCache.txt
SDL2_DIR:UNINITIALIZED=/usr/lib/x86_64-linux-gnu/cmake/SDL2
```

### Fixes applied

1. **SDK install rewritten** (`Verify or install SDL2 SDK`): discovers the real
   top-level folder from the archive instead of assuming `SDL2-<version>-VC`,
   validates the required files (`cmake\sdl2-config.cmake`,
   `cmake\sdl2-config-version.cmake`, `include\SDL.h`, `lib\x64\SDL2.lib`,
   `lib\x64\SDL2.dll`), and only then installs. It retries each source three
   times, falls back to the second official URL, and on total failure prints the
   attempted URLs, archive size, discovered folder names and per-file status
   before exiting non-zero. A corrupt or incomplete cache is detected and
   rebuilt rather than trusted.
2. **`SDL2_DIR` corrected** to `<root>\cmake`, which is where the archive
   actually puts `sdl2-config.cmake`.
3. **Resolved SDK is asserted** (`Confirm the intended SDL2 SDK was resolved`):
   `find_package(SDL2)` accepts any SDL2 it can find, so the job reads `SDL2_DIR`
   back out of `CMakeCache.txt` and fails unless it matches the SDK this job
   installed. Without this, the job could pass while linking an unrelated SDL2.
   The check matches any cache entry type, dumps every SDL2 line when the entry
   is missing, and compares with an ordinal case-insensitive prefix test after
   normalising separators.
4. **`SDL_SetMainReady()` added** before `SDL_Init()` in `src/app/main.cpp`. The
   program defines `SDL_MAIN_HANDLED` and supplies its own `main`, and SDL
   documents that such an application must call `SDL_SetMainReady()` first.
5. **Portability includes fixed**: `std::size` in `simulation.cpp` now includes
   `<iterator>`, and `std::ptrdiff_t` in `image_io.cpp` now includes `<cstddef>`.
   GCC supplied these transitively; MSVC is stricter.
6. **CMake reports the resolved SDL2** location at configure time, so a wrong or
   missing SDK is visible in the log instead of surfacing as a link error.

The install logic was executed and validated locally with PowerShell 7.4.6
against the real archive: clean install, cache-hit reuse, corrupt-cache rebuild,
and total-download-failure all behaved as intended (the failure path exited 1
with full diagnostics). The assertion was checked against all five cache entry
types. The PowerShell syntax of every new step was parse-checked.

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

