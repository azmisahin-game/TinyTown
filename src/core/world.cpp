#include "core/world.hpp"

#include <algorithm>
#include <iterator>

#include "core/random.hpp"

namespace tt {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Shared road grid. Three vertical and three horizontal streets keep the town
// compact and readable, and produce four city blocks.
constexpr int kRoadCount = 3;
constexpr float kRoadPositions[kRoadCount] = {-24.0f, 0.0f, 24.0f};

constexpr float kCellSize = 1.0f;

// Muted miniature-diorama palette.
constexpr Color3 kBuildingPalette[] = {
    {0.86f, 0.80f, 0.71f},
    {0.79f, 0.72f, 0.62f},
    {0.72f, 0.75f, 0.78f},
    {0.83f, 0.72f, 0.66f},
    {0.68f, 0.73f, 0.68f},
    {0.88f, 0.84f, 0.78f},
    {0.74f, 0.68f, 0.66f},
};

}  // namespace

World generateWorld(uint64_t seed) {
    World world;
    world.seed = seed;
    world.halfExtent = 48.0f;
    world.cellSize = kCellSize;
    world.roadHalfWidth = 3.5f;
    world.sidewalkWidth = 2.4f;

    const float half = world.halfExtent;
    const int cells = static_cast<int>(std::floor((half * 2.0f) / kCellSize));
    world.gridWidth = cells;
    world.gridHeight = cells;
    world.walkable.assign(static_cast<size_t>(cells) * static_cast<size_t>(cells), 1);

    world.roadLines.assign(std::begin(kRoadPositions), std::end(kRoadPositions));

    Rng rng(seed);

    // --- Blocks between the streets -----------------------------------------
    // Block interiors sit between sidewalk edges; the 2x2 arrangement matches
    // the four road-free quadrants.
    const float blockHalf = (kRoadPositions[1] - kRoadPositions[0]) * 0.5f -
                            world.roadHalfWidth - world.sidewalkWidth - 0.6f;

    struct BlockDef {
        float cx;
        float cz;
        bool park;
        bool depot;
    };
    const BlockDef blockDefs[] = {
        {-12.0f, -12.0f, true, false},   // park (north-west)
        {12.0f, -12.0f, false, false},   // residential (north-east)
        {-12.0f, 12.0f, false, true},    // depot (south-west)
        {12.0f, 12.0f, false, false},    // mixed (south-east)
    };

    int column = 0;
    for (const BlockDef& def : blockDefs) {
        Block block;
        block.rect = {def.cx - blockHalf, def.cz - blockHalf, def.cx + blockHalf, def.cz + blockHalf};
        block.isPark = def.park;
        block.isDepot = def.depot;
        block.column = column % 2;
        block.row = column / 2;
        world.blocks.push_back(block);
        ++column;
    }

    // --- Buildings -----------------------------------------------------------
    // The residential block reserves one slot for the landmark tower so the
    // tower never intersects a generated building.
    Vec2 towerSlot{0.0f, 0.0f};
    bool haveTowerSlot = false;

    for (const Block& block : world.blocks) {
        if (block.isPark) continue;

        // Fill the block with a small 2x2 cluster of buildings, with a gap so
        // the block never becomes a solid slab.
        const float margin = 0.9f;
        const Rect inner{block.rect.x0 + margin, block.rect.z0 + margin,
                         block.rect.x1 - margin, block.rect.z1 - margin};
        const int cols = 2;
        const int rows = 2;
        const float slotW = inner.width() / static_cast<float>(cols);
        const float slotD = inner.depth() / static_cast<float>(rows);

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                const float slotCx = inner.x0 + slotW * (static_cast<float>(c) + 0.5f);
                const float slotCz = inner.z0 + slotD * (static_cast<float>(r) + 0.5f);

                // The depot block keeps one slot empty for the loading yard.
                if (block.isDepot && r == 1 && c == 1) continue;

                if (!block.isDepot && !block.isPark && !haveTowerSlot && r == 0 && c == 1) {
                    towerSlot = {slotCx, slotCz};
                    haveTowerSlot = true;
                    continue;   // reserved for the tower
                }

                const float jitterX = rng.range(-0.25f, 0.25f);
                const float jitterZ = rng.range(-0.25f, 0.25f);

                const float w = rng.range(slotW * 0.52f, slotW * 0.82f);
                const float d = rng.range(slotD * 0.52f, slotD * 0.82f);
                const float cx = slotCx + jitterX;
                const float cz = slotCz + jitterZ;

                Building b;
                b.footprint = {cx - w * 0.5f, cz - d * 0.5f, cx + w * 0.5f, cz + d * 0.5f};
                b.height = rng.range(6.0f, 16.0f);
                const int paletteIndex =
                    rng.rangeInt(0, static_cast<int>(std::size(kBuildingPalette)) - 1);
                b.color = kBuildingPalette[paletteIndex];
                // A small colour variation keeps neighbours distinguishable.
                const float tint = rng.range(-0.03f, 0.03f);
                b.color.r = clampf(b.color.r + tint, 0.0f, 1.0f);
                b.color.g = clampf(b.color.g + tint, 0.0f, 1.0f);
                b.color.b = clampf(b.color.b + tint, 0.0f, 1.0f);
                b.windowCols = std::max(1, static_cast<int>(w / 2.0f));
                b.windowRows = std::max(1, static_cast<int>((b.height - 1.5f) / 2.4f));
                b.hasRoofDetail = rng.chance(0.75f);
                world.buildings.push_back(b);
            }
        }
    }

    // A single landmark tower makes the skyline readable.
    {
        const float w = 3.4f;
        const float d = 3.4f;
        Building tower;
        tower.footprint = {towerSlot.x - w * 0.5f, towerSlot.z - d * 0.5f,
                           towerSlot.x + w * 0.5f, towerSlot.z + d * 0.5f};
        tower.height = 24.0f;
        tower.color = {0.80f, 0.78f, 0.74f};
        tower.windowCols = 2;
        tower.windowRows = 8;
        tower.hasRoofDetail = true;
        world.buildings.push_back(tower);
    }

    // --- Rasterize blocking geometry into the grid ---------------------------
    auto markBlockedRect = [&](const Rect& rect) {
        const int x0 = std::max(0, gridCellX(world, {rect.x0, 0.0f}));
        const int x1 = std::min(world.gridWidth - 1, gridCellX(world, {rect.x1, 0.0f}));
        const int z0 = std::max(0, gridCellZ(world, {0.0f, rect.z0}));
        const int z1 = std::min(world.gridHeight - 1, gridCellZ(world, {0.0f, rect.z1}));
        for (int z = z0; z <= z1; ++z) {
            for (int x = x0; x <= x1; ++x) {
                world.walkable[static_cast<size_t>(z) * static_cast<size_t>(world.gridWidth) +
                               static_cast<size_t>(x)] = 0;
            }
        }
    };

    // Buildings block their footprint expanded by the agent radius, so the
    // grid already accounts for clearance.
    constexpr float kAgentRadius = 0.42f;
    for (const Building& b : world.buildings) {
        Rect expanded{b.footprint.x0 - kAgentRadius, b.footprint.z0 - kAgentRadius,
                      b.footprint.x1 + kAgentRadius, b.footprint.z1 + kAgentRadius};
        markBlockedRect(expanded);
    }

    // --- Props ---------------------------------------------------------------
    auto addProp = [&](PropType type, float x, float z, float scale) {
        Prop p;
        p.type = type;
        p.position = {x, z};
        p.scale = scale;
        p.rotation = rng.range(0.0f, 2.0f * kPi);
        world.props.push_back(p);
    };

    // Park: trees, bushes and benches in a loose grid, with a clear plaza.
    for (const Block& block : world.blocks) {
        if (!block.isPark) continue;
        const float step = 4.0f;
        for (float z = block.rect.z0 + 2.0f; z < block.rect.z1 - 1.0f; z += step) {
            for (float x = block.rect.x0 + 2.0f; x < block.rect.x1 - 1.0f; x += step) {
                if (std::fabs(x - block.rect.centerX()) < 3.0f &&
                    std::fabs(z - block.rect.centerZ()) < 3.0f) {
                    continue;  // leave the plaza open
                }
                if (rng.chance(0.72f)) {
                    addProp(PropType::Tree, x + rng.range(-0.6f, 0.6f),
                            z + rng.range(-0.6f, 0.6f), rng.range(0.8f, 1.3f));
                } else if (rng.chance(0.5f)) {
                    addProp(PropType::Bush, x + rng.range(-0.6f, 0.6f),
                            z + rng.range(-0.6f, 0.6f), rng.range(0.6f, 1.0f));
                } else {
                    addProp(PropType::Bench, x + rng.range(-0.6f, 0.6f),
                            z + rng.range(-0.6f, 0.6f), 1.0f);
                }
            }
        }
    }

    // Street lamps along the sidewalks and crates at the depot yard.
    for (float road : kRoadPositions) {
        for (float t = -half + 8.0f; t <= half - 8.0f; t += 16.0f) {
            addProp(PropType::Lamp, road + world.roadHalfWidth + world.sidewalkWidth * 0.5f, t, 1.0f);
            addProp(PropType::Lamp, road - world.roadHalfWidth - world.sidewalkWidth * 0.5f, t + 8.0f, 1.0f);
            addProp(PropType::Lamp, t, road + world.roadHalfWidth + world.sidewalkWidth * 0.5f, 1.0f);
            addProp(PropType::Lamp, t + 8.0f, road - world.roadHalfWidth - world.sidewalkWidth * 0.5f, 1.0f);
        }
    }

    for (const Block& block : world.blocks) {
        if (!block.isDepot) continue;
        for (int i = 0; i < 5; ++i) {
            addProp(PropType::Crate,
                    block.rect.centerX() + rng.range(-3.0f, 3.0f),
                    block.rect.centerZ() + rng.range(-3.0f, 3.0f),
                    rng.range(0.7f, 1.1f));
        }
    }

    // --- Landmarks, delivery points and patrol points ------------------------
    // Landmarks must sit on walkable ground; snapToWalkable guarantees that
    // without hand-tuning coordinates against the generated buildings.
    world.landmarks.clear();

    const Block* depotBlock = nullptr;
    const Block* parkBlock = nullptr;
    for (const Block& block : world.blocks) {
        if (block.isDepot) depotBlock = &block;
        if (block.isPark) parkBlock = &block;
    }

    auto addLandmark = [&](const std::string& name, Vec2 p) {
        world.landmarks.push_back({name, snapToWalkable(world, p)});
    };

    addLandmark("Depot", depotBlock ? Vec2{depotBlock->rect.centerX(), depotBlock->rect.centerZ()}
                                    : Vec2{-12.0f, 12.0f});
    addLandmark("Park Plaza", parkBlock ? Vec2{parkBlock->rect.centerX(), parkBlock->rect.centerZ()}
                                        : Vec2{-12.0f, -12.0f});

    // Delivery points sit on sidewalks in front of building blocks.
    const std::pair<const char*, Vec2> deliveryDefs[] = {
        {"North Store", {12.0f, -4.4f}},
        {"East Cafe", {4.4f, 12.0f}},
        {"South Workshop", {-12.0f, 4.4f}},
        {"West Market", {-4.4f, -12.0f}},
    };
    for (const auto& def : deliveryDefs) {
        addLandmark(def.first, def.second);
    }

    // Patrol corners are the sidewalk intersections of the road grid.
    for (float rx : kRoadPositions) {
        for (float rz : kRoadPositions) {
            const Vec2 p{rx + world.roadHalfWidth + world.sidewalkWidth * 0.5f,
                         rz + world.roadHalfWidth + world.sidewalkWidth * 0.5f};
            if (isWalkable(world, p)) world.patrolPoints.push_back(p);
        }
    }

    // The player starts on the sidewalk at the south end of the central street
    // looking north into the town, which shows the road, both sidewalks, the
    // tower and the park on the first frame.
    world.spawnPoint = snapToWalkable(world, {0.0f, 34.0f});
    world.spawnYaw = 0.0f;
    world.spawnPitch = -0.06f;

    // Lamps and trees are solid, so agents and the camera must treat them as
    // obstacles. Only the single cell the prop stands in is blocked: the
    // sidewalk is wide enough that the walking lane beside it stays clear.
    for (const Prop& prop : world.props) {
        if (prop.type != PropType::Lamp && prop.type != PropType::Tree) continue;
        const int ix = gridCellX(world, prop.position);
        const int iz = gridCellZ(world, prop.position);
        if (!insideGrid(world, ix, iz)) continue;
        world.walkable[static_cast<size_t>(iz) * world.gridWidth + static_cast<size_t>(ix)] = 0;
    }

    // Blocking a prop can invalidate a landmark that was snapped earlier, so
    // re-snap every landmark and the spawn point now that the grid is final.
    for (Landmark& landmark : world.landmarks) {
        landmark.position = snapToWalkable(world, landmark.position);
    }
    world.spawnPoint = snapToWalkable(world, world.spawnPoint);

    return world;
}

bool insideGrid(const World& world, int ix, int iz) {
    return ix >= 0 && iz >= 0 && ix < world.gridWidth && iz < world.gridHeight;
}

int gridCellX(const World& world, Vec2 p) {
    return static_cast<int>(std::floor((p.x + world.halfExtent) / world.cellSize));
}

int gridCellZ(const World& world, Vec2 p) {
    return static_cast<int>(std::floor((p.z + world.halfExtent) / world.cellSize));
}

int gridIndexFromWorld(const World& world, Vec2 p) {
    const int ix = gridCellX(world, p);
    const int iz = gridCellZ(world, p);
    if (!insideGrid(world, ix, iz)) return -1;
    return iz * world.gridWidth + ix;
}

Vec2 gridCellCenter(const World& world, int ix, int iz) {
    return {static_cast<float>(ix) * world.cellSize - world.halfExtent + world.cellSize * 0.5f,
            static_cast<float>(iz) * world.cellSize - world.halfExtent + world.cellSize * 0.5f};
}

bool isWalkable(const World& world, Vec2 p) {
    const int index = gridIndexFromWorld(world, p);
    if (index < 0) return false;
    return world.walkable[static_cast<size_t>(index)] != 0;
}

bool isBlocked(const World& world, Vec2 p) { return !isWalkable(world, p); }

Vec2 clampToWorld(const World& world, Vec2 p) {
    const float limit = world.halfExtent - 1.0f;
    return {clampf(p.x, -limit, limit), clampf(p.z, -limit, limit)};
}

Vec2 snapToWalkable(const World& world, Vec2 p) {
    const Vec2 clamped = clampToWorld(world, p);
    if (isWalkable(world, clamped)) return clamped;

    const int cx = gridCellX(world, clamped);
    const int cz = gridCellZ(world, clamped);
    const int maxRing = std::max(world.gridWidth, world.gridHeight);

    for (int ring = 1; ring <= maxRing; ++ring) {
        for (int dz = -ring; dz <= ring; ++dz) {
            for (int dx = -ring; dx <= ring; ++dx) {
                // Only walk the ring border; interior was covered by a smaller ring.
                if (std::abs(dx) != ring && std::abs(dz) != ring) continue;
                const int ix = cx + dx;
                const int iz = cz + dz;
                if (!insideGrid(world, ix, iz)) continue;
                if (world.walkable[static_cast<size_t>(iz) * world.gridWidth +
                                   static_cast<size_t>(ix)] == 0) {
                    continue;
                }
                return gridCellCenter(world, ix, iz);
            }
        }
    }
    return clamped;
}

Vec2 moveWithCollision(const World& world, Vec2 from, Vec2 to) {
    Vec2 result = from;

    // Axis-separated movement: try each axis independently so the mover slides
    // along a wall instead of stopping dead.
    if (isWalkable(world, {to.x, result.z})) result.x = to.x;
    if (isWalkable(world, {result.x, to.z})) result.z = to.z;

    return clampToWorld(world, result);
}

}  // namespace tt
