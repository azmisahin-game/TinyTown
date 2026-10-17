#include "core/camera.hpp"

#include <algorithm>
#include <cmath>

namespace tt {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kMaxPitch = 1.45f;
constexpr float kMaxStepDt = 0.05f;
constexpr float kBaseSpeed = 6.0f;
constexpr float kSprintMultiplier = 2.0f;
// The camera keeps a wider clearance than an agent: it should not clip into a
// wall when the player strafes along it.
constexpr float kCameraRadius = 0.35f;

}  // namespace

float clampPitch(float pitch) { return clampf(pitch, -kMaxPitch, kMaxPitch); }

void Camera::reset(Vec2 position, float yaw, float pitch) {
    position_ = {position.x, eyeHeight_, position.z};
    yaw_ = yaw;
    pitch_ = clampPitch(pitch);
}

Vec2 Camera::forward2D() const { return {std::sin(yaw_), -std::cos(yaw_)}; }

void Camera::setEyeHeight(float height) {
    eyeHeight_ = std::max(0.2f, height);
    position_.y = eyeHeight_;
}

void Camera::look(float deltaYaw, float deltaPitch) {
    yaw_ += deltaYaw;
    while (yaw_ > kPi) yaw_ -= 2.0f * kPi;
    while (yaw_ < -kPi) yaw_ += 2.0f * kPi;
    pitch_ = clampPitch(pitch_ + deltaPitch);
}

void Camera::update(const World& world, const CameraInput& input, float dt) {
    if (dt <= 0.0f) return;
    const float step = std::min(dt, kMaxStepDt);

    float forward = 0.0f;
    float side = 0.0f;
    if (input.forward) forward += 1.0f;
    if (input.backward) forward -= 1.0f;
    if (input.right) side += 1.0f;
    if (input.left) side -= 1.0f;

    // Normalize so diagonal movement is not faster than straight movement.
    const float magnitude = std::sqrt(forward * forward + side * side);
    if (magnitude <= 0.0f) return;
    forward /= magnitude;
    side /= magnitude;

    float speed = kBaseSpeed;
    if (input.sprint) speed *= kSprintMultiplier;

    const float fx = std::sin(yaw_);
    const float fz = -std::cos(yaw_);
    const float sx = std::cos(yaw_);
    const float sz = std::sin(yaw_);

    const float dx = (fx * forward + sx * side) * speed * step;
    const float dz = (fz * forward + sz * side) * speed * step;

    const Vec2 from{position_.x, position_.z};

    // Probe the four corners of the camera footprint so the eye cannot end up
    // half inside a wall.
    auto canStandAt = [&](float x, float z) {
        const Vec2 probes[4] = {
            {x + kCameraRadius, z},
            {x - kCameraRadius, z},
            {x, z + kCameraRadius},
            {x, z - kCameraRadius},
        };
        if (isBlocked(world, {x, z})) return false;
        for (const Vec2& p : probes) {
            if (isBlocked(world, p)) return false;
        }
        return true;
    };

    Vec2 result = from;
    if (canStandAt(from.x + dx, result.z)) result.x += dx;
    if (canStandAt(result.x, from.z + dz)) result.z += dz;

    result = clampToWorld(world, result);
    position_.x = result.x;
    position_.z = result.z;
}

}  // namespace tt
