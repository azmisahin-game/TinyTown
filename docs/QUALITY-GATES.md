# Quality gates and acceptance checklist

Use this before calling any milestone complete.

## Build and tests

- [ ] CMake configure succeeds from a clean build directory.
- [ ] Native build succeeds on each platform claimed in the README.
- [ ] Unit tests cover deterministic world generation and agent/task/path logic.
- [ ] CI runs the documented build and test commands; workflow names are not treated as evidence unless the run succeeded.
- [ ] The application is launched manually on at least one graphics-capable environment for each release candidate.
- [ ] No compiler warnings introduced by the change are ignored without explanation.

## Runtime behaviour

- [ ] Window resize and close work.
- [ ] Keyboard/mouse input works after losing and regaining focus.
- [ ] Camera movement is frame-rate independent and cannot escape valid world bounds.
- [ ] Agents do not pass through obstacles; unreachable routes fail safely.
- [ ] Reset returns the simulation to a known seed/state.
- [ ] Selecting an agent shows truthful state, target, and task result.
- [ ] No network, model download, API key, or background service is required for v0.1.
- [ ] Mute/volume works if audio exists; the app remains usable without audio.

## Documentation and public release

- [ ] Build prerequisites and exact commands are written down.
- [ ] Supported, tested, and untested platforms are clearly separated.
- [ ] README does not advertise unimplemented AI, TTS, or video export.
- [ ] A screenshot comes from the actual app, not a generated concept image.
- [ ] Any bundled assets or third-party dependencies have license attribution.
- [ ] Release archive works on a clean machine or its runtime requirements are stated.
- [ ] If GitHub Pages is enabled, it deploys a real, current site; it never reports success after a failed build.

## Final report format for coding agents

1. **Implemented:** user-visible capabilities now present.
2. **Files changed:** key paths and purpose.
3. **Commands executed:** exact commands, not paraphrases.
4. **Results:** pass/fail plus meaningful output.
5. **Not verified:** platform/toolchain checks that could not run.
6. **Known limitations:** no optimism disguised as completion.
7. **Next smallest step:** one concrete follow-up task.
