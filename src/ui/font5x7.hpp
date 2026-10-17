#pragma once

// A 5x7 uppercase bitmap font used by the in-app overlay.
//
// The font is defined as ASCII art so a human can read and correct it, and it
// has no graphics dependency: glyphPixel() is a pure function that the test
// binary checks for coverage. Lowercase letters are drawn as uppercase, which
// is a deliberate, documented style choice for the HUD.

namespace tt::font {

constexpr int kGlyphWidth = 5;
constexpr int kGlyphHeight = 7;
constexpr int kCellWidth = 6;   // glyph plus one column of spacing
constexpr int kCellHeight = 8;  // glyph plus one row of spacing
constexpr int kFirstChar = 32;
constexpr int kLastChar = 126;
constexpr int kGlyphCount = kLastChar - kFirstChar + 1;

// True when the character can be drawn. Lowercase ASCII maps to uppercase.
bool hasGlyph(char c);

// Character actually used when drawing (lowercase folded to uppercase).
char normalize(char c);

// Pixel test in glyph space. row 0 is the top row, col 0 the left column.
// Returns false for out-of-range coordinates or unsupported characters.
bool glyphPixel(char c, int row, int col);

// Row string (7 entries of 5 characters, '#' = ink) for inspection/testing.
// Returns nullptr when the character has no glyph.
const char* const* glyphRows(char c);

}  // namespace tt::font
