#include "render/renderer.hpp"

#include <SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace tt {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Directional-light shading factors per face. Cheap, stable and gives the
// diorama a readable sense of depth without a lighting pipeline.
constexpr float kShadeFront = 0.86f;
constexpr float kShadeBack = 0.70f;
constexpr float kShadeLeft = 0.78f;
constexpr float kShadeRight = 0.94f;
constexpr float kShadeTop = 1.06f;
constexpr float kShadeBottom = 0.60f;

struct Palette {
    Color3 sky{0.58f, 0.72f, 0.85f};
    Color3 ground{0.52f, 0.60f, 0.44f};
    Color3 grass{0.47f, 0.62f, 0.40f};
    Color3 asphalt{0.40f, 0.41f, 0.44f};
    Color3 sidewalk{0.70f, 0.69f, 0.66f};
    Color3 roadLine{0.94f, 0.88f, 0.62f};
    Color3 roof{0.50f, 0.36f, 0.31f};
    Color3 window{0.40f, 0.66f, 0.80f};
    Color3 windowLit{0.99f, 0.90f, 0.62f};
    Color3 treeTrunk{0.40f, 0.30f, 0.21f};
    Color3 treeLeaf{0.35f, 0.57f, 0.32f};
    Color3 bush{0.38f, 0.58f, 0.33f};
    Color3 lampPost{0.34f, 0.34f, 0.37f};
    Color3 lampGlow{1.00f, 0.94f, 0.72f};
    Color3 bench{0.58f, 0.41f, 0.27f};
    Color3 crate{0.68f, 0.52f, 0.33f};
    Color3 depotPad{0.62f, 0.60f, 0.57f};
    Color3 agentSkin{0.94f, 0.80f, 0.68f};
    Color3 package{0.78f, 0.57f, 0.31f};
};

const Palette kPalette;

float shade(float channel, float factor) { return clampf(channel * factor, 0.0f, 1.0f); }

void setColor(const Color3& c, float factor) {
    glColor3f(shade(c.r, factor), shade(c.g, factor), shade(c.b, factor));
}

// Axis-aligned box. `y` is the bottom, so height grows upward.
void box(float x, float y, float z, float w, float h, float d, const Color3& color) {
    const float x0 = x - w * 0.5f;
    const float x1 = x + w * 0.5f;
    const float z0 = z - d * 0.5f;
    const float z1 = z + d * 0.5f;
    const float y0 = y;
    const float y1 = y + h;

    glBegin(GL_QUADS);

    setColor(color, kShadeFront);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    setColor(color, kShadeBack);
    glVertex3f(x1, y0, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);

    setColor(color, kShadeLeft);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);

    setColor(color, kShadeRight);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);

    setColor(color, kShadeTop);
    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0);

    setColor(color, kShadeBottom);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    glEnd();
}

// A low-poly blob used for tree canopies and bushes.
void blob(float x, float y, float z, float radius, float squash, const Color3& color) {
    constexpr int kSides = 7;
    constexpr int kRings = 3;
    for (int ring = 0; ring < kRings; ++ring) {
        const float t0 = static_cast<float>(ring) / static_cast<float>(kRings);
        const float t1 = static_cast<float>(ring + 1) / static_cast<float>(kRings);
        const float r0 = radius * std::sin(t0 * kPi);
        const float r1 = radius * std::sin(t1 * kPi);
        const float y0 = y + radius * squash * (-std::cos(t0 * kPi));
        const float y1 = y + radius * squash * (-std::cos(t1 * kPi));

        glBegin(GL_QUAD_STRIP);
        for (int s = 0; s <= kSides; ++s) {
            const float a = (2.0f * kPi * static_cast<float>(s)) / static_cast<float>(kSides);
            const float ca = std::cos(a);
            const float sa = std::sin(a);
            setColor(color, kShadeTop - 0.18f * t0);
            glVertex3f(x + ca * r0, y0, z + sa * r0);
            glVertex3f(x + ca * r1, y1, z + sa * r1);
        }
        glEnd();
    }
}

void cylinder(float x, float y, float z, float radius, float height, const Color3& color) {
    constexpr int kSides = 6;
    glBegin(GL_QUAD_STRIP);
    for (int s = 0; s <= kSides; ++s) {
        const float a = (2.0f * kPi * static_cast<float>(s)) / static_cast<float>(kSides);
        const float ca = std::cos(a);
        const float sa = std::sin(a);
        setColor(color, kShadeRight);
        glVertex3f(x + ca * radius, y, z + sa * radius);
        glVertex3f(x + ca * radius, y + height, z + sa * radius);
    }
    glEnd();
}

void groundPlane(const Color3& color, float extent) {
    glBegin(GL_QUADS);
    setColor(color, 1.0f);
    glVertex3f(-extent, 0.0f, extent);
    glVertex3f(extent, 0.0f, extent);
    glVertex3f(extent, 0.0f, -extent);
    glVertex3f(-extent, 0.0f, -extent);
    glEnd();
}

}  // namespace

bool Renderer::initialize(std::string& error) {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_FLAT);
    glClearColor(kPalette.sky.r, kPalette.sky.g, kPalette.sky.b, 1.0f);
    glClearDepth(1.0);

    // A cheap fog makes distant geometry fall away into the sky colour, which
    // reads as depth in a small diorama. It starts well beyond the town so the
    // world itself stays clear and only the far ground plane fades.
    glEnable(GL_FOG);
    const GLfloat fogColor[4] = {kPalette.sky.r, kPalette.sky.g, kPalette.sky.b, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 90.0f);
    glFogf(GL_FOG_END, 260.0f);

    const GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        error = "OpenGL reported an error during initialisation (code " +
                std::to_string(static_cast<int>(err)) + ")";
        return false;
    }

    initialized_ = true;
    return true;
}

void Renderer::shutdown() { initialized_ = false; }

void Renderer::resize(int width, int height) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);
    glViewport(0, 0, width_, height_);
}

void Renderer::applyProjection() const {
    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
    const float fovDegrees = 70.0f;
    const float nearZ = 0.1f;
    const float farZ = 260.0f;

    const float top = nearZ * std::tan(fovDegrees * kPi / 360.0f);
    const float right = top * aspect;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-right, right, -top, top, nearZ, farZ);
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::beginFrame(const Camera& camera, float /*timeSeconds*/) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyProjection();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const float pitch = camera.pitch();
    const float yaw = camera.yaw();
    const Vec3 eye = camera.position();

    // Rotate the world by the inverse camera orientation, then translate.
    glRotatef(-pitch * 180.0f / kPi, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw * 180.0f / kPi, 0.0f, 1.0f, 0.0f);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

void Renderer::drawWorld(const World& world) {
    const float half = world.halfExtent;
    const float roadHalf = world.roadHalfWidth;
    const float sidewalk = world.sidewalkWidth;

    // Ground, then roads, then sidewalks: each layer sits slightly higher so
    // there is no z-fighting between them.
    groundPlane(kPalette.ground, half + 12.0f);

    for (float road : world.roadLines) {
        // Road running along z.
        box(road, 0.01f, 0.0f, roadHalf * 2.0f, 0.04f, half * 2.0f, kPalette.asphalt);
        // Road running along x.
        box(0.0f, 0.01f, road, half * 2.0f, 0.04f, roadHalf * 2.0f, kPalette.asphalt);

        // Dashed centre lines.
        for (float t = -half + 4.0f; t < half - 4.0f; t += 8.0f) {
            box(road, 0.05f, t, 0.16f, 0.02f, 3.4f, kPalette.roadLine);
            box(t, 0.05f, road, 3.4f, 0.02f, 0.16f, kPalette.roadLine);
        }

        // Sidewalks on both sides of both directions.
        box(road + roadHalf + sidewalk * 0.5f, 0.06f, 0.0f, sidewalk, 0.12f, half * 2.0f,
            kPalette.sidewalk);
        box(road - roadHalf - sidewalk * 0.5f, 0.06f, 0.0f, sidewalk, 0.12f, half * 2.0f,
            kPalette.sidewalk);
        box(0.0f, 0.06f, road + roadHalf + sidewalk * 0.5f, half * 2.0f, 0.12f, sidewalk,
            kPalette.sidewalk);
        box(0.0f, 0.06f, road - roadHalf - sidewalk * 0.5f, half * 2.0f, 0.12f, sidewalk,
            kPalette.sidewalk);
    }

    // Blocks: grass for the park, a paved yard for the depot.
    for (const Block& block : world.blocks) {
        if (block.isPark) {
            box(block.rect.centerX(), 0.03f, block.rect.centerZ(), block.rect.width(), 0.05f,
                block.rect.depth(), kPalette.grass);
        } else if (block.isDepot) {
            box(block.rect.centerX(), 0.04f, block.rect.centerZ(), block.rect.width(), 0.07f,
                block.rect.depth(), kPalette.depotPad);
        } else {
            box(block.rect.centerX(), 0.03f, block.rect.centerZ(), block.rect.width(), 0.05f,
                block.rect.depth(), kPalette.grass);
        }
    }

    // Buildings with windows and roofs.
    for (const Building& b : world.buildings) {
        const float w = b.footprint.width();
        const float d = b.footprint.depth();
        const float cx = b.footprint.centerX();
        const float cz = b.footprint.centerZ();

        box(cx, 0.2f, cz, w, b.height, d, b.color);

        if (b.hasRoofDetail) {
            box(cx, 0.2f + b.height, cz, w + 0.3f, 0.28f, d + 0.3f, kPalette.roof);
            // A small rooftop unit breaks up the flat silhouette.
            box(cx, 0.2f + b.height + 0.28f, cz, std::max(0.6f, w * 0.25f), 0.5f,
                std::max(0.6f, d * 0.25f), kPalette.roof);
        } else {
            box(cx, 0.2f + b.height, cz, w + 0.2f, 0.16f, d + 0.2f, kPalette.roof);
        }

        // Windows on the four faces. Lit windows are chosen by a stable hash so
        // the pattern is identical for a given building every frame.
        const float windowStart = 1.6f;
        const float rowStep = 2.4f;
        const float colStep = 2.0f;
        const float usableHeight = b.height - windowStart - 0.6f;
        const int rows = std::max(1, static_cast<int>(usableHeight / rowStep));
        const int colsX = std::max(1, static_cast<int>((w - 1.2f) / colStep));
        const int colsZ = std::max(1, static_cast<int>((d - 1.2f) / colStep));

        const int hash = static_cast<int>(b.footprint.x0 * 7.0f) * 31 +
                         static_cast<int>(b.footprint.z0 * 7.0f);

        auto windowColor = [&](int index) {
            const bool lit = ((hash + index * 17) % 5) == 0;
            return lit ? kPalette.windowLit : kPalette.window;
        };

        for (int r = 0; r < rows; ++r) {
            const float y = windowStart + static_cast<float>(r) * rowStep;
            if (y + 1.0f > b.height) break;

            for (int c = 0; c < colsX; ++c) {
                const float x = cx - (static_cast<float>(colsX - 1) * 0.5f) * colStep +
                                static_cast<float>(c) * colStep;
                const int index = r * 16 + c;
                // Front (+z) and back (-z) faces.
                box(x, y, cz + d * 0.5f + 0.02f, 1.0f, 1.1f, 0.06f, windowColor(index));
                box(x, y, cz - d * 0.5f - 0.02f, 1.0f, 1.1f, 0.06f, windowColor(index + 7));
            }

            for (int c = 0; c < colsZ; ++c) {
                const float z = cz - (static_cast<float>(colsZ - 1) * 0.5f) * colStep +
                                static_cast<float>(c) * colStep;
                const int index = r * 16 + c + 3;
                // Left (-x) and right (+x) faces.
                box(cx - w * 0.5f - 0.02f, y, z, 0.06f, 1.1f, 1.0f, windowColor(index));
                box(cx + w * 0.5f + 0.02f, y, z, 0.06f, 1.1f, 1.0f, windowColor(index + 5));
            }
        }
    }

    // Props.
    for (const Prop& p : world.props) {
        switch (p.type) {
            case PropType::Tree: {
                const float trunkHeight = 1.4f * p.scale;
                cylinder(p.position.x, 0.05f, p.position.z, 0.14f * p.scale, trunkHeight,
                         kPalette.treeTrunk);
                blob(p.position.x, 0.05f + trunkHeight + 0.75f * p.scale, p.position.z,
                     1.05f * p.scale, 0.85f, kPalette.treeLeaf);
                break;
            }
            case PropType::Bush:
                blob(p.position.x, 0.15f, p.position.z, 0.55f * p.scale, 0.75f, kPalette.bush);
                break;
            case PropType::Lamp: {
                cylinder(p.position.x, 0.05f, p.position.z, 0.09f, 3.4f, kPalette.lampPost);
                box(p.position.x, 3.45f, p.position.z, 0.42f, 0.22f, 0.42f, kPalette.lampGlow);
                break;
            }
            case PropType::Bench:
                box(p.position.x, 0.35f, p.position.z, 1.5f, 0.14f, 0.5f, kPalette.bench);
                box(p.position.x - 0.6f, 0.05f, p.position.z, 0.16f, 0.3f, 0.4f, kPalette.bench);
                box(p.position.x + 0.6f, 0.05f, p.position.z, 0.16f, 0.3f, 0.4f, kPalette.bench);
                break;
            case PropType::Crate:
                box(p.position.x, 0.05f, p.position.z, 0.7f * p.scale, 0.7f * p.scale,
                    0.7f * p.scale, kPalette.crate);
                break;
        }
    }
}

void Renderer::drawAgents(const std::vector<Agent>& agents, int selectedAgent,
                          float timeSeconds) {
    for (const Agent& a : agents) {
        const float x = a.position.x;
        const float z = a.position.z;

        // A soft ring marks the selected agent so it is findable in the town.
        if (a.id == selectedAgent) {
            constexpr int kSegments = 28;
            const float pulse = 0.55f + 0.12f * std::sin(timeSeconds * 4.0f);
            glBegin(GL_LINE_LOOP);
            setColor({1.0f, 0.95f, 0.55f}, 1.0f);
            for (int i = 0; i < kSegments; ++i) {
                const float t = (2.0f * kPi * static_cast<float>(i)) / static_cast<float>(kSegments);
                glVertex3f(x + std::cos(t) * 0.85f, 0.12f, z + std::sin(t) * 0.85f);
            }
            glEnd();
            // A small marker above the head.
            box(x, 2.15f, z, 0.16f, 0.5f + pulse * 0.3f, 0.16f, {1.0f, 0.92f, 0.45f});
        }

        // A short pole plus a body box reads as a person at diorama scale.
        const float bob = 0.04f * std::sin(timeSeconds * 6.0f + static_cast<float>(a.id));
        box(x, 0.06f, z, 0.34f, 0.9f, 0.28f, a.color);                       // legs/torso
        box(x, 0.96f + bob, z, 0.30f, 0.34f, 0.30f, kPalette.agentSkin);     // head

        // A facing wedge makes heading readable at a glance.
        const float fx = std::sin(a.heading);
        const float fz = std::cos(a.heading);
        box(x + fx * 0.3f, 0.35f, z + fz * 0.3f, 0.26f, 0.36f, 0.26f, a.color);

        // A carried package is a visible fact, not just panel text.
        if (a.carryingPackage) {
            box(x, 1.34f + bob, z, 0.34f, 0.34f, 0.34f, kPalette.package);
        }

        // A faint ground shadow anchors the agent to the ground.
        box(x, 0.02f, z, 0.6f, 0.02f, 0.6f, {0.20f, 0.24f, 0.20f});
    }
}

void Renderer::endFrame() { glFlush(); }

std::vector<unsigned char> Renderer::readPixels(int& outWidth, int& outHeight) const {
    outWidth = width_;
    outHeight = height_;
    std::vector<unsigned char> pixels(static_cast<size_t>(width_) *
                                      static_cast<size_t>(height_) * 3u);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

}  // namespace tt
