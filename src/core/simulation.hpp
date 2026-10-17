#pragma once

// Deterministic agent simulation. No graphics, no platform headers, no I/O:
// the whole module can be stepped and asserted in a headless test binary.

#include <cstdint>
#include <string>
#include <vector>

#include "core/pathfinding.hpp"
#include "core/types.hpp"
#include "core/world.hpp"

namespace tt {

enum class AgentState {
    Idle,
    Navigating,
    Working,
    Completed,
    Failed,
};

const char* toString(AgentState state);

enum class TaskKind {
    Patrol,     // walk a loop of street corners
    Deliver,    // carry a package to a storefront, then return to the depot
    Survey,     // travel to a landmark and inspect it
};

const char* toString(TaskKind kind);

struct TaskResult {
    bool finished = false;
    bool success = false;
    std::string note;
};

struct Agent {
    int id = 0;
    std::string name;
    TaskKind task = TaskKind::Patrol;
    AgentState state = AgentState::Idle;

    Vec2 position{};
    float heading = 0.0f;          // radians, facing direction
    float speed = 3.2f;            // world units per second
    Color3 color{};

    // Task bookkeeping -----------------------------------------------------
    std::vector<Vec2> route;       // remaining polyline (world space)
    size_t routeIndex = 0;
    std::string targetName;        // human-readable current destination
    Vec2 target{};
    int loopIndex = 0;             // patrol/delivery progression counter
    float workTimer = 0.0f;        // seconds remaining in Working state
    float resultTimer = 0.0f;      // seconds a Completed/Failed result stays visible
    bool carryingPackage = false;

    int completedTasks = 0;
    int failedTasks = 0;
    std::string lastResult;        // truthful outcome text for the panel

    // Debug/visualisation: the current planned path (kept even when empty).
    std::vector<Vec2> debugPath;
};

struct SimConfig {
    uint64_t seed = 1337;
    int agentCount = 3;
    float agentSpeed = 3.2f;
    float workDuration = 1.6f;      // seconds spent "working" at a target
    float resultHold = 1.0f;        // seconds a Completed/Failed result stays visible
    float maxStepDt = 0.05f;        // clamp so a long stall cannot teleport
    float arriveRadius = 0.45f;
};

class Simulation {
public:
    // Build a world + agents from a seed. Calling this again resets everything
    // to a known state.
    void reset(uint64_t seed, int agentCount);
    void reset(const SimConfig& config);

    void update(float dt);

    const World& world() const { return world_; }
    const std::vector<Agent>& agents() const { return agents_; }
    const SimConfig& config() const { return config_; }

    // Total simulated seconds since the last reset (advances only while
    // unpaused).
    float elapsed() const { return elapsed_; }
    int completedTaskCount() const;
    int failedTaskCount() const;

    // Send an agent to a named landmark. Returns false (and records a failure
    // on the agent) when the target is unknown or unreachable.
    bool assignTask(int agentId, const std::string& landmarkName);

    // Reset a single agent back to its starting behaviour without touching the
    // rest of the world.
    void resetAgent(int agentId);

private:
    void seedAgents(int count);
    void startPatrol(Agent& agent);
    void startDelivery(Agent& agent);
    void startSurvey(Agent& agent);
    void beginNextTask(Agent& agent);
    void stepAgent(Agent& agent, float dt);
    void advanceAlongRoute(Agent& agent, float dt);
    void failAgent(Agent& agent, const std::string& reason);
    void completeAgentTask(Agent& agent, const std::string& note);
    bool assignRoute(Agent& agent, Vec2 destination, const std::string& label);
    const Landmark* findLandmark(const std::string& name) const;

    World world_;
    SimConfig config_;
    std::vector<Agent> agents_;
    float elapsed_ = 0.0f;
};

// Names of every landmark an agent can be sent to, for the UI and tests.
std::vector<std::string> landmarkNames(const World& world);

// Look up a landmark position by name. Returns false when unknown.
bool findLandmarkPosition(const World& world, const std::string& name, Vec2& out);

}  // namespace tt
