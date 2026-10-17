// Font tests. The font is pure data plus a pure lookup, so it can be checked
// without a window. These guard the mistakes that are easy to make and hard to
// see: a missing glyph, a glyph that is not rectangular, and two glyphs that
// are accidentally identical.

#include "ui/font5x9.hpp"

#include <cstring>
#include <string>

#include "test_framework.hpp"

using namespace tt;

namespace {

// The font is uppercase-only by design. A lowercase ASCII letter is drawn by
// folding it to its uppercase form, so the real contract is: every printable
// ASCII character either has a glyph or folds to one that does.
uint32_t asciiFold(uint32_t c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}

bool hasInk(uint32_t codepoint) {
    for (int row = 0; row < font::kGlyphHeight; ++row) {
        for (int col = 0; col < font::kGlyphWidth; ++col) {
            if (font::glyphPixel(codepoint, row, col)) return true;
        }
    }
    return false;
}

bool sameShape(uint32_t a, uint32_t b) {
    for (int row = 0; row < font::kGlyphHeight; ++row) {
        for (int col = 0; col < font::kGlyphWidth; ++col) {
            if (font::glyphPixel(a, row, col) != font::glyphPixel(b, row, col)) return false;
        }
    }
    return true;
}

}  // namespace

TEST(Font, EveryPrintableAsciiCharacterIsDrawable) {
    // Directly or through case folding. The font stores only uppercase ASCII,
    // so 'a'..'z' are covered by 'A'..'Z'.
    for (uint32_t c = 32; c <= 126; ++c) {
        const uint32_t folded = asciiFold(c);
        CHECK_MSG(font::hasGlyph(folded),
                  "missing glyph for code " + std::to_string(c) + " (folds to " +
                      std::to_string(folded) + ")");
    }
}

TEST(Font, UppercaseAsciiIsStoredDirectly) {
    // Folding must not be the only path for uppercase text.
    for (uint32_t c = 'A'; c <= 'Z'; ++c) {
        CHECK_MSG(font::hasGlyph(c), "missing uppercase glyph " + std::string(1, static_cast<char>(c)));
    }
    for (uint32_t c = '0'; c <= '9'; ++c) {
        CHECK_MSG(font::hasGlyph(c), "missing digit glyph " + std::string(1, static_cast<char>(c)));
    }
}

TEST(Font, TurkishLettersHaveGlyphs) {
    // C-cedilla, c-cedilla, G-breve, g-breve, dotted I, dotless i,
    // O-umlaut, o-umlaut, S-cedilla, s-cedilla, U-umlaut, u-umlaut.
    const uint32_t turkish[] = {0x00C7, 0x00E7, 0x011E, 0x011F, 0x0130, 0x0131,
                                0x00D6, 0x00F6, 0x015E, 0x015F, 0x00DC, 0x00FC};
    for (uint32_t codepoint : turkish) {
        CHECK_MSG(font::hasGlyph(codepoint),
                  "missing Turkish glyph U+" + std::to_string(codepoint));
    }
}

TEST(Font, ControlCharactersHaveNoGlyph) {
    CHECK(!font::hasGlyph('\n'));
    CHECK(!font::hasGlyph('\t'));
    CHECK(!font::hasGlyph(1));
    CHECK(!font::hasGlyph(0x1F600));  // emoji: no glyph, must not crash
}

TEST(Font, GlyphArtIsWellFormed) {
    int count = 0;
    const uint32_t* codepoints = font::supportedCodepoints(count);
    CHECK(count > 80);

    for (int i = 0; i < count; ++i) {
        const char* const* rows = font::glyphArt(codepoints[i]);
        CHECK_MSG(rows != nullptr, "glyphArt returned null for a supported codepoint");
        if (rows == nullptr) continue;
        for (int row = 0; row < font::kGlyphHeight; ++row) {
            CHECK_MSG(rows[row] != nullptr, "glyph row is null");
            if (rows[row] == nullptr) continue;
            CHECK_MSG(std::strlen(rows[row]) == static_cast<size_t>(font::kGlyphWidth),
                      "glyph U+" + std::to_string(codepoints[i]) + " row " +
                          std::to_string(row) + " has width " +
                          std::to_string(std::strlen(rows[row])) + ", expected " +
                          std::to_string(font::kGlyphWidth));
        }
    }
}

TEST(Font, SupportedCodepointsAreSortedAndUnique) {
    // Lookup binary searches the table, so the order is a correctness
    // requirement, not a preference.
    int count = 0;
    const uint32_t* codepoints = font::supportedCodepoints(count);
    for (int i = 1; i < count; ++i) {
        CHECK_MSG(codepoints[i] > codepoints[i - 1],
                  "codepoints must be strictly ascending at index " + std::to_string(i));
    }
}

TEST(Font, SpaceIsBlankAndLettersAreNot) {
    CHECK(!hasInk(' '));
    CHECK(hasInk('A'));
    CHECK(hasInk('Z'));
    CHECK(hasInk('0'));
    CHECK(hasInk('9'));
    // Turkish letters must carry real ink, not just an empty box.
    CHECK(hasInk(0x00C7));
    CHECK(hasInk(0x011E));
    CHECK(hasInk(0x0130));
    CHECK(hasInk(0x015E));
    CHECK(hasInk(0x00DC));
}

TEST(Font, TurkishLettersAreDistinctFromTheirAsciiBases) {
    // A diacritic that was never drawn would silently look like the base letter.
    CHECK_MSG(!sameShape('C', 0x00C7), "C-cedilla looks identical to C");
    CHECK_MSG(!sameShape('G', 0x011E), "G-breve looks identical to G");
    CHECK_MSG(!sameShape('I', 0x0130), "dotted I looks identical to I");
    CHECK_MSG(!sameShape('O', 0x00D6), "O-umlaut looks identical to O");
    CHECK_MSG(!sameShape('S', 0x015E), "S-cedilla looks identical to S");
    CHECK_MSG(!sameShape('U', 0x00DC), "U-umlaut looks identical to U");
}

TEST(Font, DiacriticsAppearInTheDiacriticZone) {
    // The breve on G-breve must be drawn above the letter body.
    bool breveInk = false;
    for (int row = 0; row < font::kBodyTop; ++row) {
        for (int col = 0; col < font::kGlyphWidth; ++col) {
            if (font::glyphPixel(0x011E, row, col)) breveInk = true;
        }
    }
    CHECK_MSG(breveInk, "G-breve has no ink in the diacritic zone");

    // The cedilla on C-cedilla must be drawn below the letter body.
    bool cedillaInk = false;
    const int lastRow = font::kGlyphHeight - 1;
    for (int col = 0; col < font::kGlyphWidth; ++col) {
        if (font::glyphPixel(0x00C7, lastRow, col)) cedillaInk = true;
    }
    CHECK_MSG(cedillaInk, "C-cedilla has no ink on the bottom row");
}

TEST(Font, OutOfRangeCoordinatesAreFalse) {
    CHECK(!font::glyphPixel('A', -1, 0));
    CHECK(!font::glyphPixel('A', font::kGlyphHeight, 0));
    CHECK(!font::glyphPixel('A', 0, -1));
    CHECK(!font::glyphPixel('A', 0, font::kGlyphWidth));
}

TEST(Font, DigitsAreDistinct) {
    // A copy-paste error between digit glyphs is otherwise invisible.
    for (uint32_t a = '0'; a <= '9'; ++a) {
        for (uint32_t b = a + 1; b <= '9'; ++b) {
            CHECK_MSG(!sameShape(a, b), std::string("digits ") + static_cast<char>(a) + " and " +
                                            static_cast<char>(b) + " are identical");
        }
    }
}

TEST(Font, UnknownCodepointDrawsAVisibleBox) {
    // A missing glyph must be visible, never blank. The replacement box is used
    // for any codepoint without a glyph.
    const uint32_t unknown = 0x2603;  // snowman
    CHECK(!font::hasGlyph(unknown));
    bool anyInk = false;
    for (int row = 0; row < font::kGlyphHeight; ++row) {
        for (int col = 0; col < font::kGlyphWidth; ++col) {
            if (font::glyphPixel(unknown, row, col)) anyInk = true;
        }
    }
    CHECK_MSG(anyInk, "an unsupported codepoint drew nothing at all");
}
