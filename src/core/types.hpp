#pragma once

// Plain value types shared by world generation, pathfinding, simulation and
// rendering. Nothing here depends on a window, a GPU or a platform API.

#include <cmath>

namespace tt {

struct Vec2 {
    float x = 0.0f;
    float z = 0.0f;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Color3 {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.z + b.z}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.z - b.z}; }
inline Vec2 operator*(Vec2 a, float s) { return {a.x * s, a.z * s}; }

inline float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.z * b.z; }
inline float length(Vec2 a) { return std::sqrt(dot(a, a)); }
inline float lengthSquared(Vec2 a) { return dot(a, a); }

inline Vec2 normalized(Vec2 a) {
    const float len = length(a);
    return len > 1e-6f ? Vec2{a.x / len, a.z / len} : Vec2{0.0f, 0.0f};
}

inline float distance(Vec2 a, Vec2 b) { return length(b - a); }

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// A screen/world axis-aligned rectangle on the ground plane.
struct Rect {
    float x0 = 0.0f;
    float z0 = 0.0f;
    float x1 = 0.0f;
    float z1 = 0.0f;

    float width() const { return x1 - x0; }
    float depth() const { return z1 - z0; }
    float centerX() const { return (x0 + x1) * 0.5f; }
    float centerZ() const { return (z0 + z1) * 0.5f; }
};

inline bool contains(const Rect& r, Vec2 p) {
    return p.x >= r.x0 && p.x <= r.x1 && p.z >= r.z0 && p.z <= r.z1;
}

}  // namespace tt
