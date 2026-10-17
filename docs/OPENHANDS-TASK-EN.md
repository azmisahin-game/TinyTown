# First OpenHands task — TinyTown v0.1

Run this task from the root of the TinyTown checkout. Read `AGENTS.md` and the product documents under `docs/` first. `reference/win32-minicity/main.cpp` is the historical prototype; do not present it as cross-platform in its current form.

## Goal

Deliver the smallest **working, testable vertical slice**, not just a proposal or empty scaffold: a thoughtfully styled tiny 3D world generated in C++ plus visible agents that perform simple tasks. The first release must run without an AI model, game engine, or cloud service.

## Workflow

1. Inspect the repository, available toolchains, and reference source. Briefly record the initial state.
2. Choose the smallest viable cross-platform desktop approach and use CMake. If you select a dependency such as SDL, explain why it is needed and document its version and license. Account for the fact that the current Win32 source will not compile unchanged on Linux/macOS.
3. Do not build a general-purpose game engine. First finish an application that opens a window, renders the town, and lets the user move.
4. Add a small seeded town and at least three agents with simple deterministic tasks such as patrolling, travelling to a target, or delivering a package. A small waypoint graph or grid pathfinder is sufficient; agents must account for obstacles.
5. Add a simple panel that shows the selected agent's task, state, and target, plus visible help/controls.
6. Add graphics-free core tests for deterministic seeds, task/state transitions, reachable and unreachable routes, and large-delta handling.
7. Once actual build/test commands exist, add Windows and Linux CI. Report real results per platform. A CI compilation must not be claimed as proof that the interactive GUI was manually run on that platform.
8. Update README and architecture docs to match the implementation. Add screenshots captured from the real app if feasible.

## Constraints

- Do not add Unity, Unreal, Godot, or another game engine.
- Do not make React, Node/npm, a Python runtime, an LLM, TTS/AI model weights, CUDA, or API keys mandatory for the first version.
- Do not attempt AI image generation, natural-speech TTS, or MP4 export in this first task. Those are separate later milestones.
- Do not download assets, sound files, or models that the implementation does not actually use.
- Do not leave broken code labelled complete. Do not fabricate tests, screenshots, or build logs.
- Avoid unrelated rewrites; preserve the smallest working version once it exists.
- If network access, packages, or tools are missing, make the best feasible progress and report the blocker clearly. Do not ask the maintainer to repeat details already documented here.

## Acceptance criteria

At the end, the maintainer must be able to:

- build from a clean build directory using documented commands;
- launch the app and move through the town;
- observe at least three agents performing tasks;
- select an agent and understand its current behaviour;
- reset the simulation to a known initial state.

## Final report

Finish with these headings: `Implemented`, `Files changed`, `Commands executed`, `Test results`, `Platforms not verified`, `Known limitations`, and `Next smallest step`. Never report a test as passing unless it actually ran. If the full scope cannot be completed, deliver the smallest app that builds and launches, then list remaining work honestly.
