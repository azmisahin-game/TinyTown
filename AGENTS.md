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
