#include "core/simulation.hpp"

#include <string>
#include <vector>

#include "test_framework.hpp"

using namespace tt;

namespace {

constexpr float kStep = 1.0f / 60.0f;

void runFor(Simulation& sim, float seconds, float step = kStep) {
    const int steps = static_cast<int>(seconds / step);
    for (int i = 0; i < steps; ++i) {
        sim.update(step);
    }
}

// Snapshot of everything that must be reproducible from a seed.
std::string simFingerprint(const Simulation& sim) {
    std::string fp;
    for (const Agent& a : sim.agents()) {
        fp += std::to_string(a.id);
        fp += ":";
        fp += toString(a.state);
        fp += ":";
        fp += toString(a.task);
        fp += ":";
        fp += std::to_string(static_cast<int>(a.position.x * 1000.0f));
        fp += ",";
        fp += std::to_string(static_cast<int>(a.position.z * 1000.0f));
        fp += ";";
    }
    return fp;
}

}  // namespace

TEST(Simulation, ResetIsDeterministic) {
    Simulation a;
    Simulation b;
    a.reset(4242ULL, 3);
    b.reset(4242ULL, 3);

    CHECK_EQ(simFingerprint(a), simFingerprint(b));

    runFor(a, 20.0f);
    runFor(b, 20.0f);

    CHECK_EQ(simFingerprint(a), simFingerprint(b));
    CHECK_EQ(a.completedTaskCount(), b.completedTaskCount());
    CHECK_EQ(a.elapsed(), b.elapsed());
}

TEST(Simulation, SeedsAtLeastThreeAgentsWithDistinctTasks) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    CHECK_EQ(sim.agents().size(), static_cast<size_t>(3));

    // The town must demonstrate patrol, delivery and survey behaviour.
    bool hasPatrol = false;
    bool hasDeliver = false;
    bool hasSurvey = false;
    for (const Agent& a : sim.agents()) {
        if (a.task == TaskKind::Patrol) hasPatrol = true;
        if (a.task == TaskKind::Deliver) hasDeliver = true;
        if (a.task == TaskKind::Survey) hasSurvey = true;
        CHECK_MSG(!a.name.empty(), "every agent needs a name for the inspect panel");
    }
    CHECK_MSG(hasPatrol, "expected a patrolling agent");
    CHECK_MSG(hasDeliver, "expected a delivery agent");
    CHECK_MSG(hasSurvey, "expected a surveying agent");
}

TEST(Simulation, AgentsStartWithARealTask) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    for (const Agent& a : sim.agents()) {
        CHECK_MSG(a.state == AgentState::Navigating || a.state == AgentState::Working,
                  std::string("agent ") + a.name + " did not start working, state=" +
                      toString(a.state));
        CHECK_MSG(!a.targetName.empty(), "agent has no readable target");
        CHECK_MSG(a.route.size() >= 2 || a.state == AgentState::Working,
                  "navigating agent has no route");
    }
}

TEST(Simulation, ObservesNavigatingThenWorkingThenCompleted) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    bool sawNavigating = false;
    bool sawWorking = false;
    bool sawCompleted = false;

    for (int i = 0; i < 6000 && !(sawNavigating && sawWorking && sawCompleted); ++i) {
        sim.update(kStep);
        for (const Agent& a : sim.agents()) {
            if (a.state == AgentState::Navigating) sawNavigating = true;
            if (a.state == AgentState::Working) sawWorking = true;
            if (a.state == AgentState::Completed) sawCompleted = true;
        }
    }

    CHECK_MSG(sawNavigating, "never observed Navigating state");
    CHECK_MSG(sawWorking, "never observed Working state");
    CHECK_MSG(sawCompleted, "never observed Completed state");
}

TEST(Simulation, CompletedTaskRecordsTruthfulResult) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    runFor(sim, 40.0f);

    CHECK_MSG(sim.completedTaskCount() > 0, "agents should complete tasks over 40s");
    CHECK_EQ(sim.failedTaskCount(), 0);

    for (const Agent& a : sim.agents()) {
        CHECK_MSG(a.completedTasks > 0, std::string("agent ") + a.name + " completed nothing");
        CHECK_MSG(!a.lastResult.empty(), "completed agent has no result text");
    }
}

TEST(Simulation, LargeDtDoesNotTeleportAgents) {
    Simulation big;
    Simulation small;
    big.reset(1337ULL, 3);
    small.reset(1337ULL, 3);

    // One huge frame must advance exactly as much as one clamped frame.
    big.update(10.0f);
    small.update(0.05f);

    CHECK_EQ(simFingerprint(big), simFingerprint(small));
    CHECK_NEAR(big.elapsed(), 0.05f, 0.0001f);
}

TEST(Simulation, NonPositiveDtIsIgnored) {
    Simulation sim;
    sim.reset(1337ULL, 3);
    const std::string before = simFingerprint(sim);

    sim.update(0.0f);
    sim.update(-5.0f);

    CHECK_EQ(before, simFingerprint(sim));
    CHECK_NEAR(sim.elapsed(), 0.0f, 0.0f);
}

TEST(Simulation, AgentsNeverEnterBlockedGeometry) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    // A long run exercises patrol, delivery and survey routes repeatedly.
    for (int i = 0; i < 4000; ++i) {
        sim.update(kStep);
        for (const Agent& a : sim.agents()) {
            CHECK_MSG(isWalkable(sim.world(), a.position),
                      std::string("agent ") + a.name + " left walkable ground at step " +
                          std::to_string(i));
        }
    }
    CHECK_MSG(sim.completedTaskCount() > 5, "long run should complete many tasks");
}

TEST(Simulation, UnknownTargetFailsSafely) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    const bool ok = sim.assignTask(0, "Atlantis");
    CHECK_MSG(!ok, "assigning an unknown landmark must fail");
    CHECK_EQ(sim.agents()[0].state, AgentState::Failed);
    CHECK_EQ(sim.agents()[0].failedTasks, 1);
    CHECK_MSG(sim.agents()[0].lastResult.find("Atlantis") != std::string::npos,
              "failure text should name the missing target");

    // The agent must recover and continue after the failure.
    runFor(sim, 6.0f);
    CHECK_MSG(sim.agents()[0].state == AgentState::Navigating ||
                  sim.agents()[0].state == AgentState::Working ||
                  sim.agents()[0].state == AgentState::Completed,
              "failed agent did not pick up a new task");
}

TEST(Simulation, ValidTargetCompletesForEveryLandmark) {
    for (const std::string& name : landmarkNames(generateWorld(1337ULL))) {
        Simulation sim;
        sim.reset(1337ULL, 3);

        const bool ok = sim.assignTask(1, name);
        CHECK_MSG(ok, "valid landmark rejected: " + name);

        const Agent& agent = sim.agents()[1];
        CHECK_EQ(agent.state, AgentState::Navigating);
        CHECK_EQ(agent.targetName, name);

        // Walk to the target and finish the work step.
        runFor(sim, 120.0f);
        CHECK_MSG(sim.agents()[1].completedTasks > 0,
                  "agent never completed task for " + name);
        CHECK_MSG(distance(sim.agents()[1].position,
                           sim.world().landmarks[0].position) >= 0.0f,
                  "sanity");
    }
}

TEST(Simulation, InvalidAgentIdIsRejected) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    CHECK(!sim.assignTask(-1, "Depot"));
    CHECK(!sim.assignTask(99, "Depot"));
    CHECK_EQ(sim.completedTaskCount(), 0);
}

TEST(Simulation, ResetRestoresKnownState) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    const std::string initial = simFingerprint(sim);

    runFor(sim, 30.0f);
    CHECK_MSG(simFingerprint(sim) != initial, "simulation should have moved on");

    sim.reset(1337ULL, 3);
    CHECK_EQ(simFingerprint(sim), initial);
    CHECK_EQ(sim.completedTaskCount(), 0);
    CHECK_EQ(sim.failedTaskCount(), 0);
    CHECK_NEAR(sim.elapsed(), 0.0f, 0.0f);
}

TEST(Simulation, ResetAgentOnlyAffectsThatAgent) {
    Simulation sim;
    sim.reset(1337ULL, 3);

    runFor(sim, 15.0f);

    const std::string otherBefore = std::to_string(static_cast<int>(sim.agents()[2].position.x * 100.0f)) +
                                    "," + std::to_string(static_cast<int>(sim.agents()[2].position.z * 100.0f));

    sim.resetAgent(0);
    CHECK_EQ(sim.agents()[0].state, AgentState::Navigating);
    CHECK_EQ(sim.agents()[0].loopIndex, 0);

    const std::string otherAfter = std::to_string(static_cast<int>(sim.agents()[2].position.x * 100.0f)) +
                                   "," + std::to_string(static_cast<int>(sim.agents()[2].position.z * 100.0f));
    CHECK_EQ(otherBefore, otherAfter);
}

TEST(Simulation, FiveAgentsStillBehave) {
    Simulation sim;
    sim.reset(777ULL, 5);
    CHECK_EQ(sim.agents().size(), static_cast<size_t>(5));

    runFor(sim, 45.0f);
    CHECK_MSG(sim.completedTaskCount() > 0, "five agents should complete tasks");
    CHECK_EQ(sim.failedTaskCount(), 0);
}
