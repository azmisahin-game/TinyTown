#pragma once

// Immediate-mode OpenGL renderer for the town and its agents.
//
// The reference prototype drew with glBegin/glVertex3f, and that technique is
// kept here on purpose: it is the smallest path to a working, stylized scene
// and needs no shader or asset pipeline. It requires an OpenGL compatibility
// profile, which is why the SDL window requests one explicitly.
//
// The renderer only reads simulation state. It never decides whether a task
// succeeded; that is the simulation's job.

#include <string>
#include <vector>

#include "core/camera.hpp"
#include "core/simulation.hpp"
#include "core/types.hpp"
#include "core/world.hpp"

namespace tt {

class Renderer {
public:
    bool initialize(std::string& error);
    void shutdown();

    void resize(int width, int height);

    // Draw one frame. `camera` supplies the view; `sim` supplies the world and
    // agents; `selectedAgent` is highlighted (-1 for none).
    void beginFrame(const Camera& camera, float timeSeconds);
    void drawWorld(const World& world);
    void drawAgents(const std::vector<Agent>& agents, int selectedAgent, float timeSeconds);
    void endFrame();

    // Read the current framebuffer. Returns RGB8 data, bottom-up as OpenGL
    // produces it. Used by the screenshot path.
    std::vector<unsigned char> readPixels(int& outWidth, int& outHeight) const;

    int width() const { return width_; }
    int height() const { return height_; }

private:
    void applyProjection() const;

    int width_ = 1280;
    int height_ = 720;
    bool initialized_ = false;
};

}  // namespace tt
