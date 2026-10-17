#include "core/camera.hpp"

#include "test_framework.hpp"

using namespace tt;

namespace {

CameraInput inputOf(bool f, bool b, bool l, bool r, bool sprint = false) {
    CameraInput in;
    in.forward = f;
    in.backward = b;
    in.left = l;
    in.right = r;
    in.sprint = sprint;
    return in;
}

}  // namespace

TEST(Camera, StartsAtRequestedPosition) {
    const World world = generateWorld(1337ULL);
    Camera camera;
    camera.reset(world.spawnPoint, 0.0f, 0.0f);

    CHECK_NEAR(camera.position2D().x, world.spawnPoint.x, 0.0001f);
    CHECK_NEAR(camera.position2D().z, world.spawnPoint.z, 0.0001f);
    CHECK_NEAR(camera.eyeHeight(), 1.7f, 0.0001f);
}

TEST(Camera, MovementIsFrameRateIndependent) {
    const World world = generateWorld(1337ULL);

    Camera coarse;
    Camera fine;
    coarse.reset({0.0f, 24.0f}, 0.0f, 0.0f);
    fine.reset({0.0f, 24.0f}, 0.0f, 0.0f);

    // 1 second of forward movement at 10 FPS vs 100 FPS.
    for (int i = 0; i < 20; ++i) coarse.update(world, inputOf(true, false, false, false), 0.05f);
    for (int i = 0; i < 200; ++i) fine.update(world, inputOf(true, false, false, false), 0.005f);

    CHECK_NEAR(coarse.position2D().x, fine.position2D().x, 0.05f);
    CHECK_NEAR(coarse.position2D().z, fine.position2D().z, 0.05f);
}

TEST(Camera, LargeDtIsClampedLikeOneSmallStep) {
    const World world = generateWorld(1337ULL);

    Camera big;
    Camera small;
    big.reset({0.0f, 24.0f}, 0.0f, 0.0f);
    small.reset({0.0f, 24.0f}, 0.0f, 0.0f);

    big.update(world, inputOf(true, false, false, false), 10.0f);
    small.update(world, inputOf(true, false, false, false), 0.05f);

    CHECK_NEAR(big.position2D().x, small.position2D().x, 0.0001f);
    CHECK_NEAR(big.position2D().z, small.position2D().z, 0.0001f);
}

TEST(Camera, SprintIsFasterButStillClampedPerStep) {
    const World world = generateWorld(1337ULL);

    Camera walk;
    Camera run;
    walk.reset({0.0f, 24.0f}, 0.0f, 0.0f);
    run.reset({0.0f, 24.0f}, 0.0f, 0.0f);

    for (int i = 0; i < 10; ++i) walk.update(world, inputOf(true, false, false, false), 0.05f);
    for (int i = 0; i < 10; ++i) run.update(world, inputOf(true, false, false, false, true), 0.05f);

    CHECK(distance(run.position2D(), {0.0f, 24.0f}) >
          distance(walk.position2D(), {0.0f, 24.0f}));
}

TEST(Camera, CannotLeaveWorldBounds) {
    const World world = generateWorld(1337ULL);
    Camera camera;
    camera.reset({0.0f, 24.0f}, 0.0f, 0.0f);

    // Push hard in every direction for a long time.
    for (int i = 0; i < 4000; ++i) {
        camera.update(world, inputOf(true, false, false, false), 0.05f);
    }
    const float limit = world.halfExtent + 0.001f;
    CHECK(std::fabs(camera.position2D().x) <= limit);
    CHECK(std::fabs(camera.position2D().z) <= limit);
    CHECK_MSG(isWalkable(world, camera.position2D()), "camera ended inside blocked geometry");
}

TEST(Camera, PitchIsClamped) {
    Camera camera;
    camera.reset({0.0f, 0.0f}, 0.0f, 0.0f);

    for (int i = 0; i < 100; ++i) camera.look(0.0f, 1.0f);
    CHECK(camera.pitch() <= 1.4501f);

    for (int i = 0; i < 200; ++i) camera.look(0.0f, -1.0f);
    CHECK(camera.pitch() >= -1.4501f);
}

TEST(Camera, DiagonalMovementIsNotFaster) {
    const World world = generateWorld(1337ULL);

    Camera straight;
    Camera diagonal;
    straight.reset({0.0f, 24.0f}, 0.0f, 0.0f);
    diagonal.reset({0.0f, 24.0f}, 0.0f, 0.0f);

    for (int i = 0; i < 10; ++i) straight.update(world, inputOf(true, false, false, false), 0.05f);
    for (int i = 0; i < 10; ++i) diagonal.update(world, inputOf(true, false, true, false), 0.05f);
    const float straightDist = distance(straight.position2D(), {0.0f, 24.0f});
    const float diagonalDist = distance(diagonal.position2D(), {0.0f, 24.0f});
    CHECK_NEAR(straightDist, diagonalDist, 0.05f);
}
