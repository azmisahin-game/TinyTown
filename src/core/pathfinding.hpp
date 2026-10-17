#pragma once

// Small grid pathfinder. Deliberately not a general navigation framework: it
// exists so a handful of agents can walk between landmarks without clipping
// through buildings. "No route" is a normal, explicit outcome.

#include <vector>

#include "core/types.hpp"
#include "core/world.hpp"

namespace tt {

struct PathResult {
    bool found = false;
    std::vector<Vec2> points;   // includes start and goal when found
    int cellsExplored = 0;
};

// A* on the walkability grid, 8-connected, without corner cutting.
// The returned polyline is smoothed with a line-of-sight pass so agents do not
// visibly zig-zag along grid cells.
PathResult findPath(const World& world, Vec2 start, Vec2 goal);

// True when a straight segment between two walkable points stays walkable.
bool hasLineOfSight(const World& world, Vec2 a, Vec2 b);

}  // namespace tt
