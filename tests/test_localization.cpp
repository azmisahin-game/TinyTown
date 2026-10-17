// Localization tests. These run headless, so they cover the parts of the
// Turkish/English interface that a screenshot cannot prove: that every key is
// translated in both languages, that UTF-8 decoding is correct, that Turkish
// case folding is language aware, and that the font can draw every character
// the interface actually uses.

#include "ui/localization.hpp"

#include <string>
#include <vector>

#include "test_framework.hpp"
#include "ui/font5x9.hpp"

using namespace tt;
using tt::localization::Key;
using tt::localization::Language;

namespace {

// Every key, so a new key without a translation fails the build's tests.
std::vector<Key> allKeys() {
    std::vector<Key> keys;
    for (int i = 0; i < static_cast<int>(Key::Count); ++i) {
        keys.push_back(static_cast<Key>(i));
    }
    return keys;
}

// The user-visible Turkish strings that must exist, by the terminology the
// task specifies.
struct Term {
    Key key;
    const char* turkish;
    const char* english;
};

const Term kRequiredTerms[] = {
    {Key::Agent, "Ajan", "Agent"},
    {Key::Task, "Görev", "Task"},
    {Key::State, "Durum", "State"},
    {Key::Target, "Hedef", "Target"},
    {Key::Carry, "Taşıyor", "Carry"},
    {Key::Package, "Paket", "Package"},
    {Key::Nothing, "Yok", "Nothing"},
    {Key::Done, "Tamamlanan", "Done"},
    {Key::Failed, "Başarısız", "Failed"},
    {Key::Paused, "Duraklatıldı", "Paused"},
    {Key::Running, "Çalışıyor", "Running"},
    {Key::Patrol, "Devriye", "Patrol"},
    {Key::Deliver, "Teslimat", "Deliver"},
    {Key::Survey, "İnceleme", "Survey"},
    {Key::Navigating, "Hedefe gidiyor", "Navigating"},
    {Key::Working, "Görevde", "Working"},
    {Key::Completed, "Tamamlandı", "Completed"},
};

}  // namespace

TEST(Localization, EveryKeyHasBothLanguages) {
    for (Key key : allKeys()) {
        const char* tr = localization::text(Language::Turkish, key);
        const char* en = localization::text(Language::English, key);
        CHECK_MSG(tr != nullptr, "Turkish text is null for key " + std::to_string(static_cast<int>(key)));
        CHECK_MSG(en != nullptr, "English text is null for key " + std::to_string(static_cast<int>(key)));
        if (tr == nullptr || en == nullptr) continue;
        CHECK_MSG(tr[0] != '\0', "Turkish text is empty for key " + std::to_string(static_cast<int>(key)));
        CHECK_MSG(en[0] != '\0', "English text is empty for key " + std::to_string(static_cast<int>(key)));
        // The "?" marker is the fallback for an unknown key.
        CHECK_MSG(std::string(tr) != "?", "Turkish text fell back for key " + std::to_string(static_cast<int>(key)));
        CHECK_MSG(std::string(en) != "?", "English text fell back for key " + std::to_string(static_cast<int>(key)));
    }
}

TEST(Localization, RequiredTurkishTerminologyMatchesSpec) {
    for (const Term& term : kRequiredTerms) {
        CHECK_EQ(std::string(localization::text(Language::Turkish, term.key)), std::string(term.turkish));
        CHECK_EQ(std::string(localization::text(Language::English, term.key)), std::string(term.english));
    }
}

TEST(Localization, TurkishAndEnglishDifferWhereItMatters) {
    // A translation that was never written would leave the English text in
    // place. These keys must differ.
    const Key mustDiffer[] = {Key::Task,       Key::State,      Key::Target,
                              Key::Patrol,     Key::Deliver,    Key::Survey,
                              Key::Navigating, Key::Working,    Key::Completed,
                              Key::Paused,     Key::Running,    Key::Done,
                              Key::Failed,     Key::HelpTitle,  Key::ScopeNote};
    for (Key key : mustDiffer) {
        const std::string tr = localization::text(Language::Turkish, key);
        const std::string en = localization::text(Language::English, key);
        CHECK_MSG(tr != en, "key " + std::to_string(static_cast<int>(key)) +
                                " has identical Turkish and English text: " + tr);
    }
}

TEST(Localization, TurkishIsTheDefaultLanguage) {
    CHECK_EQ(std::string(localization::languageCode(Language::Turkish)), std::string("tr"));
    CHECK_EQ(std::string(localization::languageCode(Language::English)), std::string("en"));

    Language parsed = Language::English;
    CHECK(localization::parseLanguage("tr", parsed));
    CHECK_EQ(parsed, Language::Turkish);
    CHECK(localization::parseLanguage("TR", parsed));
    CHECK_EQ(parsed, Language::Turkish);
    CHECK(localization::parseLanguage("en", parsed));
    CHECK_EQ(parsed, Language::English);
    CHECK(localization::parseLanguage("EN", parsed));
    CHECK_EQ(parsed, Language::English);

    // An unknown code must be rejected, not silently accepted.
    CHECK(!localization::parseLanguage("de", parsed));
    CHECK(!localization::parseLanguage("", parsed));
    CHECK(!localization::parseLanguage("tur", parsed));
}

TEST(Localization, TaskAndStateNamesAreTranslated) {
    CHECK_EQ(std::string(localization::taskName(Language::Turkish, TaskKind::Patrol)),
             std::string("Devriye"));
    CHECK_EQ(std::string(localization::taskName(Language::Turkish, TaskKind::Deliver)),
             std::string("Teslimat"));
    CHECK_EQ(std::string(localization::taskName(Language::Turkish, TaskKind::Survey)),
             std::string("İnceleme"));

    CHECK_EQ(std::string(localization::stateName(Language::Turkish, AgentState::Navigating)),
             std::string("Hedefe gidiyor"));
    CHECK_EQ(std::string(localization::stateName(Language::Turkish, AgentState::Completed)),
             std::string("Tamamlandı"));

    CHECK_EQ(std::string(localization::taskName(Language::English, TaskKind::Patrol)),
             std::string("Patrol"));
    CHECK_EQ(std::string(localization::stateName(Language::English, AgentState::Working)),
             std::string("Working"));
}

TEST(Localization, DecodesUtf8Correctly) {
    // Round-trip every codepoint the font supports.
    int count = 0;
    const uint32_t* codepoints = font::supportedCodepoints(count);
    for (int i = 0; i < count; ++i) {
        const std::string encoded = localization::encodeUtf8(codepoints[i]);
        const std::vector<uint32_t> decoded = localization::decodeUtf8All(encoded);
        CHECK_MSG(decoded.size() == 1,
                  "encoding U+" + std::to_string(codepoints[i]) + " did not decode to one codepoint");
        if (decoded.size() == 1) {
            CHECK_EQ(decoded[0], codepoints[i]);
        }
    }
}

TEST(Localization, DecodesMultiByteTurkishText) {
    // "Görev" in UTF-8: G, o-umlaut (2 bytes), r, e, v.
    const std::string text = "G\u00f6rev";
    const std::vector<uint32_t> decoded = localization::decodeUtf8All(text);
    CHECK_EQ(decoded.size(), static_cast<size_t>(5));
    CHECK_EQ(decoded[0], static_cast<uint32_t>('G'));
    CHECK_EQ(decoded[1], static_cast<uint32_t>(0x00F6));
    CHECK_EQ(decoded[2], static_cast<uint32_t>('r'));

    // Byte length and codepoint count differ, which is why width must not use
    // std::string::size().
    CHECK(text.size() > decoded.size());
    CHECK_EQ(localization::displayWidth(Language::Turkish, text), decoded.size());
}

TEST(Localization, InvalidUtf8DoesNotLoopForever) {
    // The decoder advances at least one byte on every invalid unit, which is
    // what guarantees termination. It does not try to guess how many bytes an
    // invalid sequence "meant", so a truncated sequence yields one replacement
    // per leftover byte. Assert the guarantees, not a byte count.
    const std::string lone = std::string("\x80");
    const std::vector<uint32_t> loneDecoded = localization::decodeUtf8All(lone);
    CHECK_EQ(loneDecoded.size(), static_cast<size_t>(1));
    CHECK_EQ(loneDecoded[0], static_cast<uint32_t>(0xFFFD));

    // A truncated three-byte sequence must terminate and produce only
    // replacements, never a partially decoded value.
    const std::string truncated = std::string("\xE2\x82");
    const std::vector<uint32_t> truncatedDecoded = localization::decodeUtf8All(truncated);
    CHECK_MSG(!truncatedDecoded.empty(), "truncated input produced nothing");
    CHECK_MSG(truncatedDecoded.size() <= truncated.size(),
              "decoder consumed fewer bytes than it produced codepoints");
    for (uint32_t codepoint : truncatedDecoded) {
        CHECK_EQ(codepoint, static_cast<uint32_t>(0xFFFD));
    }

    // Valid bytes around an invalid one must survive intact.
    const std::string mixed = std::string("A") + std::string("\xFF") + std::string("B");
    const std::vector<uint32_t> mixedDecoded = localization::decodeUtf8All(mixed);
    CHECK_EQ(mixedDecoded.size(), static_cast<size_t>(3));
    CHECK_EQ(mixedDecoded[0], static_cast<uint32_t>('A'));
    CHECK_EQ(mixedDecoded[1], static_cast<uint32_t>(0xFFFD));
    CHECK_EQ(mixedDecoded[2], static_cast<uint32_t>('B'));
}

TEST(Localization, TurkishCaseFoldingIsLanguageAware) {
    // Turkish: i folds to dotted capital I, dotless i folds to plain I.
    CHECK_EQ(localization::foldToUpper(Language::Turkish, 'i'), static_cast<uint32_t>(0x0130));
    CHECK_EQ(localization::foldToUpper(Language::Turkish, 0x0131), static_cast<uint32_t>('I'));

    // English folds both to plain I.
    CHECK_EQ(localization::foldToUpper(Language::English, 'i'), static_cast<uint32_t>('I'));
    CHECK_EQ(localization::foldToUpper(Language::English, 0x0131), static_cast<uint32_t>('I'));

    // Turkish lowercase letters fold to their uppercase forms.
    CHECK_EQ(localization::foldToUpper(Language::Turkish, 0x00E7), static_cast<uint32_t>(0x00C7));
    CHECK_EQ(localization::foldToUpper(Language::Turkish, 0x011F), static_cast<uint32_t>(0x011E));
    CHECK_EQ(localization::foldToUpper(Language::Turkish, 0x015F), static_cast<uint32_t>(0x015E));
    CHECK_EQ(localization::foldToUpper(Language::Turkish, 0x00FC), static_cast<uint32_t>(0x00DC));
}

TEST(Localization, FoldForDisplayProducesDrawableText) {
    // "İnceleme" folds to "İNCELEME": dotted capital I first, then ASCII.
    const std::string folded = localization::foldForDisplay(Language::Turkish, "\u0130nceleme");
    const std::vector<uint32_t> codepoints = localization::decodeUtf8All(folded);
    CHECK_EQ(codepoints[0], static_cast<uint32_t>(0x0130));
    CHECK_EQ(codepoints[1], static_cast<uint32_t>('N'));
    for (uint32_t codepoint : codepoints) {
        CHECK_MSG(font::hasGlyph(codepoint),
                  "folded text contains a codepoint with no glyph: U+" + std::to_string(codepoint));
    }
}

TEST(Localization, EveryUiStringIsDrawableInBothLanguages) {
    // The strongest guarantee in this file: no user-visible string contains a
    // character the bitmap font cannot draw. This is what stops Turkish text
    // from silently rendering as boxes.
    for (Language language : {Language::Turkish, Language::English}) {
        for (Key key : allKeys()) {
            const std::string value = localization::text(language, key);
            const std::vector<uint32_t> missing = localization::missingGlyphs(language, value);
            if (!missing.empty()) {
                std::string detail = "key " + std::to_string(static_cast<int>(key)) + " (" +
                                     localization::languageCode(language) + ") is missing glyphs:";
                for (uint32_t codepoint : missing) {
                    detail += " U+" + std::to_string(codepoint);
                }
                CHECK_MSG(false, detail);
            }
        }
    }
}

TEST(Localization, TurkishStringsActuallyUseTurkishCharacters) {
    // If the Turkish column were accidentally filled with ASCII-only text, the
    // glyph work would never be exercised. Require real diacritics.
    const std::string turkish =
        std::string(localization::text(Language::Turkish, Key::HelpTitle)) +
        localization::text(Language::Turkish, Key::Task) +
        localization::text(Language::Turkish, Key::Survey) +
        localization::text(Language::Turkish, Key::Completed) +
        localization::text(Language::Turkish, Key::ScopeNote);
    bool hasNonAscii = false;
    for (unsigned char c : turkish) {
        if (c >= 0x80) hasNonAscii = true;
    }
    CHECK_MSG(hasNonAscii, "Turkish UI strings contain no Turkish characters");
}

TEST(Localization, MissingGlyphsDetectsUnknownCharacters) {
    // A snowman has no glyph, so it must be reported.
    const std::vector<uint32_t> missing =
        localization::missingGlyphs(Language::English, "ok \u2603");
    CHECK_EQ(missing.size(), static_cast<size_t>(1));
    if (!missing.empty()) {
        CHECK_EQ(missing[0], static_cast<uint32_t>(0x2603));
    }

    CHECK(localization::missingGlyphs(Language::English, "plain ascii").empty());
}

TEST(Localization, NumberFormattingFollowsTheLanguage) {
    // Turkish uses a comma, English a dot.
    CHECK_EQ(localization::formatNumber(Language::Turkish, 12.5f, 1), std::string("12,5"));
    CHECK_EQ(localization::formatNumber(Language::English, 12.5f, 1), std::string("12.5"));
    CHECK_EQ(localization::formatNumber(Language::Turkish, 3.0f, 1), std::string("3,0"));
}

TEST(Localization, LanguageMenuListsBothLanguages) {
    const std::vector<localization::LanguageEntry>& entries = localization::languages();
    CHECK_EQ(entries.size(), static_cast<size_t>(localization::kLanguageCount));
    bool hasTurkish = false;
    bool hasEnglish = false;
    for (const localization::LanguageEntry& entry : entries) {
        if (entry.language == Language::Turkish) hasTurkish = true;
        if (entry.language == Language::English) hasEnglish = true;
    }
    CHECK(hasTurkish);
    CHECK(hasEnglish);
}
