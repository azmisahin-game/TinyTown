#include "ui/hud.hpp"

#include <SDL_opengl.h>

#include <algorithm>
#include <cmath>

#include "core/camera.hpp"

namespace tt::ui {
namespace {

constexpr float kGlyphAdvance = static_cast<float>(font::kCellWidth);
constexpr float kLineAdvance = static_cast<float>(font::kCellHeight);
constexpr float kPanelPadding = 8.0f;

void glColor4(float r, float g, float b, float a) { glColor4f(r, g, b, a); }

using localization::Key;
using localization::Language;

std::string tr(Language language, Key key) { return localization::text(language, key); }

}  // namespace

std::vector<std::string> wrapText(const std::string& text, size_t maxChars) {
    std::vector<std::string> lines;
    if (maxChars == 0) return lines;

    // Wrap by display cells, not bytes, so Turkish text breaks at the right
    // place. Each codepoint occupies one cell.
    auto cells = [](const std::string& value) {
        size_t count = 0;
        size_t index = 0;
        while (index < value.size()) {
            localization::decodeUtf8(value, index);
            ++count;
        }
        return count;
    };

    std::string current;
    size_t index = 0;
    while (index < text.size()) {
        size_t wordEnd = text.find(' ', index);
        if (wordEnd == std::string::npos) wordEnd = text.size();
        const std::string word = text.substr(index, wordEnd - index);
        index = wordEnd + 1;

        if (word.empty()) continue;

        if (current.empty()) {
            current = word;
        } else if (cells(current) + 1 + cells(word) <= maxChars) {
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

std::vector<std::string> helpLines(localization::Language language) {
    using localization::Key;
    std::vector<std::string> lines;
    lines.push_back(localization::text(language, Key::HelpTitle));
    lines.push_back("");
    lines.push_back("  " + std::string(localization::text(language, Key::HelpMoveKeys)) + "   " +
                    localization::text(language, Key::HelpMove));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpSprintKey)) + "   " +
                    localization::text(language, Key::HelpSprint));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpLookKeys)));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpMouseCaptureKey)) +
                    "   " + localization::text(language, Key::HelpMouseCapture));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpViewModeKey)) +
                    "   " + localization::text(language, Key::HelpViewMode));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpSelectKeys)) + "   " +
                    localization::text(language, Key::HelpSelect));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpPauseKeys)) + "   " +
                    localization::text(language, Key::HelpPause));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpResetKey)) + "   " +
                    localization::text(language, Key::HelpReset));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpResetAgentKey)) +
                    "   " + localization::text(language, Key::HelpResetAgent));
    lines.push_back("  L   " + std::string(localization::text(language, Key::HelpLanguage)));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpScreenshotKey)) +
                    "   " + localization::text(language, Key::HelpScreenshot));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpHelpKeys)) + "   " +
                    localization::text(language, Key::HelpHelp));
    lines.push_back("  " + std::string(localization::text(language, Key::HelpQuitKey)) + "   " +
                    localization::text(language, Key::HelpQuit));
    return lines;
}

float Hud::lineHeight(float scale) { return kLineAdvance * scale; }

float Hud::textWidth(const std::string& text, float scale) const {
    // Width is measured in display cells. A UTF-8 string has fewer codepoints
    // than bytes, so byte length would over-report Turkish text.
    return static_cast<float>(localization::displayWidth(language_, text)) * kGlyphAdvance *
           scale;
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
    size_t index = 0;
    while (index < text.size()) {
        const uint32_t raw = localization::decodeUtf8(text, index);
        if (raw == '\n') {
            penX = x;
            y += kLineAdvance * scale;
            continue;
        }
        // Fold per language, then draw the folded codepoint. The font is
        // uppercase-only and language aware (Turkish i maps to dotted I).
        const uint32_t codepoint = localization::foldToUpper(language_, raw);
        for (int row = 0; row < font::kGlyphHeight; ++row) {
            for (int col = 0; col < font::kGlyphWidth; ++col) {
                if (!font::glyphPixel(codepoint, row, col)) continue;
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

void Hud::drawPanel(float x, float y, float w, float h, const std::vector<std::string>& lines,
                    float scale, size_t accentLine) {
    fillRect(x, y, w, h, theme_.panelR, theme_.panelG, theme_.panelB, theme_.panelAlpha);
    strokeRect(x, y, w, h, theme_.accentR, theme_.accentG, theme_.accentB);

    float penY = y + kPanelPadding;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == accentLine) {
            drawTextColor(x + kPanelPadding, penY, lines[i], theme_.accentR, theme_.accentG,
                          theme_.accentB, scale);
        } else {
            drawText(x + kPanelPadding, penY, lines[i], scale);
        }
        penY += lineHeight(scale);
    }
}

void Hud::drawHelp(float x, float y, float scale) {
    const std::vector<std::string> lines = helpLines(language_);

    float maxWidth = 0.0f;
    for (const std::string& line : lines) {
        maxWidth = std::max(maxWidth, textWidth(line, scale));
    }

    // The honest scope note sits under the controls, wrapped to the panel width.
    std::vector<std::string> all = lines;
    all.push_back("");
    const size_t maxChars = static_cast<size_t>(std::max(20.0f, maxWidth / (kGlyphAdvance * scale)));
    for (const std::string& part : wrapText(localization::text(language_, Key::ScopeNote), maxChars)) {
        all.push_back(part);
        maxWidth = std::max(maxWidth, textWidth(part, scale));
    }

    const float panelWidth = maxWidth + kPanelPadding * 2.0f;
    const float panelHeight =
        static_cast<float>(all.size()) * lineHeight(scale) + kPanelPadding * 2.0f;

    drawPanel(x, y, panelWidth, panelHeight, all, scale, 0);
}

void Hud::drawStatusLine(const Simulation& sim, bool paused, float x, float y, float scale) {
    const std::string status =
        tr(language_, Key::Seed) + " " + std::to_string(sim.config().seed) + "   " +
        tr(language_, Key::Time) + " " + localization::formatNumber(language_, sim.elapsed(), 1) +
        "s   " + tr(language_, Key::Done) + " " + std::to_string(sim.completedTaskCount()) +
        "   " + tr(language_, Key::Failed) + " " + std::to_string(sim.failedTaskCount()) +
        "   [" + tr(language_, paused ? Key::Paused : Key::Running) + "]";

    const float panelWidth = textWidth(status, scale) + kPanelPadding * 2.0f;
    const float panelHeight = lineHeight(scale) + kPanelPadding * 1.4f;

    fillRect(x, y, panelWidth, panelHeight, theme_.panelR, theme_.panelG, theme_.panelB,
             theme_.panelAlpha);
    strokeRect(x, y, panelWidth, panelHeight, theme_.accentR, theme_.accentG, theme_.accentB);
    drawText(x + kPanelPadding, y + kPanelPadding * 0.7f, status, scale);
}

void Hud::drawAgentList(const Simulation& sim, int selectedAgent, float x, float y, float scale) {
    const std::vector<Agent>& agents = sim.agents();

    std::vector<std::string> lines;
    lines.push_back(tr(language_, Key::Agents) + "   " + tr(language_, Key::AgentsHint));
    for (const Agent& a : agents) {
        const std::string marker = (a.id == selectedAgent) ? ">" : " ";
        lines.push_back(marker + " " + std::to_string(a.id) + " " + a.name + "   " +
                        localization::taskName(language_, a.task) + "   " +
                        localization::stateName(language_, a.state));
    }

    float maxWidth = 0.0f;
    for (const std::string& line : lines) {
        maxWidth = std::max(maxWidth, textWidth(line, scale));
    }

    const float panelWidth = maxWidth + kPanelPadding * 2.0f;
    const float panelHeight =
        static_cast<float>(lines.size()) * lineHeight(scale) + kPanelPadding * 2.0f;

    fillRect(x, y, panelWidth, panelHeight, theme_.panelR, theme_.panelG, theme_.panelB,
             theme_.panelAlpha);
    strokeRect(x, y, panelWidth, panelHeight, theme_.accentR, theme_.accentG, theme_.accentB);

    float penY = y + kPanelPadding;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == 0) {
            drawTextColor(x + kPanelPadding, penY, lines[i], theme_.accentR, theme_.accentG,
                          theme_.accentB, scale);
        } else if (static_cast<int>(i) - 1 == selectedAgent) {
            drawTextColor(x + kPanelPadding, penY, lines[i], 1.0f, 1.0f, 0.7f, scale);
        } else {
            drawText(x + kPanelPadding, penY, lines[i], scale);
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

    auto label = [&](Key key) { return tr(language_, key); };

    std::vector<std::string> lines;
    lines.push_back(label(Key::Agent) + " " + std::to_string(a.id) + "   " + a.name);
    lines.push_back("");
    lines.push_back(label(Key::Task) + "   " + localization::taskName(language_, a.task));
    lines.push_back(label(Key::State) + "   " + localization::stateName(language_, a.state));
    lines.push_back(label(Key::Target) + "   " +
                    (a.targetName.empty() ? label(Key::None) : a.targetName));
    lines.push_back(label(Key::At) + "   " + localization::formatNumber(language_, a.position.x, 1) +
                    ", " + localization::formatNumber(language_, a.position.z, 1));
    lines.push_back(label(Key::Goal) + "   " + localization::formatNumber(language_, a.target.x, 1) +
                    ", " + localization::formatNumber(language_, a.target.z, 1));
    lines.push_back(label(Key::Speed) + "   " + localization::formatNumber(language_, a.speed, 1));
    lines.push_back(label(Key::Carry) + "   " +
                    label(a.carryingPackage ? Key::Package : Key::Nothing));
    lines.push_back("");
    lines.push_back(label(Key::Done) + "   " + std::to_string(a.completedTasks));
    lines.push_back(label(Key::Failed) + "   " + std::to_string(a.failedTasks));

    // Wrap the truthful result text so long notes remain readable.
    if (!a.lastResult.empty()) {
        lines.push_back("");
        const size_t maxChars =
            static_cast<size_t>(std::max(8.0f, width / (kGlyphAdvance * scale)));
        for (const std::string& part :
             wrapText(label(Key::Result) + " " + a.lastResult, maxChars)) {
            lines.push_back(part);
        }
    }

    const float panelHeight =
        static_cast<float>(lines.size()) * lineHeight(scale) + kPanelPadding * 2.0f;
    drawPanel(x, y, width, panelHeight, lines, scale, 0);
}

}  // namespace tt::ui
