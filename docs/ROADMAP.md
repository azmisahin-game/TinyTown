# Roadmap and release gates

Each milestone should end with a runnable artefact and a short record of what was
tested. Do not start a later milestone to compensate for an unfinished earlier one.

## M0 — Inspect and establish a baseline

- Read the current repository, `AGENTS.md`, and `reference/win32-minicity/main.cpp`.
- Confirm the prototype's dependencies and limitations.
- Choose and document the smallest cross-platform native build path.
- Set up CMake, a minimal app, and build instructions.
- Keep the app launchable at the end of the milestone.

**Gate:** a clean configure/build succeeds on at least Linux and Windows CI; local
graphics launch is tested on one real environment. Never describe CI compilation as
a successful interactive run.

> **Status: done on Linux and Windows.** A clean configure, build, test, and
> offscreen launch were performed on Debian 13 with GCC 14, SDL2 2.32.4, and Mesa
> llvmpipe. Windows is compiled and tested by CI (MSVC 2022, 56/56 tests). The
> first Windows CI run failed at the SDL2 download step because the workflow
> assumed the archive's folder name; the install step was rewritten to discover
> and validate the real layout. See `docs/QUALITY-GATES.md` for the run history.
> The interactive GUI has still not been launched on Windows or macOS.

## M1 — TinyTown v0.1: a world with agents

- Build a compact, coherent town with camera movement and collision.
- Add deterministic world generation and reset.
- Add three visible agents and simple navigation/task state machines.
- Add a small inspection overlay and help/controls.
- Add headless simulation tests and cross-platform CI.

**Gate:** a user can launch it, move around, see agents complete tasks, inspect an
agent, and reset the world. README claims match verified behaviour.

> **Status: implemented and verified on Linux.** 56 headless tests pass. The
> acceptance behaviours (build, move, observe three agents, inspect one, reset)
> were exercised through the real binary. See `docs/QUALITY-GATES.md` for the
> recorded commands and results, and the README for what is explicitly *not*
> claimed (Windows GUI, macOS, a human desktop session).

## M2 — Scene as output

- Add an optional synthesized ambient soundscape with mute and volume.
- Add still-image capture.
- Add deterministic time-based capture to an image sequence.
- If practical, write a WAV sidecar synchronized to the capture.

**Gate:** captured files are valid and open in external viewers/players. Errors are
surfaced clearly. The interactive experience remains usable when audio is disabled.

> **Status: partially done.** Still-image capture exists (`--screenshot` writes a
> real PNG, or PPM without zlib). Audio, image sequences, and WAV are not started.

## M3 — Short clips

- Add optional MP4 generation via a clearly documented encoder integration.
- Make frame rate, duration, output path, and render quality explicit.
- Test interrupted recording, missing encoder, write permissions, and unsupported codec.

**Gate:** a generated sample clip plays outside the app and its source settings are
reproducible. Do not bundle codecs without a licensing review.

> **Status: not started.**

## M4 — Public page and browser demo

- Create an accessible project page explaining purpose, architecture, current limitations, controls, and build/test status.
- Produce a real WebAssembly/browser build from the shared core if the renderer and dependencies support it.
- Use GitHub Pages after the deploy workflow can publish a real site/build and smoke-test it.
- Keep native release downloads and browser demo clearly distinguished.

**Gate:** clean browser load, controls, resize, audio-unlock behaviour, WebGL failure
handling, and one fallback/error path are manually checked. Add automated browser
tests when they are reliable.

> **Status: not started.** Note that the v0.1 renderer uses a compatibility-profile
> immediate-mode OpenGL path, which would have to be replaced before a browser
> target is realistic. `docs/PAGE-COPY.md` holds draft copy for the page.

## M5 — Optional AI/media adapters

- Pick one specific user-visible use case before choosing a model or library.
- Design an optional adapter, its capability and resource requirements, and a failure mode.
- Keep the no-model workflow working.
- Document model source, license, disk/RAM/VRAM requirements, hardware tested, and whether network access is used.

**Gate:** disabling or removing the adapter does not break TinyTown's base world,
tests, or releases.

> **Status: not started.** No AI dependency is present in the repository.
