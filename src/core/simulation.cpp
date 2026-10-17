#include "core/simulation.hpp"

#include <algorithm>
#include <cmath>

#include "core/random.hpp"

namespace tt {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Distinct, readable agent colours against the muted town palette.
constexpr Color3 kAgentColors[] = {
    {0.90f, 0.36f, 0.30f},   // warm red
    {0.28f, 0.55f, 0.85f},   // blue
    {0.34f, 0.72f, 0.45f},   // green
    {0.88f, 0.66f, 0.24f},   // amber
    {0.66f, 0.45f, 0.82f},   // violet
};

const char* kAgentNames[] = {
    "Ada", "Bora", "Ceren", "Deniz", "Emre", "Figen",
};

float wrapAngle(float a) {
    while (a > kPi) a -= 2.0f * kPi;
    while (a < -kPi) a += 2.0f * kPi;
    return a;
}

// Turn the agent smoothly toward a direction instead of snapping instantly.
float turnToward(float current, float desired, float maxDelta) {
    const float diff = wrapAngle(desired - current);
    if (std::fabs(diff) <= maxDelta) return wrapAngle(desired);
    return wrapAngle(current + (diff > 0.0f ? maxDelta : -maxDelta));
}

}  // namespace

const char* toString(AgentState state) {
    switch (state) {
        case AgentState::Idle: return "Idle";
        case AgentState::Navigating: return "Navigating";
        case AgentState::Working: return "Working";
        case AgentState::Completed: return "Completed";
        case AgentState::Failed: return "Failed";
    }
    return "Unknown";
}

const char* toString(TaskKind kind) {
    switch (kind) {
        case TaskKind::Patrol: return "Patrol";
        case TaskKind::Deliver: return "Deliver";
        case TaskKind::Survey: return "Survey";
    }
    return "Unknown";
}

std::vector<std::string> landmarkNames(const World& world) {
    std::vector<std::string> names;
    names.reserve(world.landmarks.size());
    for (const Landmark& lm : world.landmarks) names.push_back(lm.name);
    return names;
}

bool findLandmarkPosition(const World& world, const std::string& name, Vec2& out) {
    for (const Landmark& lm : world.landmarks) {
        if (lm.name == name) {
            out = lm.position;
            return true;
        }
    }
    return false;
}

const Landmark* Simulation::findLandmark(const std::string& name) const {
    for (const Landmark& lm : world_.landmarks) {
        if (lm.name == name) return &lm;
    }
    return nullptr;
}

void Simulation::reset(uint64_t seed, int agentCount) {
    SimConfig cfg;
    cfg.seed = seed;
    cfg.agentCount = agentCount;
    reset(cfg);
}

void Simulation::reset(const SimConfig& config) {
    config_ = config;
    world_ = generateWorld(config.seed);
    elapsed_ = 0.0f;
    seedAgents(std::max(1, config.agentCount));
}

void Simulation::seedAgents(int count) {
    agents_.clear();
    agents_.reserve(static_cast<size_t>(count));

    Rng rng(config_.seed ^ 0x51ED270B1F9A3C4DULL);

    for (int i = 0; i < count; ++i) {
        Agent agent;
        agent.id = i;
        agent.name = kAgentNames[i % static_cast<int>(std::size(kAgentNames))];
        // Small deterministic speed variation keeps the group from moving as a
        // single block without making behaviour unpredictable.
        agent.speed = config_.agentSpeed * rng.range(0.92f, 1.08f);
        agent.color = kAgentColors[i % static_cast<int>(std::size(kAgentColors))];

        // Stagger starts around the depot so agents do not overlap at t=0.
        const float angle = (2.0f * kPi * static_cast<float>(i)) / static_cast<float>(count);
        const Vec2 offset{std::cos(angle) * 2.2f, std::sin(angle) * 2.2f};
        agent.position = snapToWalkable(world_, world_.landmarks[world_.depotIndex].position + offset);
        agent.heading = angle;

        // Give the town one of each behaviour so all three task types are
        // visible from the first seconds of the session.
        switch (i % 3) {
            case 0: agent.task = TaskKind::Patrol; break;
            case 1: agent.task = TaskKind::Deliver; break;
            default: agent.task = TaskKind::Survey; break;
        }

        agents_.push_back(agent);
    }

    for (Agent& agent : agents_) {
        beginNextTask(agent);
    }
}

void Simulation::resetAgent(int agentId) {
    if (agentId < 0 || agentId >= static_cast<int>(agents_.size())) return;
    Agent& agent = agents_[static_cast<size_t>(agentId)];
    agent.state = AgentState::Idle;
    agent.route.clear();
    agent.routeIndex = 0;
    agent.workTimer = 0.0f;
    agent.carryingPackage = false;
    agent.loopIndex = 0;
    agent.position = snapToWalkable(
        world_, world_.landmarks[world_.depotIndex].position +
                    Vec2{static_cast<float>(agent.id) * 1.5f - 2.0f, 2.0f});
    agent.lastResult = "Reset to start";
    beginNextTask(agent);
}

bool Simulation::assignRoute(Agent& agent, Vec2 destination, const std::string& label) {
    const PathResult path = findPath(world_, agent.position, destination);
    if (!path.found) {
        failAgent(agent, "No route to " + label);
        return false;
    }

    agent.route = path.points;
    agent.routeIndex = 0;
    agent.target = destination;
    agent.targetName = label;
    agent.debugPath = path.points;
    agent.state = AgentState::Navigating;
    return true;
}

bool Simulation::assignTask(int agentId, const std::string& landmarkName) {
    if (agentId < 0 || agentId >= static_cast<int>(agents_.size())) return false;

    Agent& agent = agents_[static_cast<size_t>(agentId)];
    const Landmark* landmark = findLandmark(landmarkName);
    if (landmark == nullptr) {
        failAgent(agent, "Unknown target: " + landmarkName);
        return false;
    }

    if (!isWalkable(world_, landmark->position)) {
        failAgent(agent, "Target is blocked: " + landmarkName);
        return false;
    }

    agent.task = TaskKind::Survey;
    agent.loopIndex = 0;
    return assignRoute(agent, landmark->position, landmark->name);
}

void Simulation::startPatrol(Agent& agent) {
    if (world_.patrolPoints.empty()) {
        failAgent(agent, "No patrol points in world");
        return;
    }
    const size_t index = static_cast<size_t>(agent.loopIndex) % world_.patrolPoints.size();
    const Vec2 target = world_.patrolPoints[index];
    const std::string label = "Patrol point " + std::to_string(index + 1);
    assignRoute(agent, target, label);
}

void Simulation::startDelivery(Agent& agent) {
    // Alternate between picking up at the depot and dropping off at a store.
    const bool pickupLeg = !agent.carryingPackage;
    const size_t storeCount = world_.landmarks.size() > 2 ? world_.landmarks.size() - 2 : 0;
    if (storeCount == 0) {
        failAgent(agent, "No delivery points in world");
        return;
    }

    if (pickupLeg) {
        const Landmark& depot = world_.landmarks[world_.depotIndex];
        assignRoute(agent, depot.position, depot.name + " (pickup)");
        return;
    }

    const size_t store = static_cast<size_t>(agent.loopIndex) % storeCount;
    const Landmark& dest = world_.landmarks[2 + store];
    assignRoute(agent, dest.position, dest.name);
}

void Simulation::startSurvey(Agent& agent) {
    const size_t count = world_.landmarks.size();
    if (count == 0) {
        failAgent(agent, "No landmarks in world");
        return;
    }
    const size_t index = static_cast<size_t>(agent.loopIndex) % count;
    const Landmark& lm = world_.landmarks[index];
    assignRoute(agent, lm.position, lm.name);
}

void Simulation::beginNextTask(Agent& agent) {
    agent.workTimer = 0.0f;
    agent.route.clear();
    agent.routeIndex = 0;
    agent.debugPath.clear();

    switch (agent.task) {
        case TaskKind::Patrol: startPatrol(agent); break;
        case TaskKind::Deliver: startDelivery(agent); break;
        case TaskKind::Survey: startSurvey(agent); break;
    }
}

void Simulation::failAgent(Agent& agent, const std::string& reason) {
    agent.state = AgentState::Failed;
    agent.route.clear();
    agent.routeIndex = 0;
    agent.debugPath.clear();
    agent.workTimer = 0.0f;
    agent.resultTimer = config_.resultHold;
    agent.lastResult = reason;
    ++agent.failedTasks;
}

void Simulation::completeAgentTask(Agent& agent, const std::string& note) {
    agent.state = AgentState::Completed;
    agent.lastResult = note;
    agent.resultTimer = config_.resultHold;
    ++agent.completedTasks;
    ++agent.loopIndex;

    if (agent.task == TaskKind::Deliver) {
        agent.carryingPackage = !agent.carryingPackage;
    }
}

void Simulation::advanceAlongRoute(Agent& agent, float dt) {
    float remaining = agent.speed * dt;

    while (remaining > 0.0f && agent.routeIndex + 1 < agent.route.size()) {
        const Vec2 current = agent.position;
        const Vec2 waypoint = agent.route[agent.routeIndex + 1];
        const float dist = distance(current, waypoint);

        if (dist <= remaining) {
            agent.position = waypoint;
            remaining -= dist;
            ++agent.routeIndex;
        } else {
            const Vec2 dir = normalized(waypoint - current);
            const Vec2 next = current + dir * remaining;
            agent.position = moveWithCollision(world_, current, next);
            remaining = 0.0f;
            // Face the direction of travel.
            agent.heading = turnToward(agent.heading, std::atan2(dir.x, dir.z), 6.0f * dt);
        }
    }

    // Arrival test is distance-based so it does not depend on route bookkeeping.
    if (distance(agent.position, agent.target) <= config_.arriveRadius) {
        agent.state = AgentState::Working;
        agent.workTimer = config_.workDuration;
    }
}

void Simulation::stepAgent(Agent& agent, float dt) {
    switch (agent.state) {
        case AgentState::Idle:
            break;

        case AgentState::Completed:
        case AgentState::Failed:
            // Hold the result briefly so the inspect panel and the world both
            // show what happened before the agent moves on.
            agent.resultTimer -= dt;
            break;

        case AgentState::Navigating:
            advanceAlongRoute(agent, dt);
            break;

        case AgentState::Working:
            agent.workTimer -= dt;
            if (agent.workTimer <= 0.0f) {
                agent.workTimer = 0.0f;
                completeAgentTask(agent, "Task complete at " + agent.targetName);
            }
            break;
    }
}

void Simulation::update(float dt) {
    if (dt <= 0.0f) return;

    // Clamp the step so a long stall (debugger, window drag) cannot teleport
    // agents across the town in one frame.
    const float step = std::min(dt, config_.maxStepDt);
    elapsed_ += step;

    for (Agent& agent : agents_) {
        stepAgent(agent, step);
    }

    // Agents that finished or failed pick up their next task once the result
    // has been visible long enough, so the town never sits still.
    for (Agent& agent : agents_) {
        if ((agent.state == AgentState::Completed || agent.state == AgentState::Failed) &&
            agent.resultTimer <= 0.0f) {
            beginNextTask(agent);
        }
    }
}

int Simulation::completedTaskCount() const {
    int total = 0;
    for (const Agent& a : agents_) total += a.completedTasks;
    return total;
}

int Simulation::failedTaskCount() const {
    int total = 0;
    for (const Agent& a : agents_) total += a.failedTasks;
    return total;
}

}  // namespace tt
