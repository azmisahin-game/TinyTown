# Architecture principles

This is a guide for the first implementation, not a mandate to create empty layers in advance. Prefer the smallest code structure that supports the v0.1 experience and its real tests.

## Suggested dependency direction

```text
Platform shell (window, input, clock, optional audio)
                  │
                  ▼
          Application / frame loop
             │            │
             ▼            ▼
          Renderer     Simulation core
                            │
                 ┌──────────┴──────────┐
                 ▼                     ▼
          World generation       Agent/task logic
                 │                     │
                 └──────► world data ◄─┘
```

The simulation core must be testable without creating a graphics window. It should not call Win32 or OpenGL directly. The renderer reads world state; it should not decide whether an agent's task has succeeded.

## Platform and rendering direction

- Retain the reference source as historical context; it uses Win32 and legacy immediate-mode OpenGL and is not cross-platform as written.
- For the native cross-platform shell, evaluate one small window/input/audio library such as SDL. Select a specific supported version only after verifying current platform support and the chosen renderer path.
- Use a renderer compatible with the actual v0.1 targets. If a later WebAssembly target is important, keep this in mind before relying on legacy desktop-only OpenGL calls.
- Use CMake and document how third-party dependencies are acquired, pinned, and licensed. Avoid requiring a package manager to be manually configured if a reproducible CMake path is practical.
- Keep world generation and agent logic in plain C++ types. A full ECS, reflection system, scene graph framework, or asset pipeline is not justified for three agents and a small town.
- Keep the art asset footprint near zero initially: generated geometry, material parameters, colours, and simple shaders. Any later vendored dependency or asset must have a documented licence.

## Suggested small module boundaries

Names are suggestions. Fewer files are better than artificial interfaces.

- `app`: startup/shutdown, main loop, events, state reset.
- `world`: seed, building/road/waypoint data, world construction.
- `simulation`: time step, agent state machine, tasks, path decisions.
- `render`: camera, geometry, materials, lighting, drawing.
- `ui`: controls and agent inspection overlay (may initially be simple built-in drawing).
- `audio`: optional procedural ambience added after the core works.

## Simulation contracts

- A simulation update takes a bounded `dt` and produces predictable state changes.
- Randomness is seeded and can be replayed when practical.
- Agents have explicit states and valid transitions, e.g. `Idle → Navigating → Working → Completed` or `Failed`.
- Task completion is based on state/world facts, not on UI text or renderer events.
- Pathfinding must handle “no route” explicitly and must never move an agent through a building just to reach a target.
- The core test suite should cover at least agent state transitions, deterministic seeded generation, reachable target and unreachable target cases, and a large-delta/time-step edge case.

## Later output pipeline

Keep this as future design; do not implement all of it in v0.1.

- Still capture: capture the rendered frame using the selected renderer's supported readback path.
- Deterministic animation: advance simulation with a fixed step and capture frames at a declared frame rate.
- Audio: render a separate WAV stream or capture audio with a well-defined timing contract.
- MP4: optional external FFmpeg process or a separately justified media library. Check process launch, error reporting, codec availability, licensing, and output location. The app must not crash if the optional encoder is missing.
- AI adapters: isolate model/service calls from simulation and rendering. Treat all model output as a proposed action that must pass validation before execution.

## Cross-platform honesty

A shared core is the goal, not proof that all APIs are portable. Record separately whether each platform was configured, compiled, and actually launched. Graphics, audio, file paths, packaging, and video encoders may differ by platform.
