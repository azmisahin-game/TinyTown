#pragma once

// Localization for user-visible text: Turkish (default) and English.
//
// This is deliberately a small table plus two pure helpers, not a framework.
// It has no graphics dependency, so the headless test binary can check that
// every key exists in both languages and that Turkish text decodes correctly.
//
// Simulation state names (AgentState, TaskKind) are translated here rather than
// in the simulation, so the core stays language neutral.

#include <cstdint>
#include <string>
#include <vector>

#include "core/simulation.hpp"

namespace tt::localization {

enum class Language {
    Turkish,   // default
    English,
};

constexpr int kLanguageCount = 2;

// A stable key for every user-visible string. Adding a key without adding both
// translations fails the tests.
enum class Key {
    // Panel headers and labels
    Agents,
    AgentsHint,
    Agent,
    Task,
    State,
    Target,
    At,
    Goal,
    Speed,
    Carry,
    Package,
    Nothing,
    Done,
    Failed,
    Result,
    None,

    // Status line
    Seed,
    Time,
    Paused,
    Running,

    // Task kinds
    Patrol,
    Deliver,
    Survey,

    // Agent states
    Idle,
    Navigating,
    Working,
    Completed,
    FailedState,

    // Help overlay
    HelpTitle,
    HelpLanguage,
    HelpMove,
    HelpMoveKeys,
    HelpSprint,
    HelpSprintKey,
    HelpLook,
    HelpLookKeys,
    HelpMouseCapture,
    HelpMouseCaptureKey,
    HelpViewMode,
    HelpViewModeKey,
    HelpSelect,
    HelpSelectKeys,
    HelpPause,
    HelpPauseKeys,
    HelpReset,
    HelpResetKey,
    HelpResetAgent,
    HelpResetAgentKey,
    HelpScreenshot,
    HelpScreenshotKey,
    HelpHelp,
    HelpHelpKeys,
    HelpQuit,
    HelpQuitKey,

    // Honest scope note shown in the overlay
    ScopeNote,

    // Startup console messages
    ConsoleBanner,
    ConsoleHint,
    WorldReset,
    WroteScreenshot,

    // Errors
    ErrorUnknownOption,
    ErrorOptionNeedsValue,
    ErrorSdlInit,
    ErrorSdlWindow,
    ErrorGlContext,
    ErrorGlContextHint,
    ErrorRendererInit,
    ErrorScreenshot,

    // Quick start (used by --help and the packaged text file)
    QuickStartTitle,
    UsageLine,
    OptionsHeading,
    ControlsHeading,
    OptionSeed,
    OptionAgents,
    OptionWidth,
    OptionHeight,
    OptionScreenshot,
    OptionScreenshotSeconds,
    OptionYaw,
    OptionPitch,
    OptionCamera,
    OptionEye,
    OptionLang,
    OptionHelp,
    ControlMove,
    ControlSprint,
    ControlMouse,
    ControlMouseCapture,
    ControlView,
    ControlSelect,
    ControlClick,
    ControlPause,
    ControlReset,
    ControlResetAgent,
    ControlLanguage,
    ControlHelp,
    ControlScreenshot,
    ControlEsc,

    Count,
};

const char* languageCode(Language language);
const char* languageName(Language language);
Language languageFromCode(const std::string& code, Language fallback);

// Parse a language code ("tr" or "en", any case). Returns false and leaves
// `out` untouched when the code is not recognised, so callers can report an
// error instead of silently falling back.
bool parseLanguage(const std::string& code, Language& out);

// The translated text for a key. Falls back to English, then to the key name,
// so a missing entry is visible instead of silently blank.
const char* text(Language language, Key key);

// Translate a simulation enum into the active language.
const char* taskName(Language language, TaskKind kind);
const char* stateName(Language language, AgentState state);

// Format a number with the active language's decimal separator. Turkish uses a
// comma, English a dot.
std::string formatNumber(Language language, float value, int decimals);

// ---------------------------------------------------------------------------
// UTF-8 helpers. The font draws codepoints, not bytes, so text must be decoded.
// ---------------------------------------------------------------------------

// Decode the UTF-8 sequence starting at `index`. Returns the codepoint and
// advances `index` past the sequence. Invalid input yields U+FFFD and advances
// by one byte, so decoding can never loop forever.
uint32_t decodeUtf8(const std::string& text, size_t& index);

// Decode the whole string into codepoints.
std::vector<uint32_t> decodeUtf8All(const std::string& text);

// Encode a single codepoint as UTF-8. Used by tests and by the source files.
std::string encodeUtf8(uint32_t codepoint);

// ---------------------------------------------------------------------------
// Case folding. Turkish is not the same as English here:
//   Turkish: i -> dotted capital I, dotless i -> I
//   English: i -> I, I -> I
// ---------------------------------------------------------------------------

// Fold one codepoint to the uppercase form the font draws.
uint32_t foldToUpper(Language language, uint32_t codepoint);

// Fold a whole UTF-8 string for display in the uppercase-only bitmap font.
std::string foldForDisplay(Language language, const std::string& text);

// Number of display cells a UTF-8 string occupies in the font. One cell per
// codepoint, so this is codepoint count rather than byte length.
size_t displayWidth(Language language, const std::string& text);

// Codepoints the font cannot draw. Empty means the text renders completely.
std::vector<uint32_t> missingGlyphs(Language language, const std::string& text);

// ---------------------------------------------------------------------------
// The language menu, for the in-app toggle and for documentation.
// ---------------------------------------------------------------------------

struct LanguageEntry {
    Language language;
    const char* code;
    const char* name;
};

const std::vector<LanguageEntry>& languages();

}  // namespace tt::localization
