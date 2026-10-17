#pragma once

// Screen-space overlay: help text, agent list, status line and the selected
// agent inspect panel.
//
// Drawn with an orthographic projection over the 3D scene. All text goes
// through the 5x9 bitmap font, so there is no font file and no text library.
// Text arrives as UTF-8 and is decoded to codepoints before drawing, so
// Turkish characters render correctly.

#include <string>
#include <vector>

#include "core/simulation.hpp"
#include "ui/font5x9.hpp"
#include "ui/localization.hpp"

namespace tt::ui {

struct HudTheme {
    float textR = 0.96f;
    float textG = 0.96f;
    float textB = 0.90f;
    float panelR = 0.06f;
    float panelG = 0.08f;
    float panelB = 0.11f;
    float panelAlpha = 0.72f;
    float accentR = 0.98f;
    float accentG = 0.82f;
    float accentB = 0.36f;
};

class Hud {
public:
    // Begin/end a screen-space pass. Must wrap all draw calls below.
    void begin(int width, int height);
    void end();

    // The active language controls text folding and number formatting.
    void setLanguage(localization::Language language) { language_ = language; }
    localization::Language language() const { return language_; }

    void drawText(float x, float y, const std::string& text, float scale = 1.0f);
    void drawTextColor(float x, float y, const std::string& text, float r, float g, float b,
                       float scale = 1.0f);

    // Height of one line of text at the given scale.
    static float lineHeight(float scale = 1.0f);
    // Width of text at the given scale. Counts codepoints, not bytes.
    float textWidth(const std::string& text, float scale = 1.0f) const;

    // Overlay blocks. Each reads the active language from the Hud.
    void drawHelp(float x, float y, float scale);
    void drawInspectPanel(const Simulation& sim, int selectedAgent, float x, float y,
                          float width, float scale);
    void drawStatusLine(const Simulation& sim, bool paused, float x, float y, float scale);
    void drawAgentList(const Simulation& sim, int selectedAgent, float x, float y, float scale);

private:
    void fillRect(float x, float y, float w, float h, float r, float g, float b, float a);
    void strokeRect(float x, float y, float w, float h, float r, float g, float b);
    void drawPanel(float x, float y, float w, float h, const std::vector<std::string>& lines,
                   float scale, size_t accentLine = static_cast<size_t>(-1));

    int width_ = 1;
    int height_ = 1;
    HudTheme theme_;
    localization::Language language_ = localization::Language::Turkish;
};

// Wrap a sentence into lines that fit `maxChars` display cells per line.
// Counting is by codepoint so Turkish text wraps at the right place.
std::vector<std::string> wrapText(const std::string& text, size_t maxChars);

// The help overlay content, in the given language. Exposed for tests and for
// the packaged quick-start text file.
std::vector<std::string> helpLines(localization::Language language);

}  // namespace tt::ui

