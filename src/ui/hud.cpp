#include "ui/hud.hpp"

#include <SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "core/camera.hpp"

namespace tt::ui {
namespace {

constexpr float kGlyphAdvance = static_cast<float>(font::kCellWidth);
constexpr float kLineAdvance = static_cast<float>(font::kCellHeight);

void glColor4(float r, float g, float b, float a) { glColor4f(r, g, b, a); }

// Formats a float with a fixed number of decimals. snprintf is used instead of
// std::to_string to keep the HUD output short and stable.
std::string formatFloat(float value, int decimals) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, static_cast<double>(value));
    return std::string(buffer);
}

}  // namespace

std::vector<std::string> wrapText(const std::string& text, size_t maxChars) {
    std::vector<std::string> lines;
    if (maxChars == 0) return lines;

    std::string current;
    size_t index = 0;
    while (index < text.size()) {
        // Read one word plus the whitespace that precedes it.
        size_t wordEnd = text.find(' ', index);
        if (wordEnd == std::string::npos) wordEnd = text.size();
        const std::string word = text.substr(index, wordEnd - index);
        index = wordEnd + 1;

        if (word.empty()) continue;

        if (current.empty()) {
            current = word;
        } else if (current.size() + 1 + word.size() <= maxChars) {
            current += " ";
            current += word;
        } else {
            lines.push_back(current);
            current = word;
        }
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

float Hud::lineHeight(float scale) { return kLineAdvance * scale; }

float Hud::textWidth(const std::string& text, float scale) {
    return static_cast<float>(text.size()) * kGlyphAdvance * scale;
}

void Hud::begin(int width, int height) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Top-left origin with y growing downward, which matches how the panel
    // layout is written.
    glOrtho(0.0, static_cast<double>(width_), static_cast<double>(height_), 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Hud::end() {
    glDisable(GL_BLEND);
    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Hud::fillRect(float x, float y, float w, float h, float r, float g, float b, float a) {
    glColor4(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Hud::strokeRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor4(r, g, b, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Hud::drawTextColor(float x, float y, const std::string& text, float r, float g, float b,
                        float scale) {
    glColor4(r, g, b, 1.0f);
    glBegin(GL_QUADS);
    float penX = x;
    for (char raw : text) {
        if (raw == '\n') {
            penX = x;
            y += kLineAdvance * scale;
            continue;
        }
        const char c = font::normalize(raw);
        for (int row = 0; row < font::kGlyphHeight; ++row) {
            for (int col = 0; col < font::kGlyphWidth; ++col) {
                if (!font::glyphPixel(c, row, col)) continue;
                const float px = penX + static_cast<float>(col) * scale;
                const float py = y + static_cast<float>(row) * scale;
                glVertex2f(px, py);
                glVertex2f(px + scale, py);
                glVertex2f(px + scale, py + scale);
                glVertex2f(px, py + scale);
            }
        }
        penX += kGlyphAdvance * scale;
    }
    glEnd();
}

void Hud::drawText(float x, float y, const std::string& text, float scale) {
    drawTextColor(x, y, text, theme_.textR, theme_.textG, theme_.textB, scale);
}

void Hud::drawHelp(const std::vector<std::string>& lines, float x, float y, float scale) {
    float maxWidth = 0.0f;
    for (const std::string& line : lines) {
        maxWidth = std::max(maxWidth, textWidth(line, scale));
    }

    const float padding = 8.0f;
    const float panelWidth = maxWidth + padding * 2.0f;
    const float panelHeight = static_cast<float>(lines.size()) * lineHeight(scale) + padding * 2.0f;

    fillRect(x, y, panelWidth, panelHeight, theme_.panelR, theme_.panelG, theme_.panelB,
             theme_.panelAlpha);
    strokeRect(x, y, panelWidth, panelHeight, theme_.accentR, theme_.accentG, theme_.accentB);

    float penY = y + padding;
    for (const std::string& line : lines) {
        drawText(x + padding, penY, line, scale);
        penY += lineHeight(scale);
    }
}

void Hud::drawStatusLine(const Simulation& sim, bool paused, float x, float y, float scale) {
    const std::string status =
        std::string("SEED ") + std::to_string(sim.config().seed) +
        "   TIME " + formatFloat(sim.elapsed(), 1) + "S" +
        "   DONE " + std::to_string(sim.completedTaskCount()) +
        "   FAIL " + std::to_string(sim.failedTaskCount()) +
        (paused ? "   [PAUSED]" : "   [RUNNING]");

    const float padding = 8.0f;
    const float panelWidth = textWidth(status, scale) + padding * 2.0f;
    const float panelHeight = lineHeight(scale) + padding * 1.4f;

    fillRect(x, y, panelWidth, panelHeight, theme_.panelR, theme_.panelG, theme_.panelB,
             theme_.panelAlpha);
    strokeRect(x, y, panelWidth, panelHeight, theme_.accentR, theme_.accentG, theme_.accentB);
    drawText(x + padding, y + padding * 0.7f, status, scale);
}

void Hud::drawAgentList(const Simulation& sim, int selectedAgent, float x, float y, float scale) {
    const std::vector<Agent>& agents = sim.agents();

    std::vector<std::string> lines;
    lines.push_back("AGENTS  (TAB / CLICK TO SELECT)");
    for (const Agent& a : agents) {
        const std::string marker = (a.id == selectedAgent) ? ">" : " ";
        lines.push_back(marker + " " + std::to_string(a.id) + " " + a.name + "  " +
                        toString(a.task) + "  " + toString(a.state));
    }

    float maxWidth = 0.0f;
    for (const std::string& line : lines) {
        maxWidth = std::max(maxWidth, textWidth(line, scale));
    }

    const float padding = 8.0f;
    const float panelWidth = maxWidth + padding * 2.0f;
    const float panelHeight = static_cast<float>(lines.size()) * lineHeight(scale) + padding * 2.0f;

    fillRect(x, y, panelWidth, panelHeight, theme_.panelR, theme_.panelG, theme_.panelB,
             theme_.panelAlpha);
    strokeRect(x, y, panelWidth, panelHeight, theme_.accentR, theme_.accentG, theme_.accentB);

    float penY = y + padding;
    for (size_t i = 0; i < lines.size(); ++i) {
        const bool isHeader = (i == 0);
        const bool isSelected = (i > 0 && static_cast<int>(i) - 1 == selectedAgent);
        if (isHeader) {
            drawTextColor(x + padding, penY, lines[i], theme_.accentR, theme_.accentG,
                          theme_.accentB, scale);
        } else if (isSelected) {
            drawTextColor(x + padding, penY, lines[i], 1.0f, 1.0f, 0.7f, scale);
        } else {
            drawText(x + padding, penY, lines[i], scale);
        }
        penY += lineHeight(scale);
    }
}

void Hud::drawInspectPanel(const Simulation& sim, int selectedAgent, float x, float y,
                           float width, float scale) {
    if (selectedAgent < 0 || selectedAgent >= static_cast<int>(sim.agents().size())) {
        return;
    }

    const Agent& a = sim.agents()[static_cast<size_t>(selectedAgent)];

    std::vector<std::string> lines;
    lines.push_back("AGENT " + std::to_string(a.id) + "  " + a.name);
    lines.push_back("");
    lines.push_back("TASK    " + std::string(toString(a.task)));
    lines.push_back("STATE   " + std::string(toString(a.state)));
    lines.push_back("TARGET  " + (a.targetName.empty() ? std::string("(none)") : a.targetName));
    lines.push_back("AT      " + formatFloat(a.position.x, 1) + ", " + formatFloat(a.position.z, 1));
    lines.push_back("GOAL    " + formatFloat(a.target.x, 1) + ", " + formatFloat(a.target.z, 1));
    lines.push_back("SPEED   " + formatFloat(a.speed, 1));
    lines.push_back("CARRY   " + std::string(a.carryingPackage ? "PACKAGE" : "NOTHING"));
    lines.push_back("");
    lines.push_back("DONE    " + std::to_string(a.completedTasks));
    lines.push_back("FAILED  " + std::to_string(a.failedTasks));

    // Wrap the truthful result text so long failure notes remain readable.
    if (!a.lastResult.empty()) {
        lines.push_back("");
        const size_t maxChars = static_cast<size_t>(std::max(8.0f, width / (kGlyphAdvance * scale)));
        for (const std::string& part : wrapText("RESULT " + a.lastResult, maxChars)) {
            lines.push_back(part);
        }
    }

    const float padding = 8.0f;
    const float panelHeight = static_cast<float>(lines.size()) * lineHeight(scale) + padding * 2.0f;

    fillRect(x, y, width, panelHeight, theme_.panelR, theme_.panelG, theme_.panelB,
             theme_.panelAlpha);
    strokeRect(x, y, width, panelHeight, theme_.accentR, theme_.accentG, theme_.accentB);

    float penY = y + padding;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0) {
            drawTextColor(x + padding, penY, lines[i], theme_.accentR, theme_.accentG,
                          theme_.accentB, scale);
        } else {
            drawText(x + padding, penY, lines[i], scale);
        }
        penY += lineHeight(scale);
    }
}

}  // namespace tt::ui
