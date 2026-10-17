// The HUD font is pure data plus a pure lookup function, so it can be tested
// without a window. This guards the two mistakes that are easy to make and
// hard to see: a glyph that is missing, and a glyph that is not rectangular.

#include "ui/font5x7.hpp"

#include <cstring>
#include <string>

#include "test_framework.hpp"

using namespace tt;

TEST(Font, EveryPrintableCharacterHasAGlyph) {
    for (char c = 32; c <= 126; ++c) {
        CHECK_MSG(font::hasGlyph(c), std::string("missing glyph for code ") +
                                        std::to_string(static_cast<int>(c)));
    }
}

TEST(Font, ControlCharactersHaveNoGlyph) {
    CHECK(!font::hasGlyph('\n'));
    CHECK(!font::hasGlyph('\t'));
    CHECK(!font::hasGlyph(static_cast<char>(1)));
}

TEST(Font, GlyphRowsAreWellFormed) {
    for (char c = 32; c <= 126; ++c) {
        const char* const* rows = font::glyphRows(c);
        CHECK_MSG(rows != nullptr, "glyphRows returned null for a printable character");
        if (rows == nullptr) continue;
        for (int row = 0; row < font::kGlyphHeight; ++row) {
            CHECK_MSG(rows[row] != nullptr, "glyph row is null");
            if (rows[row] == nullptr) continue;
            CHECK_MSG(std::strlen(rows[row]) == static_cast<size_t>(font::kGlyphWidth),
                      std::string("glyph '") + c + "' row " + std::to_string(row) +
                          " has width " + std::to_string(std::strlen(rows[row])) + ", expected " +
                          std::to_string(font::kGlyphWidth));
        }
    }
}

TEST(Font, SpaceIsBlankAndLettersAreNot) {
    for (int row = 0; row < font::kGlyphHeight; ++row) {
        for (int col = 0; col < font::kGlyphWidth; ++col) {
            CHECK(!font::glyphPixel(' ', row, col));
        }
    }

    auto hasInk = [](char c) {
        for (int row = 0; row < font::kGlyphHeight; ++row) {
            for (int col = 0; col < font::kGlyphWidth; ++col) {
                if (font::glyphPixel(c, row, col)) return true;
            }
        }
        return false;
    };

    CHECK(hasInk('A'));
    CHECK(hasInk('Z'));
    CHECK(hasInk('0'));
    CHECK(hasInk('9'));
}

TEST(Font, LowercaseFoldsToUppercase) {
    CHECK_EQ(font::normalize('a'), 'A');
    CHECK_EQ(font::normalize('z'), 'Z');
    CHECK_EQ(font::normalize('M'), 'M');

    // Folding must produce identical pixels.
    for (int row = 0; row < font::kGlyphHeight; ++row) {
        for (int col = 0; col < font::kGlyphWidth; ++col) {
            CHECK_EQ(font::glyphPixel('a', row, col), font::glyphPixel('A', row, col));
        }
    }
}

TEST(Font, OutOfRangeCoordinatesAreFalse) {
    CHECK(!font::glyphPixel('A', -1, 0));
    CHECK(!font::glyphPixel('A', font::kGlyphHeight, 0));
    CHECK(!font::glyphPixel('A', 0, -1));
    CHECK(!font::glyphPixel('A', 0, font::kGlyphWidth));
}

TEST(Font, DigitsAreDistinct) {
    // A copy-paste error between digit glyphs is otherwise invisible.
    for (char a = '0'; a <= '9'; ++a) {
        for (char b = static_cast<char>(a + 1); b <= '9'; ++b) {
            bool identical = true;
            for (int row = 0; row < font::kGlyphHeight && identical; ++row) {
                for (int col = 0; col < font::kGlyphWidth; ++col) {
                    if (font::glyphPixel(a, row, col) != font::glyphPixel(b, row, col)) {
                        identical = false;
                        break;
                    }
                }
            }
            CHECK_MSG(!identical, std::string("digits ") + a + " and " + b + " are identical");
        }
    }
}
