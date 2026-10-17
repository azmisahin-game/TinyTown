#include "core/pathfinding.hpp"

#include <string>

#include "test_framework.hpp"

using namespace tt;

namespace {

// Confirm every segment of a path stays on walkable ground. This is the real
// guarantee that matters: agents must never clip through a building.
bool pathIsClear(const World& world, const PathResult& path) {
    if (!path.found) return false;
    for (size_t i = 0; i + 1 < path.points.size(); ++i) {
        if (!hasLineOfSight(world, path.points[i], path.points[i + 1])) return false;
    }
    return true;
}

float pathLength(const PathResult& path) {
    float total = 0.0f;
    for (size_t i = 0; i + 1 < path.points.size(); ++i) {
        total += distance(path.points[i], path.points[i + 1]);
    }
    return total;
}

}  // namespace

TEST(Pathfinding, FindsRouteBetweenLandmarks) {
    const World world = generateWorld(1337ULL);

    const Vec2 depot = world.landmarks[world.depotIndex].position;
    const Vec2 plaza = world.landmarks[world.plazaIndex].position;

    const PathResult path = findPath(world, depot, plaza);
    CHECK(path.found);
    CHECK(path.points.size() >= 2);
    CHECK(pathIsClear(world, path));
    CHECK(path.cellsExplored > 0);

    // Endpoints must match the requested points exactly.
    CHECK_NEAR(path.points.front().x, depot.x, 0.001f);
    CHECK_NEAR(path.points.front().z, depot.z, 0.001f);
    CHECK_NEAR(path.points.back().x, plaza.x, 0.001f);
    CHECK_NEAR(path.points.back().z, plaza.z, 0.001f);
}

TEST(Pathfinding, EveryLandmarkPairIsReachable) {
    const World world = generateWorld(1337ULL);
    for (const Landmark& from : world.landmarks) {
        for (const Landmark& to : world.landmarks) {
            const PathResult path = findPath(world, from.position, to.position);
            CHECK_MSG(path.found, "no route from " + from.name + " to " + to.name);
            CHECK_MSG(pathIsClear(world, path),
                      "path crosses blocked geometry: " + from.name + " -> " + to.name);
        }
    }
}

TEST(Pathfinding, SamePointProducesTrivialPath) {
    const World world = generateWorld(1337ULL);
    const Vec2 depot = world.landmarks[world.depotIndex].position;
    const PathResult path = findPath(world, depot, depot);
    CHECK(path.found);
    CHECK_EQ(path.points.size(), static_cast<size_t>(2));
}

TEST(Pathfinding, BlockedStartHasNoRoute) {
    const World world = generateWorld(1337ULL);
    const Vec2 insideBuilding{world.buildings.front().footprint.centerX(),
                              world.buildings.front().footprint.centerZ()};
    const PathResult path = findPath(world, insideBuilding, world.landmarks[0].position);
    CHECK_MSG(!path.found, "a start inside a building must report no route");
    CHECK(path.points.empty());
}

TEST(Pathfinding, BlockedGoalHasNoRoute) {
    const World world = generateWorld(1337ULL);
    const Vec2 insideBuilding{world.buildings.front().footprint.centerX(),
                              world.buildings.front().footprint.centerZ()};
    const PathResult path = findPath(world, world.landmarks[0].position, insideBuilding);
    CHECK_MSG(!path.found, "a goal inside a building must report no route");
    CHECK(path.points.empty());
}

TEST(Pathfinding, OutOfBoundsTargetHasNoRoute) {
    const World world = generateWorld(1337ULL);
    const Vec2 outside{world.halfExtent + 50.0f, 0.0f};
    const PathResult path = findPath(world, world.landmarks[0].position, outside);
    CHECK(!path.found);
}

TEST(Pathfinding, WallBetweenPointsBlocksDirectLine) {
    const World world = generateWorld(1337ULL);
    const Building& b = world.buildings.front();
    const Vec2 left{b.footprint.x0 - 2.0f, b.footprint.centerZ()};
    const Vec2 right{b.footprint.x1 + 2.0f, b.footprint.centerZ()};

    // If both sides are walkable, a straight line through the building must be
    // rejected, and the route must be longer than the straight-line distance.
    if (isWalkable(world, left) && isWalkable(world, right)) {
        CHECK_MSG(!hasLineOfSight(world, left, right),
                  "line through a building should be blocked");

        const PathResult path = findPath(world, left, right);
        CHECK(path.found);
        CHECK(pathIsClear(world, path));
        CHECK_MSG(pathLength(path) > distance(left, right) + 0.5f,
                  "route should detour around the building");
    }
}

TEST(Pathfinding, PathIsDeterministic) {
    const World world = generateWorld(1337ULL);
    const Vec2 a = world.landmarks[0].position;
    const Vec2 b = world.landmarks[2].position;

    const PathResult first = findPath(world, a, b);
    const PathResult second = findPath(world, a, b);

    CHECK_EQ(first.found, second.found);
    CHECK_EQ(first.points.size(), second.points.size());
    for (size_t i = 0; i < first.points.size(); ++i) {
        CHECK_NEAR(first.points[i].x, second.points[i].x, 0.0f);
        CHECK_NEAR(first.points[i].z, second.points[i].z, 0.0f);
    }
}

TEST(Pathfinding, HasLineOfSightRejectsBlockedEndpoints) {
    const World world = generateWorld(1337ULL);
    const Vec2 blocked{world.buildings.front().footprint.centerX(),
                       world.buildings.front().footprint.centerZ()};
    CHECK(!hasLineOfSight(world, world.landmarks[0].position, blocked));
}
