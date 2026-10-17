// TinyTown v0.1 — entry point.
//
// Responsibilities, kept deliberately small:
//   * open a window and an OpenGL compatibility context (SDL2)
//   * translate input into camera/simulation commands
//   * run a fixed-step simulation with a bounded frame delta
//   * hand the world and agents to the renderer, then draw the overlay
//
// No game engine, no scripting layer, no asset pipeline, no network access.

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "app/image_io.hpp"
#include "app/session.hpp"
#include "core/camera.hpp"
#include "core/simulation.hpp"
#include "render/renderer.hpp"
#include "ui/hud.hpp"
#include "ui/localization.hpp"

namespace {

constexpr int kDefaultWidth = 1280;
constexpr int kDefaultHeight = 720;
constexpr float kFixedStep = 1.0f / 60.0f;
constexpr float kMaxFrameDelta = 0.25f;   // a longer stall is treated as a pause
constexpr float kMouseSensitivity = 0.0022f;

struct Options {
    int width = kDefaultWidth;
    int height = kDefaultHeight;
    int agents = 3;
    uint64_t seed = 1337;
    bool screenshotMode = false;
    std::string screenshotPath;
    float screenshotSeconds = 3.0f;
    bool showHelp = false;
    bool printAgents = false;
    // Camera overrides. Used by the screenshot command to produce a stable,
    // documented viewpoint, and useful interactively too.
    bool hasYaw = false;
    bool hasPitch = false;
    float yaw = 0.0f;
    float pitch = 0.0f;
    bool hasPosition = false;
    float cameraX = 0.0f;
    float cameraZ = 0.0f;
    bool hasEyeHeight = false;
    float eyeHeight = 1.7f;
    // Turkish is the product default; --lang overrides it.
    tt::localization::Language language = tt::localization::Language::Turkish;
};

// Prints the quick-start text in the given language. Used by --help and by the
// packaged QUICKSTART file, so both stay in step.
void printUsage(const char* exe, tt::localization::Language language) {
    using tt::localization::Key;
    using tt::localization::text;

    std::printf("%s\n\n", text(language, Key::QuickStartTitle));
    // The usage line contains one %s for the executable path.
    std::printf(text(language, Key::UsageLine), exe);
    std::printf("\n\n");

    std::printf("%s\n", text(language, Key::OptionsHeading));
    const Key optionKeys[] = {
        Key::OptionSeed,       Key::OptionAgents,     Key::OptionWidth,
        Key::OptionHeight,     Key::OptionScreenshot, Key::OptionScreenshotSeconds,
        Key::OptionYaw,        Key::OptionPitch,      Key::OptionCamera,
        Key::OptionEye,        Key::OptionLang,       Key::OptionHelp,
    };
    for (Key key : optionKeys) {
        std::printf("%s\n", text(language, key));
    }

    std::printf("\n%s\n", text(language, Key::ControlsHeading));
    const Key controlKeys[] = {
        Key::ControlMove,        Key::ControlSprint,       Key::ControlMouse,
        Key::ControlMouseCapture, Key::ControlView,        Key::ControlSelect,
        Key::ControlClick,       Key::ControlPause,        Key::ControlReset,
        Key::ControlResetAgent,  Key::ControlLanguage,     Key::ControlHelp,
        Key::ControlScreenshot,  Key::ControlEsc,
    };
    for (Key key : controlKeys) {
        std::printf("%s\n", text(language, key));
    }

    std::printf("\n%s\n", text(language, Key::ScopeNote));
}

bool parseArgs(int argc, char** argv, Options& options) {
    // Errors are reported in the language selected so far, so a Turkish user
    // sees Turkish messages from the very first failure.
    auto report = [&](tt::localization::Key key, const char* detail) {
        std::fprintf(stderr, tt::localization::text(options.language, key), detail);
        std::fprintf(stderr, "\n");
    };

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto nextValue = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                report(tt::localization::Key::ErrorOptionNeedsValue, name);
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            // Do not return here: a later --lang must still be honoured, so
            // "--help --lang en" prints English help.
            options.showHelp = true;
        } else if (arg == "--seed") {
            const char* v = nextValue("--seed");
            if (!v) return false;
            options.seed = std::strtoull(v, nullptr, 10);
        } else if (arg == "--agents") {
            const char* v = nextValue("--agents");
            if (!v) return false;
            options.agents = std::max(1, std::atoi(v));
        } else if (arg == "--width") {
            const char* v = nextValue("--width");
            if (!v) return false;
            options.width = std::max(320, std::atoi(v));
        } else if (arg == "--height") {
            const char* v = nextValue("--height");
            if (!v) return false;
            options.height = std::max(240, std::atoi(v));
        } else if (arg == "--screenshot") {
            const char* v = nextValue("--screenshot");
            if (!v) return false;
            options.screenshotMode = true;
            options.screenshotPath = v;
        } else if (arg == "--screenshot-seconds") {
            const char* v = nextValue("--screenshot-seconds");
            if (!v) return false;
            options.screenshotSeconds = std::max(0.0f, static_cast<float>(std::atof(v)));
        } else if (arg == "--yaw") {
            const char* v = nextValue("--yaw");
            if (!v) return false;
            options.hasYaw = true;
            options.yaw = static_cast<float>(std::atof(v));
        } else if (arg == "--pitch") {
            const char* v = nextValue("--pitch");
            if (!v) return false;
            options.hasPitch = true;
            options.pitch = static_cast<float>(std::atof(v));
        } else if (arg == "--camera") {
            const char* xv = nextValue("--camera");
            if (!xv) return false;
            const char* zv = nextValue("--camera");
            if (!zv) return false;
            options.hasPosition = true;
            options.cameraX = static_cast<float>(std::atof(xv));
            options.cameraZ = static_cast<float>(std::atof(zv));
        } else if (arg == "--eye") {
            const char* v = nextValue("--eye");
            if (!v) return false;
            options.hasEyeHeight = true;
            options.eyeHeight = std::max(0.2f, static_cast<float>(std::atof(v)));
        } else if (arg == "--print-agents") {
            options.printAgents = true;
        } else if (arg == "--lang") {
            const char* v = nextValue("--lang");
            if (!v) return false;
            tt::localization::Language requested = options.language;
            if (!tt::localization::parseLanguage(v, requested)) {
                std::fprintf(stderr, "error: unknown language '%s' (expected tr or en)\n", v);
                return false;
            }
            options.language = requested;
        } else {
            report(tt::localization::Key::ErrorUnknownOption, arg.c_str());
            return false;
        }
    }
    return true;
}

// Project a world point to screen coordinates using the matrices OpenGL is
// currently using. Used for click-to-select; implemented directly so the app
// needs no GLU dependency on any platform.
bool projectToScreen(const tt::Vec3& world, float& outX, float& outY, float& outDepth) {
    GLdouble model[16];
    GLdouble proj[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // eye = model * world
    const double wx = world.x;
    const double wy = world.y;
    const double wz = world.z;
    const double ex = model[0] * wx + model[4] * wy + model[8] * wz + model[12];
    const double ey = model[1] * wx + model[5] * wy + model[9] * wz + model[13];
    const double ez = model[2] * wx + model[6] * wy + model[10] * wz + model[14];
    const double ew = model[3] * wx + model[7] * wy + model[11] * wz + model[15];
    if (std::fabs(ew) < 1e-9) return false;

    // clip = proj * eye
    const double cx = proj[0] * ex + proj[4] * ey + proj[8] * ez + proj[12] * ew;
    const double cy = proj[1] * ex + proj[5] * ey + proj[9] * ez + proj[13] * ew;
    const double cz = proj[2] * ex + proj[6] * ey + proj[10] * ez + proj[14] * ew;
    const double cw = proj[3] * ex + proj[7] * ey + proj[11] * ez + proj[15] * ew;
    if (std::fabs(cw) < 1e-9) return false;

    const double ndcX = cx / cw;
    const double ndcY = cy / cw;
    const double ndcZ = cz / cw;

    const double vx = static_cast<double>(viewport[0]);
    const double vy = static_cast<double>(viewport[1]);
    const double vw = static_cast<double>(viewport[2]);
    const double vh = static_cast<double>(viewport[3]);

    outX = static_cast<float>(vx + (ndcX * 0.5 + 0.5) * vw);
    // NDC y grows upward; the window and HUD use a top-down y.
    outY = static_cast<float>(vy + (1.0 - (ndcY * 0.5 + 0.5)) * vh);
    outDepth = static_cast<float>(ndcZ * 0.5 + 0.5);
    return true;
}

int pickAgent(const tt::Simulation& sim, int mouseX, int mouseY, int width, int height) {
    int best = -1;
    float bestDistance = 48.0f;   // pixels; generous but predictable

    for (const tt::Agent& a : sim.agents()) {
        float sx = 0.0f;
        float sy = 0.0f;
        float depth = 0.0f;
        // Aim at the agent's head rather than its feet.
        if (!projectToScreen({a.position.x, 1.1f, a.position.z}, sx, sy, depth)) continue;
        if (depth < 0.0f || depth > 1.0f) continue;
        if (sx < 0.0f || sx > static_cast<float>(width)) continue;
        if (sy < 0.0f || sy > static_cast<float>(height)) continue;

        const float dx = sx - static_cast<float>(mouseX);
        const float dy = sy - static_cast<float>(mouseY);
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < bestDistance) {
            bestDistance = dist;
            best = a.id;
        }
    }
    return best;
}

struct MouseButtonState {
    bool left = false;
};

// A tiny debounce helper so a key press fires once even if it is held.
class KeyEdge {
public:
    bool pressed(bool down) {
        const bool edge = down && !down_;
        down_ = down;
        return edge;
    }

private:
    bool down_ = false;
};

}  // namespace

int main(int argc, char** argv) {
    using tt::localization::Key;
    using tt::localization::Language;
    using tt::localization::text;

    Options options;
    if (!parseArgs(argc, argv, options)) {
        printUsage(argv[0], options.language);
        return 2;
    }
    if (options.showHelp) {
        printUsage(argv[0], options.language);
        return 0;
    }
    Language language = options.language;

    // We provide our own main() and define SDL_MAIN_HANDLED above, so SDL must
    // be told that the application entry point is already handled. Without this
    // call SDL_Init() fails on Windows, where SDL normally redirects the entry
    // point through SDL2main.
    // https://wiki.libsdl.org/SDL2/SDL_SetMainReady
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, text(language, Key::ErrorSdlInit), SDL_GetError());
        std::fprintf(stderr, "\n");
        return 1;
    }

    // The renderer uses the same immediate-mode drawing style as the reference
    // prototype, so a compatibility profile is required.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (options.screenshotMode) {
        // A hidden window still gives a real, renderable OpenGL context, which
        // is how the documented screenshot command works without a desktop.
        windowFlags |= SDL_WINDOW_HIDDEN;
    }

    SDL_Window* window = SDL_CreateWindow("TinyTown v0.1", SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED, options.width, options.height,
                                         windowFlags);
    if (window == nullptr) {
        std::fprintf(stderr, text(language, Key::ErrorSdlWindow), SDL_GetError());
        std::fprintf(stderr, "\n");
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        std::fprintf(stderr, text(language, Key::ErrorGlContext), SDL_GetError());
        std::fprintf(stderr, "\n%s\n", text(language, Key::ErrorGlContextHint));
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);

    tt::Renderer renderer;
    std::string renderError;
    if (!renderer.initialize(renderError)) {
        std::fprintf(stderr, text(language, Key::ErrorRendererInit), renderError.c_str());
        std::fprintf(stderr, "\n");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int drawableWidth = options.width;
    int drawableHeight = options.height;
    SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
    renderer.resize(drawableWidth, drawableHeight);

    tt::SimConfig simConfig;
    simConfig.seed = options.seed;
    simConfig.agentCount = options.agents;

    tt::Simulation simulation;
    simulation.reset(simConfig);

    tt::Camera camera;
    camera.reset(options.hasPosition ? tt::Vec2{options.cameraX, options.cameraZ}
                                     : simulation.world().spawnPoint,
                 options.hasYaw ? options.yaw : simulation.world().spawnYaw,
                 options.hasPitch ? options.pitch : simulation.world().spawnPitch);

    bool overviewMode = options.hasEyeHeight && options.eyeHeight > 6.0f;
    const float streetEyeHeight = 1.7f;
    const float overviewEyeHeight = 26.0f;
    const float defaultPitch =
        options.hasPitch ? options.pitch
                         : (overviewMode ? -0.75f : simulation.world().spawnPitch);
    const float defaultEye = options.hasEyeHeight
                                 ? options.eyeHeight
                                 : (overviewMode ? overviewEyeHeight : streetEyeHeight);

    tt::ui::Hud hud;

    const bool interactive = !options.screenshotMode;
    bool running = true;
    tt::CameraInput input;
    MouseButtonState mouse;

    tt::Session session;
    tt::SessionConfig sessionConfig;
    sessionConfig.fixedStep = kFixedStep;
    sessionConfig.maxFrameDelta = kMaxFrameDelta;
    session.configure(sessionConfig);
    // The language chosen on the command line is the starting language.
    session.setLanguage(language);

    // The overview toggle lives on Session; the camera reacts to it here.
    auto applyViewMode = [&]() {
        camera.setEyeHeight(session.overview() ? overviewEyeHeight : defaultEye);
        camera.setPitch(defaultPitch);
    };
    session.setOverview(overviewMode);
    applyViewMode();

    KeyEdge keyTab;
    KeyEdge keyP;
    KeyEdge keyR;
    KeyEdge keyN;
    KeyEdge keyH;
    KeyEdge keyF2;
    KeyEdge keyM;
    KeyEdge keyV;
    KeyEdge keyL;
    KeyEdge keyEsc;
    KeyEdge numberKeys[9];

    if (interactive) {
        std::printf(text(language, Key::ConsoleBanner),
                    static_cast<unsigned long long>(options.seed), options.agents);
        std::printf("\n%s\n", text(language, Key::ConsoleHint));
    }

    Uint64 previousTicks = SDL_GetPerformanceCounter();
    const double ticksPerSecond = static_cast<double>(SDL_GetPerformanceFrequency());

    while (running) {
        Uint64 nowTicks = SDL_GetPerformanceCounter();
        const float frameDelta = static_cast<float>(
            static_cast<double>(nowTicks - previousTicks) / ticksPerSecond);
        previousTicks = nowTicks;

        // --- Events ---------------------------------------------------------
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                        event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
                        renderer.resize(drawableWidth, drawableHeight);
                    } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                        // Never leave a key stuck down after focus loss.
                        session.onFocusLost(input);
                        mouse.left = false;
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                    } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        running = false;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (session.mouseCaptured()) {
                        camera.look(static_cast<float>(event.motion.xrel) * kMouseSensitivity,
                                    -static_cast<float>(event.motion.yrel) * kMouseSensitivity);
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouse.left = true;
                        if (!session.mouseCaptured()) {
                            // Click to select; capture stays explicit via M.
                            const int picked = pickAgent(simulation, event.button.x,
                                                         event.button.y, drawableWidth,
                                                         drawableHeight);
                            session.selectAgent(picked,
                                                static_cast<int>(simulation.agents().size()));
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) mouse.left = false;
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    const bool down = (event.type == SDL_KEYDOWN);
                    const SDL_Keycode key = event.key.keysym.sym;
                    const int agentCount = static_cast<int>(simulation.agents().size());

                    switch (key) {
                        case SDLK_w: case SDLK_UP: input.forward = down; break;
                        case SDLK_s: case SDLK_DOWN: input.backward = down; break;
                        case SDLK_a: case SDLK_LEFT: input.left = down; break;
                        case SDLK_d: case SDLK_RIGHT: input.right = down; break;
                        case SDLK_LSHIFT: case SDLK_RSHIFT: input.sprint = down; break;
                        default: break;
                    }

                    if (down) {
                        if (key == SDLK_TAB) {
                            if (keyTab.pressed(true)) session.selectNextAgent(agentCount);
                        } else if (key >= SDLK_1 && key <= SDLK_9) {
                            const int index = key - SDLK_1;
                            if (numberKeys[index].pressed(true)) {
                                session.selectAgent(index, agentCount);
                            }
                        } else if (key == SDLK_p || key == SDLK_SPACE) {
                            if (keyP.pressed(true)) session.togglePause();
                        } else if (key == SDLK_r) {
                            if (keyR.pressed(true)) session.requestReset();
                        } else if (key == SDLK_n) {
                            if (keyN.pressed(true)) session.requestAgentReset();
                        } else if (key == SDLK_h || key == SDLK_F1) {
                            if (keyH.pressed(true)) session.toggleHelp();
                        } else if (key == SDLK_F2) {
                            if (keyF2.pressed(true)) session.requestScreenshot();
                        } else if (key == SDLK_v) {
                            if (keyV.pressed(true)) {
                                session.toggleOverview();
                                applyViewMode();
                            }
                        } else if (key == SDLK_l) {
                            // L is free: the language key must not collide with
                            // an existing control.
                            if (keyL.pressed(true)) {
                                session.cycleLanguage();
                                language = session.language();
                                std::printf("%s: %s\n", text(language, Key::HelpLanguage),
                                            tt::localization::languageName(language));
                            }
                        } else if (key == SDLK_m) {
                            if (keyM.pressed(true)) {
                                session.toggleMouseCapture();
                                SDL_SetRelativeMouseMode(session.mouseCaptured() ? SDL_TRUE
                                                                                 : SDL_FALSE);
                            }
                        } else if (key == SDLK_ESCAPE) {
                            if (keyEsc.pressed(true)) {
                                if (session.mouseCaptured()) {
                                    session.releaseMouseCapture();
                                    SDL_SetRelativeMouseMode(SDL_FALSE);
                                } else {
                                    running = false;
                                }
                            }
                        }
                    } else {
                        keyTab.pressed(false);
                        keyP.pressed(false);
                        keyR.pressed(false);
                        keyN.pressed(false);
                        keyH.pressed(false);
                        keyF2.pressed(false);
                        keyM.pressed(false);
                        keyV.pressed(false);
                        keyL.pressed(false);
                        keyEsc.pressed(false);
                        if (key >= SDLK_1 && key <= SDLK_9) numberKeys[key - SDLK_1].pressed(false);
                    }
                    break;
                }

                default:
                    break;
            }
        }

        // --- Requested state changes ----------------------------------------
        if (session.takeResetRequest()) {
            simulation.reset(simConfig);
            camera.reset(options.hasPosition ? tt::Vec2{options.cameraX, options.cameraZ}
                                             : simulation.world().spawnPoint,
                         options.hasYaw ? options.yaw : simulation.world().spawnYaw,
                         options.hasPitch ? options.pitch : simulation.world().spawnPitch);
            applyViewMode();
            session.selectAgent(0, static_cast<int>(simulation.agents().size()));
            std::printf(text(language, Key::WorldReset),
                        static_cast<unsigned long long>(simConfig.seed));
            std::printf("\n");
        }
        if (session.takeAgentResetRequest()) {
            simulation.resetAgent(session.selectedAgent());
        }
        const bool requestScreenshot = session.takeScreenshotRequest();

        // --- Simulation -----------------------------------------------------
        // Fixed-step integration keeps behaviour reproducible regardless of the
        // frame rate, and the accumulator is bounded so a stall cannot cause a
        // burst of catch-up steps.
        if (interactive) {
            session.advance(frameDelta, simulation);
        } else {
            // Screenshot mode drives simulated time directly so the documented
            // command is fast and deterministic, with no dependence on the
            // wall-clock speed of the machine.
            const float target = options.screenshotSeconds;
            int guard = 0;
            while (simulation.elapsed() < target && guard < 200000) {
                simulation.update(kFixedStep);
                ++guard;
            }
        }

        // --- Camera ---------------------------------------------------------
        if (interactive) {
            camera.update(simulation.world(), input, frameDelta);
        }

        // --- Render ---------------------------------------------------------
        const float wallClock = session.wallClock();
        const int selectedAgent = session.selectedAgent();

        renderer.beginFrame(camera, wallClock);
        renderer.drawWorld(simulation.world());
        renderer.drawAgents(simulation.agents(), selectedAgent, wallClock);

        const float hudScale = tt::clampf(static_cast<float>(drawableHeight) / 480.0f, 1.0f, 3.0f);

        if (session.showHelp()) {
            // The overlay always follows the session language, so pressing L
            // changes the visible interface immediately.
            hud.setLanguage(session.language());
            hud.begin(drawableWidth, drawableHeight);
            hud.drawHelp(12.0f, 12.0f, hudScale);

            const float statusHeight = tt::ui::Hud::lineHeight(hudScale) + 12.0f;
            hud.drawStatusLine(simulation, session.paused(), 12.0f,
                               static_cast<float>(drawableHeight) - statusHeight - 12.0f,
                               hudScale);

            const float panelX = static_cast<float>(drawableWidth) - 330.0f * hudScale;
            hud.drawAgentList(simulation, selectedAgent, panelX, 12.0f, hudScale);
            hud.drawInspectPanel(simulation, selectedAgent, panelX,
                                 12.0f + 10.0f * tt::ui::Hud::lineHeight(hudScale), 330.0f * hudScale,
                                 hudScale);
            hud.end();
        }

        // --- Screenshot ------------------------------------------------------
        // In screenshot mode capture once after the requested simulated time;
        // interactively F2 captures the current frame.
        const bool autoCapture =
            options.screenshotMode && simulation.elapsed() >= options.screenshotSeconds;
        if (autoCapture || requestScreenshot) {
            std::string path = options.screenshotPath;
            if (path.empty()) {
                // Interactive F2 writes a timestamped file into captures/.
                std::error_code ec;
                std::filesystem::create_directories("captures", ec);
                const std::string stamp = std::to_string(static_cast<long long>(wallClock * 1000.0f));
                path = "captures/tinytown_" + std::to_string(simConfig.seed) + "_" + stamp + "." +
                       tt::image::preferredExtension();
            }

            int width = 0;
            int height = 0;
            const std::vector<unsigned char> pixels = renderer.readPixels(width, height);
            std::string error;
            if (!tt::image::writeScreenshot(path, pixels, width, height, error)) {
                std::fprintf(stderr, text(language, Key::ErrorScreenshot), error.c_str());
                std::fprintf(stderr, "\n");
                if (options.screenshotMode) {
                    renderer.shutdown();
                    SDL_GL_DeleteContext(glContext);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 1;
                }
            } else {
                std::printf(text(language, Key::WroteScreenshot), path.c_str(), width, height);
                std::printf("\n");

                if (options.printAgents) {
                    std::printf("--- agent state at capture (t=%.2fs) ---\n", simulation.elapsed());
                    for (const tt::Agent& a : simulation.agents()) {
                        std::printf(
                            "  #%d %-6s task=%-8s state=%-10s target=%-14s at=(%.1f, %.1f) "
                            "done=%d failed=%d carry=%s\n",
                            a.id, a.name.c_str(), tt::toString(a.task), tt::toString(a.state),
                            a.targetName.c_str(), static_cast<double>(a.position.x),
                            static_cast<double>(a.position.z), a.completedTasks, a.failedTasks,
                            a.carryingPackage ? "package" : "nothing");
                    }
                }
            }
            if (options.screenshotMode) running = false;
        }

        SDL_GL_SwapWindow(window);
    }

    renderer.shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
