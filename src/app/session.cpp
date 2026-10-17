#include "app/session.hpp"

#include <algorithm>

namespace tt {

void Session::cycleLanguage() {
    language_ = language_ == localization::Language::Turkish ? localization::Language::English
                                                            : localization::Language::Turkish;
}

void Session::selectNextAgent(int agentCount) {
    if (agentCount <= 0) {
        selectedAgent_ = 0;
        return;
    }
    selectedAgent_ = (selectedAgent_ + 1) % agentCount;
}

void Session::selectAgent(int index, int agentCount) {
    if (index < 0 || index >= agentCount) return;
    selectedAgent_ = index;
}

bool Session::takeResetRequest() {
    const bool value = resetRequested_;
    resetRequested_ = false;
    return value;
}

bool Session::takeAgentResetRequest() {
    const bool value = agentResetRequested_;
    agentResetRequested_ = false;
    return value;
}

bool Session::takeScreenshotRequest() {
    const bool value = screenshotRequested_;
    screenshotRequested_ = false;
    return value;
}

void Session::onFocusLost(CameraInput& input) {
    input = CameraInput{};
    mouseCaptured_ = false;
}

int Session::advance(float frameDelta, Simulation& sim) {
    // Clamp first: a stall must never translate into a large simulation step.
    const float clamped = std::min(frameDelta, config_.maxFrameDelta);
    wallClock_ += std::max(0.0f, clamped);

    if (paused_) return 0;

    accumulator_ += clamped;
    int steps = 0;
    while (accumulator_ >= config_.fixedStep && steps < config_.maxStepsPerFrame) {
        sim.update(config_.fixedStep);
        accumulator_ -= config_.fixedStep;
        ++steps;
    }
    // If the step budget was exhausted, drop the backlog rather than carrying
    // debt into the next frame.
    if (steps == config_.maxStepsPerFrame) accumulator_ = 0.0f;
    return steps;
}

}  // namespace tt
