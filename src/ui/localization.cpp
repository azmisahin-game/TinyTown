#include "ui/localization.hpp"

#include <cmath>
#include <cstdio>
#include <iterator>

#include "ui/font5x9.hpp"

namespace tt::localization {
namespace {

// One row per key, one column per language. Keeping both translations on the
// same line makes a missing translation obvious during review.
struct Entry {
    Key key;
    const char* turkish;
    const char* english;
};

constexpr Entry kEntries[] = {
    // Panel headers and labels
    {Key::Agents, "Ajanlar", "Agents"},
    {Key::AgentsHint, "(seçmek için Tab / tıkla)", "(Tab / click to select)"},
    {Key::Agent, "Ajan", "Agent"},
    {Key::Task, "Görev", "Task"},
    {Key::State, "Durum", "State"},
    {Key::Target, "Hedef", "Target"},
    {Key::At, "Konum", "At"},
    {Key::Goal, "Hedef nokta", "Goal"},
    {Key::Speed, "Hız", "Speed"},
    {Key::Carry, "Taşıyor", "Carry"},
    {Key::Package, "Paket", "Package"},
    {Key::Nothing, "Yok", "Nothing"},
    {Key::Done, "Tamamlanan", "Done"},
    {Key::Failed, "Başarısız", "Failed"},
    {Key::Result, "Sonuç", "Result"},
    {Key::None, "(yok)", "(none)"},

    // Status line
    {Key::Seed, "Tohum", "Seed"},
    {Key::Time, "Süre", "Time"},
    {Key::Paused, "Duraklatıldı", "Paused"},
    {Key::Running, "Çalışıyor", "Running"},

    // Task kinds
    {Key::Patrol, "Devriye", "Patrol"},
    {Key::Deliver, "Teslimat", "Deliver"},
    {Key::Survey, "İnceleme", "Survey"},

    // Agent states
    {Key::Idle, "Bekliyor", "Idle"},
    {Key::Navigating, "Hedefe gidiyor", "Navigating"},
    {Key::Working, "Görevde", "Working"},
    {Key::Completed, "Tamamlandı", "Completed"},
    {Key::FailedState, "Başarısız", "Failed"},

    // Help overlay
    {Key::HelpTitle, "TinyTown v0.1 — Kontroller", "TinyTown v0.1 — Controls"},
    {Key::HelpLanguage, "Dil", "Language"},
    {Key::HelpMove, "Hareket", "Move"},
    {Key::HelpMoveKeys, "W A S D / ok tuşları", "W A S D / arrow keys"},
    {Key::HelpSprint, "Hızlı koş", "Sprint"},
    {Key::HelpSprintKey, "Shift", "Shift"},
    {Key::HelpLook, "Bakış", "Look"},
    {Key::HelpLookKeys, "Fare (M ile imleci al)", "Mouse (press M to capture)"},
    {Key::HelpMouseCapture, "İmleci al / bırak", "Capture / release mouse"},
    {Key::HelpMouseCaptureKey, "M", "M"},
    {Key::HelpViewMode, "Genel / sokak görünümü", "Overview / street view"},
    {Key::HelpViewModeKey, "V", "V"},
    {Key::HelpSelect, "Ajan seç", "Select agent"},
    {Key::HelpSelectKeys, "Tab / 1-9 / tıkla", "Tab / 1-9 / click"},
    {Key::HelpPause, "Duraklat / devam", "Pause / resume"},
    {Key::HelpPauseKeys, "P veya boşluk", "P or Space"},
    {Key::HelpReset, "Dünyayı tohuma sıfırla", "Reset world to seed"},
    {Key::HelpResetKey, "R", "R"},
    {Key::HelpResetAgent, "Seçili ajanı sıfırla", "Reset selected agent"},
    {Key::HelpResetAgentKey, "N", "N"},
    {Key::HelpScreenshot, "Ekran görüntüsü kaydet", "Save screenshot"},
    {Key::HelpScreenshotKey, "F2", "F2"},
    {Key::HelpHelp, "Bu yardımı aç / kapa", "Toggle this help"},
    {Key::HelpHelpKeys, "H / F1", "H / F1"},
    {Key::HelpQuit, "İmleci bırak, sonra çık", "Release mouse, then quit"},
    {Key::HelpQuitKey, "Esc", "Esc"},

    {Key::ScopeNote,
     "Ajanlar kural tabanlı görev durum makineleridir. Bu sürümde LLM veya öğrenen yapay zekâ yoktur.",
     "Agents are rule-based task state machines. This version uses no LLM or learning AI."},

    // Console
    {Key::ConsoleBanner, "TinyTown v0.1 — tohum %llu, %d ajan",
     "TinyTown v0.1 — seed %llu, %d agents"},
    {Key::ConsoleHint,
     "Kontroller için H. Sıfırlamak için R. Dil değiştirmek için L. Çıkmak için Esc.",
     "Press H for controls. Press R to reset. Press L to switch language. Esc quits."},
    {Key::WorldReset, "Dünya %llu tohumuna sıfırlandı", "World reset to seed %llu"},
    {Key::WroteScreenshot, "%s yazıldı (%dx%d)", "Wrote %s (%dx%d)"},

    // Errors (shown to the user; internal logs may stay technical English)
    {Key::ErrorUnknownOption, "hata: bilinmeyen seçenek '%s'", "error: unknown option '%s'"},
    {Key::ErrorOptionNeedsValue, "hata: %s bir değer bekliyor", "error: %s needs a value"},
    {Key::ErrorSdlInit, "hata: SDL başlatılamadı: %s", "error: SDL_Init failed: %s"},
    {Key::ErrorSdlWindow, "hata: pencere oluşturulamadı: %s",
     "error: SDL_CreateWindow failed: %s"},
    {Key::ErrorGlContext, "hata: OpenGL bağlamı oluşturulamadı: %s",
     "error: could not create an OpenGL context: %s"},
    {Key::ErrorGlContextHint,
     "       bir grafik sürücüsü (veya Mesa gibi yazılım GL) gerekiyor",
     "       a graphics driver (or a software GL like Mesa) is required"},
    {Key::ErrorRendererInit, "hata: görüntüleyici başlatılamadı: %s",
     "error: renderer initialisation failed: %s"},
    {Key::ErrorScreenshot, "hata: ekran görüntüsü alınamadı: %s",
     "error: screenshot failed: %s"},

    // Quick start
    {Key::QuickStartTitle, "TinyTown v0.1 — küçük prosedürel bir dünya ve basit ajanlar",
     "TinyTown v0.1 — a tiny procedural town with simple agents"},
    {Key::UsageLine, "Kullanım: %s [seçenekler]", "Usage: %s [options]"},
    {Key::OptionsHeading, "Seçenekler:", "Options:"},
    {Key::ControlsHeading, "Kontroller (etkileşimli mod):", "Controls (interactive mode):"},
    {Key::OptionSeed, "  --seed <n>            dünya tohumu (varsayılan 1337)",
     "  --seed <n>            world seed (default 1337)"},
    {Key::OptionAgents, "  --agents <n>          ajan sayısı (varsayılan 3)",
     "  --agents <n>          number of agents (default 3)"},
    {Key::OptionWidth, "  --width <px>          pencere genişliği (varsayılan 1280)",
     "  --width <px>          window width (default 1280)"},
    {Key::OptionHeight, "  --height <px>         pencere yüksekliği (varsayılan 720)",
     "  --height <px>         window height (default 720)"},
    {Key::OptionScreenshot,
     "  --screenshot <yol>    --screenshot-seconds kadar çalışıp görüntü\n"
     "                        yazar ve çıkar (etkileşimsiz)",
     "  --screenshot <path>   render for --screenshot-seconds then write an\n"
     "                        image and exit (no interaction)"},
    {Key::OptionScreenshotSeconds,
     "  --screenshot-seconds <s>  yakalamadan önce geçen simülasyon saniyesi (3)",
     "  --screenshot-seconds <s>  simulated seconds before the capture (default 3)"},
    {Key::OptionYaw, "  --yaw <radyan>        başlangıç kamera yönü",
     "  --yaw <radians>       initial camera yaw"},
    {Key::OptionPitch, "  --pitch <radyan>      başlangıç kamera eğimi",
     "  --pitch <radians>     initial camera pitch"},
    {Key::OptionCamera, "  --camera <x> <z>      başlangıç kamera konumu",
     "  --camera <x> <z>      initial camera ground position"},
    {Key::OptionEye,
     "  --eye <metre>         başlangıç göz yüksekliği (büyük değer genel\n"
     "                        görünümle başlar)",
     "  --eye <metres>        initial eye height (a high value starts in\n"
     "                        overview mode looking down at the town)"},
    {Key::OptionLang, "  --lang <tr|en>        arayüz dili (varsayılan tr)",
     "  --lang <tr|en>        interface language (default tr)"},
    {Key::OptionHelp, "  --help                bu mesajı göster",
     "  --help                show this message"},
    {Key::ControlMove, "  W A S D / ok tuşları  hareket", "  W A S D / arrows      move"},
    {Key::ControlSprint, "  Shift                 hızlı koş", "  Shift                 sprint"},
    {Key::ControlMouse, "  Fare                  bakış (önce M ile imleci al)",
     "  Mouse                 look (after pressing M to capture the cursor)"},
    {Key::ControlMouseCapture, "  M                     imleci al / bırak",
     "  M                     toggle mouse capture"},
    {Key::ControlView, "  V                     genel / sokak görünümü",
     "  V                     toggle overview / street camera"},
    {Key::ControlSelect, "  Tab / 1-9             ajan seç",
     "  Tab / 1-9             select agent"},
    {Key::ControlClick, "  Sol tık               imlecin altındaki ajanı seç",
     "  Left click            select the agent under the cursor"},
    {Key::ControlPause, "  P veya Boşluk         duraklat / devam",
     "  P or Space            pause / resume the simulation"},
    {Key::ControlReset, "  R                     dünyayı tohuma sıfırla",
     "  R                     reset the whole simulation to the seed"},
    {Key::ControlResetAgent, "  N                     yalnızca seçili ajanı sıfırla",
     "  N                     reset the selected agent only"},
    {Key::ControlLanguage, "  L                     arayüz dilini değiştir (TR / EN)",
     "  L                     switch interface language (TR / EN)"},
    {Key::ControlHelp, "  H veya F1             bu yardım katmanını aç / kapa",
     "  H or F1               toggle this help overlay"},
    {Key::ControlScreenshot, "  F2                    captures/ içine ekran görüntüsü kaydet",
     "  F2                    save a screenshot into captures/"},
    {Key::ControlEsc, "  Esc                   imleci bırak, bırakınca çık",
     "  Esc                   release the mouse, or quit when it is released"},
};

constexpr int kEntryCount = static_cast<int>(std::size(kEntries));

const Entry* findEntry(Key key) {
    for (const Entry& entry : kEntries) {
        if (entry.key == key) return &entry;
    }
    return nullptr;
}

const char* pick(Language language, const Entry& entry) {
    return language == Language::Turkish ? entry.turkish : entry.english;
}

// Turkish uppercase folding. The dotted/dotless i distinction is the only place
// Turkish differs from English for the letters this font draws.
uint32_t foldTurkish(uint32_t codepoint) {
    switch (codepoint) {
        case 'i': return 0x0130;   // i -> dotted capital I
        case 0x0131: return 'I';   // dotless i -> I
        case 0x0130: return 0x0130;
        case 'I': return 'I';
        // Lowercase Turkish letters fold to their uppercase forms.
        case 0x00E7: return 0x00C7;
        case 0x011F: return 0x011E;
        case 0x00F6: return 0x00D6;
        case 0x015F: return 0x015E;
        case 0x00FC: return 0x00DC;
        case 0x00E9: return 0x00C9;
        default: break;
    }
    if (codepoint >= 'a' && codepoint <= 'z') {
        return static_cast<uint32_t>(codepoint - 'a' + 'A');
    }
    return codepoint;
}

uint32_t foldEnglish(uint32_t codepoint) {
    if (codepoint >= 'a' && codepoint <= 'z') {
        return static_cast<uint32_t>(codepoint - 'a' + 'A');
    }
    switch (codepoint) {
        // Turkish lowercase still folds so mixed input renders.
        case 0x00E7: return 0x00C7;
        case 0x011F: return 0x011E;
        case 0x00F6: return 0x00D6;
        case 0x015F: return 0x015E;
        case 0x00FC: return 0x00DC;
        case 0x00E9: return 0x00C9;
        // In English, dotless i has no distinct uppercase; use plain I.
        case 0x0131: return 'I';
        default: return codepoint;
    }
}

}  // namespace

const char* languageCode(Language language) {
    return language == Language::Turkish ? "tr" : "en";
}

const char* languageName(Language language) {
    return language == Language::Turkish ? "TURKCE" : "ENGLISH";
}

Language languageFromCode(const std::string& code, Language fallback) {
    Language parsed = fallback;
    return parseLanguage(code, parsed) ? parsed : fallback;
}

bool parseLanguage(const std::string& code, Language& out) {
    if (code.size() != 2) return false;
    char a = code[0];
    char b = code[1];
    if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z') b = static_cast<char>(b - 'A' + 'a');
    if (a != 't' && a != 'e') return false;
    if (a == 't' && b == 'r') {
        out = Language::Turkish;
        return true;
    }
    if (a == 'e' && b == 'n') {
        out = Language::English;
        return true;
    }
    return false;
}

const char* text(Language language, Key key) {
    const Entry* entry = findEntry(key);
    if (entry == nullptr) return "?";
    return pick(language, *entry);
}

const char* taskName(Language language, TaskKind kind) {
    switch (kind) {
        case TaskKind::Patrol: return text(language, Key::Patrol);
        case TaskKind::Deliver: return text(language, Key::Deliver);
        case TaskKind::Survey: return text(language, Key::Survey);
    }
    return "?";
}

const char* stateName(Language language, AgentState state) {
    switch (state) {
        case AgentState::Idle: return text(language, Key::Idle);
        case AgentState::Navigating: return text(language, Key::Navigating);
        case AgentState::Working: return text(language, Key::Working);
        case AgentState::Completed: return text(language, Key::Completed);
        case AgentState::Failed: return text(language, Key::FailedState);
    }
    return "?";
}

std::string formatNumber(Language language, float value, int decimals) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, static_cast<double>(value));
    std::string result(buffer);
    if (language == Language::Turkish) {
        // Turkish uses a comma as the decimal separator.
        for (char& c : result) {
            if (c == '.') c = ',';
        }
    }
    return result;
}

uint32_t decodeUtf8(const std::string& text, size_t& index) {
    if (index >= text.size()) return 0;

    const unsigned char first = static_cast<unsigned char>(text[index]);
    auto continuation = [&](size_t offset) -> bool {
        if (index + offset >= text.size()) return false;
        const unsigned char c = static_cast<unsigned char>(text[index + offset]);
        return (c & 0xC0) == 0x80;
    };
    auto payload = [&](size_t offset) -> uint32_t {
        return static_cast<uint32_t>(static_cast<unsigned char>(text[index + offset]) & 0x3F);
    };

    if (first < 0x80) {
        ++index;
        return first;
    }
    if ((first & 0xE0) == 0xC0 && continuation(1)) {
        const uint32_t cp = (static_cast<uint32_t>(first & 0x1F) << 6) | payload(1);
        index += 2;
        return cp;
    }
    if ((first & 0xF0) == 0xE0 && continuation(1) && continuation(2)) {
        const uint32_t cp = (static_cast<uint32_t>(first & 0x0F) << 12) | (payload(1) << 6) |
                            payload(2);
        index += 3;
        return cp;
    }
    if ((first & 0xF8) == 0xF0 && continuation(1) && continuation(2) && continuation(3)) {
        const uint32_t cp = (static_cast<uint32_t>(first & 0x07) << 18) | (payload(1) << 12) |
                            (payload(2) << 6) | payload(3);
        index += 4;
        return cp;
    }

    // Invalid byte: report the replacement character and move one byte on.
    ++index;
    return 0xFFFD;
}

std::vector<uint32_t> decodeUtf8All(const std::string& text) {
    std::vector<uint32_t> codepoints;
    codepoints.reserve(text.size());
    size_t index = 0;
    while (index < text.size()) {
        codepoints.push_back(decodeUtf8(text, index));
    }
    return codepoints;
}

std::string encodeUtf8(uint32_t codepoint) {
    std::string out;
    if (codepoint < 0x80) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    return out;
}

uint32_t foldToUpper(Language language, uint32_t codepoint) {
    return language == Language::Turkish ? foldTurkish(codepoint)
                                         : foldEnglish(codepoint);
}

std::string foldForDisplay(Language language, const std::string& text) {
    std::string out;
    out.reserve(text.size());
    size_t index = 0;
    while (index < text.size()) {
        out += encodeUtf8(foldToUpper(language, decodeUtf8(text, index)));
    }
    return out;
}

size_t displayWidth(Language language, const std::string& text) {
    // One display cell per codepoint. The language does not change the cell
    // count for this font; the parameter is kept so call sites stay uniform and
    // a future proportional font has somewhere to branch.
    (void)language;
    size_t count = 0;
    size_t index = 0;
    while (index < text.size()) {
        decodeUtf8(text, index);
        ++count;
    }
    return count;
}

std::vector<uint32_t> missingGlyphs(Language language, const std::string& text) {
    std::vector<uint32_t> missing;
    size_t index = 0;
    while (index < text.size()) {
        const uint32_t folded = foldToUpper(language, decodeUtf8(text, index));
        // A newline is handled by the layout code as a line break, so it needs
        // no glyph. Everything else must be drawable.
        if (folded == '\n') continue;
        if (font::hasGlyph(folded)) continue;
        // Report each distinct missing codepoint once.
        bool seen = false;
        for (uint32_t existing : missing) {
            if (existing == folded) {
                seen = true;
                break;
            }
        }
        if (!seen) missing.push_back(folded);
    }
    return missing;
}

const std::vector<LanguageEntry>& languages() {
    static const std::vector<LanguageEntry> entries = {
        {Language::Turkish, "tr", "TURKCE"},
        {Language::English, "en", "ENGLISH"},
    };
    return entries;
}

}  // namespace tt::localization
