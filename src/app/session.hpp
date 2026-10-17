#pragma once

// Application session state: pause, selection, view mode and the fixed-step
// time accumulator.
//
// This is deliberately free of SDL and OpenGL types. main.cpp translates window
// events into calls on this object, which means the behaviour that is easy to
// get wrong (pause/resume, selection clamping, frame-time clamping) can be
// tested in the headless test binary.

#include <cstdint>

#include "core/camera.hpp"
#include "core/simulation.hpp"
#include "ui/localization.hpp"

namespace tt {

struct SessionConfig {
    float fixedStep = 1.0f / 60.0f;
    // A frame longer than this is treated as a stall: the simulation advances by
    // at most this much, so returning from a paused or dragged window does not
    // teleport anything.
    float maxFrameDelta = 0.25f;
    // Upper bound on catch-up steps per frame.
    int maxStepsPerFrame = 8;
};

class Session {
public:
    void configure(const SessionConfig& config) { config_ = config; }

    // --- User actions -------------------------------------------------------
    void togglePause() { paused_ = !paused_; }
    void toggleHelp() { showHelp_ = !showHelp_; }
    void toggleMouseCapture() { mouseCaptured_ = !mouseCaptured_; }
    void releaseMouseCapture() { mouseCaptured_ = false; }
    void toggleOverview() { overview_ = !overview_; }
    void setOverview(bool on) { overview_ = on; }

    // Cycle the interface language. Turkish is the default, so one press moves
    // to English and the next returns to Turkish.
    void cycleLanguage();
    void setLanguage(localization::Language language) { language_ = language; }

    // Cycle to the next agent. Safe when there are no agents.
    void selectNextAgent(int agentCount);

    // Select a specific agent; out-of-range indices are ignored so a stale
    // index can never point past the end of the agent list.
    void selectAgent(int index, int agentCount);

    // Ask for a full reset / a single-agent reset. The request is consumed by
    // take*() so the caller performs it exactly once.
    void requestReset() { resetRequested_ = true; }
    void requestAgentReset() { agentResetRequested_ = true; }
    void requestScreenshot() { screenshotRequested_ = true; }

    bool takeResetRequest();
    bool takeAgentResetRequest();
    bool takeScreenshotRequest();

    // Clear per-frame input and drop the mouse capture, e.g. on focus loss.
    void onFocusLost(CameraInput& input);

    // --- Per-frame ----------------------------------------------------------
    // Advance the simulation using a fixed step. Returns the number of steps
    // actually taken, which makes the clamp observable in tests.
    int advance(float frameDelta, Simulation& sim);

    // --- Accessors ----------------------------------------------------------
    bool paused() const { return paused_; }
    bool showHelp() const { return showHelp_; }
    bool mouseCaptured() const { return mouseCaptured_; }
    bool overview() const { return overview_; }
    localization::Language language() const { return language_; }
    int selectedAgent() const { return selectedAgent_; }
    float accumulator() const { return accumulator_; }
    float wallClock() const { return wallClock_; }
    const SessionConfig& config() const { return config_; }

private:
    SessionConfig config_{};
    bool paused_ = false;
    bool showHelp_ = true;
    bool mouseCaptured_ = false;
    bool overview_ = false;
    // Turkish is the product default; English is one key press away.
    localization::Language language_ = localization::Language::Turkish;
    bool resetRequested_ = false;
    bool agentResetRequested_ = false;
    bool screenshotRequested_ = false;
    int selectedAgent_ = 0;
    float accumulator_ = 0.0f;
    float wallClock_ = 0.0f;
};

}  // namespace tt
