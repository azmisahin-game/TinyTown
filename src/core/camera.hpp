#pragma once

// First-person camera state and movement. Pure math plus a collision query
// against the world grid, so it can be unit tested without a window.

#include "core/types.hpp"
#include "core/world.hpp"

namespace tt {

struct CameraInput {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool sprint = false;
};

// Pitch is limited so the camera can never flip over.
float clampPitch(float pitch);

class Camera {
public:
    void reset(Vec2 position, float yaw, float pitch);

    // Frame-rate independent movement. `dt` is clamped internally so a long
    // stall cannot teleport the camera through the world.
    void update(const World& world, const CameraInput& input, float dt);

    void look(float deltaYaw, float deltaPitch);

    Vec2 position2D() const { return {position_.x, position_.z}; }
    Vec3 position() const { return position_; }
    void setPosition(Vec2 p) { position_.x = p.x; position_.z = p.z; }

    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }
    void setYaw(float yaw) { yaw_ = yaw; }
    void setPitch(float pitch) { pitch_ = clampPitch(pitch); }

    float eyeHeight() const { return eyeHeight_; }
    // Changing the eye height keeps the ground position; used by the elevated
    // overview mode so the same controls work at any height.
    void setEyeHeight(float height);

    // Forward direction on the ground plane (unit length).
    Vec2 forward2D() const;

private:
    Vec3 position_{0.0f, 1.7f, 0.0f};
    float yaw_ = 0.0f;
    float pitch_ = 0.0f;
    float eyeHeight_ = 1.7f;
};

}  // namespace tt
