#pragma once

// Procedural town data. Pure C++20: no graphics calls, no platform headers.
//
// The town is a small diorama: a road grid with sidewalks, blocks filled with
// stylized buildings, a park, a depot and street props. Everything is derived
// from a single 64-bit seed, so the same seed always produces the same town.

#include <cstdint>
#include <string>
#include <vector>

#include "core/types.hpp"

namespace tt {

struct Building {
    Rect footprint;
    float height = 8.0f;
    Color3 color{};
    int windowCols = 2;
    int windowRows = 3;
    bool hasRoofDetail = true;
};

enum class PropType { Tree, Lamp, Bench, Crate, Bush };

struct Prop {
    PropType type = PropType::Tree;
    Vec2 position{};
    float scale = 1.0f;
    float rotation = 0.0f;
};

struct Block {
    Rect rect;          // interior ground (grass / pavement)
    bool isPark = false;
    bool isDepot = false;
    int column = 0;
    int row = 0;
};

// A labelled place an agent can be sent to. Kept explicit so the inspect panel
// can report a human-readable destination instead of raw coordinates.
struct Landmark {
    std::string name;
    Vec2 position{};
};

struct World {
    uint64_t seed = 0;
    float halfExtent = 48.0f;   // world spans [-halfExtent, +halfExtent]
    float cellSize = 1.0f;

    int gridWidth = 0;
    int gridHeight = 0;
    std::vector<uint8_t> walkable;   // 1 = free, 0 = blocked (buildings/border)

    std::vector<Building> buildings;
    std::vector<Prop> props;
    std::vector<Block> blocks;

    float roadHalfWidth = 3.5f;
    float sidewalkWidth = 1.6f;
    std::vector<float> roadLines;        // shared x/z coordinates of roads

    // Named destinations. Index 0 is always the depot and index 1 the plaza;
    // the remaining entries are storefront delivery points.
    std::vector<Landmark> landmarks;
    int depotIndex = 0;
    int plazaIndex = 1;

    std::vector<Vec2> patrolPoints;      // street corners for patrolling
    Vec2 spawnPoint{};                   // player start position
    float spawnYaw = 0.0f;               // initial camera yaw
    float spawnPitch = 0.0f;             // initial camera pitch
};

// Build a complete town from a seed. Deterministic: identical seed, identical
// world, on every platform and compiler.
World generateWorld(uint64_t seed);

// Grid helpers -----------------------------------------------------------------

bool insideGrid(const World& world, int ix, int iz);

// Convert a world-space point to a grid cell index (may be outside the grid).
int gridIndexFromWorld(const World& world, Vec2 p);
int gridCellX(const World& world, Vec2 p);
int gridCellZ(const World& world, Vec2 p);

Vec2 gridCellCenter(const World& world, int ix, int iz);

// True when the point is inside the grid and not blocked.
bool isWalkable(const World& world, Vec2 p);

// True when the point is blocked or outside the grid. This is the inverse of
// isWalkable and is what collision checks use.
bool isBlocked(const World& world, Vec2 p);

// Clamp a point into the world bounds (does not resolve building collisions).
Vec2 clampToWorld(const World& world, Vec2 p);

// Find the nearest walkable cell centre to `p`, searching outward in rings.
// Returns `p` unchanged only if the whole grid is blocked (which never happens
// for a generated town). Used so landmarks and spawn points are always valid.
Vec2 snapToWalkable(const World& world, Vec2 p);

// Move from `from` toward `to`, sliding along blocked axes. Returns the final
// position. Never ends up inside a blocked cell.
//
// The walkability grid is already expanded by the agent radius during world
// generation, so a centre-point test is the correct clearance check here and
// keeps movement exactly consistent with the pathfinder.
Vec2 moveWithCollision(const World& world, Vec2 from, Vec2 to);

}  // namespace tt
