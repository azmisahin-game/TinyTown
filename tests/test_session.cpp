// Session tests cover the behaviour that window/input bugs hide: pause,
// selection, frame-time clamping and focus loss. None of this needs SDL, so it
// runs in the same headless binary as the simulation tests.

#include "app/session.hpp"

#include "test_framework.hpp"

using namespace tt;

namespace {

SessionConfig testConfig() {
    SessionConfig config;
    config.fixedStep = 1.0f / 60.0f;
    config.maxFrameDelta = 0.25f;
    config.maxStepsPerFrame = 8;
    return config;
}

}  // namespace

TEST(Session, StartsRunningWithHelpVisible) {
    Session session;
    session.configure(testConfig());

    CHECK(!session.paused());
    CHECK(session.showHelp());
    CHECK(!session.mouseCaptured());
    CHECK_EQ(session.selectedAgent(), 0);
}

TEST(Session, PauseStopsSimulationProgress) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    Session session;
    session.configure(testConfig());

    session.togglePause();
    CHECK(session.paused());

    for (int i = 0; i < 120; ++i) {
        const int steps = session.advance(1.0f / 60.0f, sim);
        CHECK_EQ(steps, 0);
    }
    CHECK_NEAR(sim.elapsed(), 0.0f, 0.0f);

    // Resuming must let time flow again.
    session.togglePause();
    CHECK(!session.paused());
    session.advance(1.0f / 60.0f, sim);
    CHECK(sim.elapsed() > 0.0f);
}

TEST(Session, LargeFrameDeltaIsClampedToOneMaxStep) {
    Simulation big;
    Simulation small;
    big.reset(1337ULL, 3);
    small.reset(1337ULL, 3);

    Session a;
    Session b;
    a.configure(testConfig());
    b.configure(testConfig());

    const int bigSteps = a.advance(10.0f, big);
    const int smallSteps = b.advance(testConfig().maxFrameDelta, small);

    CHECK_EQ(bigSteps, smallSteps);
    CHECK_EQ(big.elapsed(), small.elapsed());

    // Simulated time advances only in whole fixed steps, and a single frame can
    // never exceed the step budget. That is the property that stops a stall
    // from teleporting agents across the town.
    const float expected = static_cast<float>(testConfig().maxStepsPerFrame) *
                           testConfig().fixedStep;
    CHECK_NEAR(big.elapsed(), expected, 0.0001f);
    CHECK_MSG(big.elapsed() < testConfig().maxFrameDelta,
              "elapsed time must stay below the clamped frame delta");
}

TEST(Session, StepBudgetIsBounded) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    Session session;
    session.configure(testConfig());

    // A frame exactly at the clamp should not exceed maxStepsPerFrame.
    const int steps = session.advance(testConfig().maxFrameDelta, sim);
    CHECK(steps <= testConfig().maxStepsPerFrame);

    // Backlog must be dropped, not carried forward into a later burst.
    const int nextSteps = session.advance(1.0f / 60.0f, sim);
    CHECK(nextSteps <= 2);
}

TEST(Session, NegativeFrameDeltaDoesNotRewindTime) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    Session session;
    session.configure(testConfig());

    session.advance(-1.0f, sim);
    CHECK_NEAR(session.wallClock(), 0.0f, 0.0f);
    CHECK_NEAR(sim.elapsed(), 0.0f, 0.0f);
}

TEST(Session, SelectionCyclesAndWraps) {
    Session session;
    session.configure(testConfig());

    session.selectNextAgent(3);
    CHECK_EQ(session.selectedAgent(), 1);
    session.selectNextAgent(3);
    CHECK_EQ(session.selectedAgent(), 2);
    session.selectNextAgent(3);
    CHECK_EQ(session.selectedAgent(), 0);
}

TEST(Session, SelectionIgnoresInvalidIndices) {
    Session session;
    session.configure(testConfig());

    session.selectAgent(2, 3);
    CHECK_EQ(session.selectedAgent(), 2);

    // A stale or bogus index must not move the selection.
    session.selectAgent(99, 3);
    CHECK_EQ(session.selectedAgent(), 2);
    session.selectAgent(-1, 3);
    CHECK_EQ(session.selectedAgent(), 2);

    // An empty agent list resets to a safe index.
    session.selectNextAgent(0);
    CHECK_EQ(session.selectedAgent(), 0);
}

TEST(Session, ResetRequestsAreConsumedOnce) {
    Session session;
    session.configure(testConfig());

    CHECK(!session.takeResetRequest());
    session.requestReset();
    CHECK(session.takeResetRequest());
    CHECK_MSG(!session.takeResetRequest(), "reset request must not fire twice");

    session.requestAgentReset();
    CHECK(session.takeAgentResetRequest());
    CHECK(!session.takeAgentResetRequest());

    session.requestScreenshot();
    CHECK(session.takeScreenshotRequest());
    CHECK(!session.takeScreenshotRequest());
}

TEST(Session, FocusLossClearsInputAndMouseCapture) {
    Session session;
    session.configure(testConfig());

    session.toggleMouseCapture();
    CHECK(session.mouseCaptured());

    CameraInput input;
    input.forward = true;
    input.sprint = true;
    input.left = true;

    session.onFocusLost(input);

    CHECK(!input.forward);
    CHECK(!input.sprint);
    CHECK(!input.left);
    CHECK(!session.mouseCaptured());
}

TEST(Session, TogglesFlipIndependently) {
    Session session;
    session.configure(testConfig());

    session.toggleHelp();
    CHECK(!session.showHelp());
    session.toggleHelp();
    CHECK(session.showHelp());

    session.toggleOverview();
    CHECK(session.overview());
    session.setOverview(false);
    CHECK(!session.overview());
}

TEST(Session, AdvanceIsFrameRateIndependent) {
    Simulation coarse;
    Simulation fine;
    coarse.reset(1337ULL, 3);
    fine.reset(1337ULL, 3);

    Session a;
    Session b;
    a.configure(testConfig());
    b.configure(testConfig());

    // One second of simulated time at 20 FPS vs 100 FPS.
    for (int i = 0; i < 20; ++i) a.advance(0.05f, coarse);
    for (int i = 0; i < 100; ++i) b.advance(0.01f, fine);

    CHECK_NEAR(coarse.elapsed(), fine.elapsed(), 0.02f);
}
