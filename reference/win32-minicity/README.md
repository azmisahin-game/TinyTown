# Reference prototype: Win32 MiniCity

`main.cpp` is the source supplied for the original experiment. It creates a Win32 window, initializes legacy OpenGL, procedurally places simple buildings, draws roads/windows/a car from cubes, and implements first-person movement and rudimentary collision checks.

## How to use it

- Treat it as an executable design reference for the first world, camera, and movement behaviour.
- Do not treat it as the final architecture or as a portable source file: it directly includes `<windows.h>`, uses Win32 window/input APIs, and draws through legacy immediate-mode OpenGL (`glBegin`/`glVertex3f`).
- Preserve the project's intent and the useful behaviour, not every implementation detail. A cross-platform shell/renderer may require a careful port.
- Avoid replacing it with a large game engine just to gain portability.

The prototype is included so future agents can understand the origin of TinyTown without needing the original local machine. This reference copy has not been modified as part of the starter kit.
