#include "core/world.hpp"

#include <set>
#include <string>

#include "test_framework.hpp"

using namespace tt;

namespace {

// Fingerprint a world so determinism can be checked across two generations.
std::string worldFingerprint(const World& world) {
    std::string fp;
    fp.reserve(world.buildings.size() * 32);
    for (const Building& b : world.buildings) {
        fp += std::to_string(static_cast<int>(b.footprint.x0 * 100.0f));
        fp += ",";
        fp += std::to_string(static_cast<int>(b.footprint.z0 * 100.0f));
        fp += ",";
        fp += std::to_string(static_cast<int>(b.height * 100.0f));
        fp += ";";
    }
    for (const Prop& p : world.props) {
        fp += std::to_string(static_cast<int>(p.position.x * 100.0f));
        fp += ",";
        fp += std::to_string(static_cast<int>(p.position.z * 100.0f));
        fp += ";";
    }
    return fp;
}

int countWalkable(const World& world) {
    int count = 0;
    for (uint8_t cell : world.walkable) {
        if (cell != 0) ++count;
    }
    return count;
}

}  // namespace

TEST(World, SameSeedProducesIdenticalWorld) {
    const World a = generateWorld(20260117ULL);
    const World b = generateWorld(20260117ULL);

    CHECK_EQ(a.buildings.size(), b.buildings.size());
    CHECK_EQ(a.props.size(), b.props.size());
    CHECK_EQ(a.landmarks.size(), b.landmarks.size());
    CHECK_EQ(worldFingerprint(a), worldFingerprint(b));

    // Walkability grid must match byte for byte.
    CHECK(a.walkable == b.walkable);
}

TEST(World, DifferentSeedsProduceDifferentWorlds) {
    const World a = generateWorld(1ULL);
    const World b = generateWorld(2ULL);

    CHECK_MSG(worldFingerprint(a) != worldFingerprint(b),
              "two different seeds produced the same layout");
}

TEST(World, HasExpectedStructure) {
    const World world = generateWorld(1337ULL);

    CHECK(world.gridWidth > 0);
    CHECK_EQ(world.gridWidth, world.gridHeight);
    CHECK_EQ(world.walkable.size(),
             static_cast<size_t>(world.gridWidth) * static_cast<size_t>(world.gridHeight));

    CHECK_MSG(world.buildings.size() >= 8, "town should have a visible cluster of buildings");
    CHECK_MSG(world.props.size() >= 10, "town should have street/park props");
    CHECK_MSG(world.landmarks.size() >= 4, "need a depot, plaza and storefronts");
    CHECK_MSG(world.patrolPoints.size() >= 3, "need several patrol corners");

    CHECK_EQ(world.landmarks[world.depotIndex].name, std::string("Depot"));
    CHECK_EQ(world.landmarks[world.plazaIndex].name, std::string("Park Plaza"));
}

TEST(World, LandmarksAndSpawnAreWalkable) {
    // Check several seeds, not just the default, so a generation change cannot
    // silently place a landmark inside a building.
    for (uint64_t seed : {1ULL, 42ULL, 1337ULL, 99991ULL}) {
        const World world = generateWorld(seed);
        for (const Landmark& lm : world.landmarks) {
            CHECK_MSG(isWalkable(world, lm.position), "landmark is blocked: " + lm.name);
        }
        CHECK_MSG(isWalkable(world, world.spawnPoint), "spawn point is blocked");
        for (const Vec2& p : world.patrolPoints) {
            CHECK_MSG(isWalkable(world, p), "patrol point is blocked");
        }
    }
}

TEST(World, BuildingsBlockTheirFootprint) {
    const World world = generateWorld(1337ULL);
    for (const Building& b : world.buildings) {
        const Vec2 center{b.footprint.centerX(), b.footprint.centerZ()};
        CHECK_MSG(isBlocked(world, center), "building centre should be blocked");
    }
}

TEST(World, WalkableAreaIsUsableButNotEverything) {
    const World world = generateWorld(1337ULL);
    const int total = world.gridWidth * world.gridHeight;
    const int walkable = countWalkable(world);
    CHECK(walkable > total / 3);
    CHECK(walkable < total);  // buildings must occupy real space
}

TEST(World, SnapToWalkableAlwaysFindsFreeGround) {
    const World world = generateWorld(1337ULL);
    // Probe a coarse lattice across the map, including points inside buildings.
    for (float z = -world.halfExtent; z <= world.halfExtent; z += 5.0f) {
        for (float x = -world.halfExtent; x <= world.halfExtent; x += 5.0f) {
            const Vec2 snapped = snapToWalkable(world, {x, z});
            CHECK_MSG(isWalkable(world, snapped), "snapToWalkable returned a blocked cell");
        }
    }
}

TEST(World, CollisionStopsAtWalls) {
    const World world = generateWorld(1337ULL);

    // Aim straight at a building centre from a walkable spot; the mover must
    // not end up inside the building.
    const Building& b = world.buildings.front();
    const Vec2 buildingCenter{b.footprint.centerX(), b.footprint.centerZ()};
    const Vec2 start = snapToWalkable(world, {buildingCenter.x, buildingCenter.z + 12.0f});

    Vec2 pos = start;
    for (int i = 0; i < 400; ++i) {
        const Vec2 dir = normalized(buildingCenter - pos);
        pos = moveWithCollision(world, pos, pos + dir * 0.25f);
    }

    CHECK_MSG(isWalkable(world, pos), "mover ended up inside blocked geometry");
    CHECK_MSG(!contains(b.footprint, pos), "mover entered the building footprint");
}
