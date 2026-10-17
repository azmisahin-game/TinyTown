#pragma once

// A 5x9 bitmap font used by the in-app overlay.
//
// Glyphs are defined as ASCII art so a human can read and correct them, and
// there is no graphics dependency: glyphPixel() is a pure function that the
// test binary checks.
//
// Layout of a 5x9 cell:
//   rows 0..1  diacritic zone (umlaut, breve, dot) for Turkish letters
//   rows 2..8  the seven-row letter body, shared by ASCII and Turkish letters
//
// Two Turkish letters carry a mark below the body (C-cedilla, S-cedilla).
// Those use rows 1..7 for the body and row 8 for the cedilla, so both cases
// still fit in 9 rows.
//
// The font is uppercase-only by design. Case folding is language aware and
// lives in tt::localization, because Turkish maps 'i' to dotted capital I and
// dotless 'i' to 'I'. The font only draws the codepoints it is handed.

#include <cstdint>

namespace tt::font {

constexpr int kGlyphWidth = 5;
constexpr int kGlyphHeight = 9;
constexpr int kCellWidth = 6;    // glyph plus one column of spacing
constexpr int kCellHeight = 10;  // glyph plus one row of spacing
constexpr int kBodyTop = 2;      // first row of the seven-row letter body

// True when the codepoint can be drawn.
bool hasGlyph(uint32_t codepoint);

// Pixel test in glyph space. row 0 is the top row, col 0 the left column.
// Returns false for out-of-range coordinates or unsupported codepoints.
bool glyphPixel(uint32_t codepoint, int row, int col);

// Nine row strings of five characters ('#' = ink), for inspection and tests.
// Returns nullptr when the codepoint has no glyph.
const char* const* glyphArt(uint32_t codepoint);

// Codepoints with a glyph, in ascending order. Used by tests and by the
// coverage report.
const uint32_t* supportedCodepoints(int& count);

}  // namespace tt::font
