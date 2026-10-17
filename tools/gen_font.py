#!/usr/bin/env python3
"""Generate src/ui/font5x9.cpp from readable glyph definitions.

Every glyph is 5 columns x 9 rows. Rows 2..8 hold the seven-row letter body
(shared with the original ASCII font). Rows 0..1 hold a diacritic zone.
C-cedilla and S-cedilla use rows 1..7 for the body and row 8 for the cedilla.

This script is a build-time authoring aid, not part of the CMake build.
"""

W = 5
H = 9

# Seven-row bodies, shared by ASCII and Turkish letters.
BODY = {
    " ": [".....", ".....", ".....", ".....", ".....", ".....", "....."],
    "!": ["..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.."],
    '"': [".#.#.", ".#.#.", ".....", ".....", ".....", ".....", "....."],
    "#": [".#.#.", ".#.#.", "#####", ".#.#.", "#####", ".#.#.", ".#.#."],
    "$": ["..#..", ".####", "#.#..", ".###.", "..#.#", "####.", "..#.."],
    "%": ["##...", "##..#", "...#.", "..#..", ".#...", "#..##", "...##"],
    "&": [".##..", "#..#.", "#.#..", ".#...", "#.#.#", "#..#.", ".##.#"],
    "'": ["..#..", "..#..", ".....", ".....", ".....", ".....", "....."],
    "(": ["...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#."],
    ")": [".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#..."],
    "*": [".....", "#.#.#", ".###.", "#####", ".###.", "#.#.#", "....."],
    "+": [".....", "..#..", "..#..", "#####", "..#..", "..#..", "....."],
    ",": [".....", ".....", ".....", ".....", "..##.", "..#..", ".#..."],
    "-": [".....", ".....", ".....", "#####", ".....", ".....", "....."],
    ".": [".....", ".....", ".....", ".....", ".....", ".##..", ".##.."],
    "/": ["....#", "...#.", "...#.", "..#..", ".#...", ".#...", "#...."],
    "0": [".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."],
    "1": ["..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."],
    "2": [".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"],
    "3": ["#####", "...#.", "..#..", "...#.", "....#", "#...#", ".###."],
    "4": ["...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."],
    "5": ["#####", "#....", "####.", "....#", "....#", "#...#", ".###."],
    "6": ["..##.", ".#...", "#....", "####.", "#...#", "#...#", ".###."],
    "7": ["#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."],
    "8": [".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."],
    "9": [".###.", "#...#", "#...#", ".####", "....#", "...#.", ".##.."],
    ":": [".....", ".##..", ".##..", ".....", ".##..", ".##..", "....."],
    ";": [".....", ".##..", ".##..", ".....", ".##..", "..#..", ".#..."],
    "<": ["...#.", "..#..", ".#...", "#....", ".#...", "..#..", "...#."],
    "=": [".....", ".....", "#####", ".....", "#####", ".....", "....."],
    ">": [".#...", "..#..", "...#.", "....#", "...#.", "..#..", ".#..."],
    "?": [".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.."],
    "@": [".###.", "#...#", "#.###", "#.#.#", "#.###", "#....", ".###."],
    "A": [".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
    "B": ["####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."],
    "C": [".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."],
    "D": ["###..", "#..#.", "#...#", "#...#", "#...#", "#..#.", "###.."],
    "E": ["#####", "#....", "#....", "####.", "#....", "#....", "#####"],
    "F": ["#####", "#....", "#....", "####.", "#....", "#....", "#...."],
    "G": [".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".###."],
    "H": ["#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
    "I": [".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."],
    "J": ["..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."],
    "K": ["#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"],
    "L": ["#....", "#....", "#....", "#....", "#....", "#....", "#####"],
    "M": ["#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"],
    "N": ["#...#", "#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#"],
    "O": [".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
    "P": ["####.", "#...#", "#...#", "####.", "#....", "#....", "#...."],
    "Q": [".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"],
    "R": ["####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"],
    "S": [".####", "#....", "#....", ".###.", "....#", "....#", "####."],
    "T": ["#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
    "U": ["#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
    "V": ["#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."],
    "W": ["#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"],
    "X": ["#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"],
    "Y": ["#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."],
    "Z": ["#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"],
    "[": [".###.", ".#...", ".#...", ".#...", ".#...", ".#...", ".###."],
    "\\": ["#....", ".#...", ".#...", "..#..", "...#.", "...#.", "....#"],
    "]": [".###.", "...#.", "...#.", "...#.", "...#.", "...#.", ".###."],
    "^": ["..#..", ".#.#.", "#...#", ".....", ".....", ".....", "....."],
    "_": [".....", ".....", ".....", ".....", ".....", ".....", "#####"],
    "`": [".#...", "..#..", ".....", ".....", ".....", ".....", "....."],
    "{": ["..##.", ".#...", ".#...", "##...", ".#...", ".#...", "..##."],
    "|": ["..#..", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
    "}": [".##..", "...#.", "...#.", "..##.", "...#.", "...#.", ".##.."],
    "~": [".....", ".....", ".#..#", "#.#.#", "#..#.", ".....", "....."],
}

CEDILLA = ["..##."]
DIAERESIS = [".....", ".#.#."]
# A breve dips in the middle. The lower row is narrower than the G body below
# it, so the mark stays visually separate from the letter.
BREVE = ["#...#", "..#.."]
DOT_ABOVE = [".....", "..#.."]
BLANK_TOP = [".....", "....."]
ACUTE = [".....", "...#."]
CIRCUMFLEX = ["..#..", ".#.#."]

# (codepoint, name, rows[9])
UNICODE_GLYPHS = [
    # C-cedilla and S-cedilla: body in rows 1..7, cedilla in row 8.
    (0x00C7, "Ccedilla", ["....."] + BODY["C"] + CEDILLA),
    (0x00E7, "ccedilla", ["....."] + BODY["C"] + CEDILLA),
    (0x015E, "Scedilla", ["....."] + BODY["S"] + CEDILLA),
    (0x015F, "scedilla", ["....."] + BODY["S"] + CEDILLA),
    # G-breve: breve above the body.
    (0x011E, "Gbreve", BREVE + BODY["G"]),
    (0x011F, "gbreve", BREVE + BODY["G"]),
    # Dotted capital I and dotless lowercase i.
    (0x0130, "Idot", DOT_ABOVE + BODY["I"]),
    (0x0131, "dotlessi", BLANK_TOP + BODY["I"]),
    # O-umlaut and U-umlaut.
    (0x00D6, "Oumlaut", DIAERESIS + BODY["O"]),
    (0x00F6, "oumlaut", DIAERESIS + BODY["O"]),
    (0x00DC, "Uumlaut", DIAERESIS + BODY["U"]),
    (0x00FC, "uumlaut", DIAERESIS + BODY["U"]),
    # Accented E, common in borrowed words.
    (0x00C9, "Eacute", ACUTE + BODY["E"]),
    (0x00E9, "eacute", ACUTE + BODY["E"]),
    # Circumflex A, as in Turkish "zeka" written with a hat.
    (0x00C2, "Acircumflex", CIRCUMFLEX + BODY["A"]),
    (0x00E2, "acircumflex", CIRCUMFLEX + BODY["A"]),
    # Em dash, used in headings and version strings.
    (0x2014, "emdash", BLANK_TOP + [".....", ".....", "#####", ".....", ".....", ".....", "....."]),
]


def validate(name, rows):
    if len(rows) != H:
        raise SystemExit(f"{name}: expected {H} rows, got {len(rows)}")
    for index, row in enumerate(rows):
        if len(row) != W:
            raise SystemExit(f"{name}: row {index} has width {len(row)}, expected {W}")
        for ch in row:
            if ch not in ".#":
                raise SystemExit(f"{name}: row {index} has invalid char {ch!r}")


def emit_entry(codepoint, name, rows):
    validate(name, rows)
    lines = [f'    {{0x{codepoint:04X}, "{name}",']
    for row in rows:
        lines.append(f'     "{row}",')
    lines.append("    },")
    return "\n".join(lines)


def main():
    glyphs = []
    # ASCII glyphs occupy the seven-row body; pad with an empty diacritic zone.
    for character, body in BODY.items():
        glyphs.append((ord(character), f"U{ord(character):04X}", BLANK_TOP + body))
    for codepoint, name, rows in UNICODE_GLYPHS:
        glyphs.append((codepoint, name, rows))

    # The lookup binary searches the table, so the order must be ascending.
    glyphs.sort(key=lambda item: item[0])
    if len({item[0] for item in glyphs}) != len(glyphs):
        raise SystemExit("duplicate codepoints in the glyph table")

    entries = [emit_entry(*item) for item in glyphs]
    count = len(glyphs)
    joined = "\n".join(entries)

    cpp = f'''#include "ui/font5x9.hpp"

// GENERATED FILE. Do not edit by hand.
// Regenerate with tools/gen_font.py. Every glyph is {W} columns x {H} rows.

namespace tt::font {{
namespace {{

struct Glyph {{
    uint32_t codepoint;
    const char* name;
    const char* rows[kGlyphHeight];
}};

// Sorted by codepoint so lookup can binary search.
const Glyph kGlyphs[] = {{
{joined}
}};

constexpr int kGlyphCount = {count};

// Drawn when a codepoint has no glyph, so a missing glyph is visible in the
// interface instead of silently disappearing.
const char* const kMissingGlyph[kGlyphHeight] = {{
    "#####", "#...#", "#...#", "#...#", "#...#", "#...#", "#####", ".....", "#####",
}};

const Glyph* findGlyph(uint32_t codepoint) {{
    int low = 0;
    int high = kGlyphCount - 1;
    while (low <= high) {{
        const int mid = low + (high - low) / 2;
        const uint32_t value = kGlyphs[mid].codepoint;
        if (value == codepoint) return &kGlyphs[mid];
        if (value < codepoint) {{
            low = mid + 1;
        }} else {{
            high = mid - 1;
        }}
    }}
    return nullptr;
}}

}}  // namespace

bool hasGlyph(uint32_t codepoint) {{ return findGlyph(codepoint) != nullptr; }}

const char* const* glyphArt(uint32_t codepoint) {{
    const Glyph* glyph = findGlyph(codepoint);
    return glyph == nullptr ? nullptr : glyph->rows;
}}

bool glyphPixel(uint32_t codepoint, int row, int col) {{
    if (row < 0 || row >= kGlyphHeight || col < 0 || col >= kGlyphWidth) return false;
    const Glyph* glyph = findGlyph(codepoint);
    const char* const* rows = glyph == nullptr ? kMissingGlyph : glyph->rows;
    return rows[row][col] == '#';
}}

const uint32_t* supportedCodepoints(int& count) {{
    static uint32_t codepoints[kGlyphCount];
    static bool built = false;
    if (!built) {{
        for (int i = 0; i < kGlyphCount; ++i) codepoints[i] = kGlyphs[i].codepoint;
        built = true;
    }}
    count = kGlyphCount;
    return codepoints;
}}

}}  // namespace tt::font
'''

    with open("src/ui/font5x9.cpp", "w") as handle:
        handle.write(cpp)
    print(f"wrote src/ui/font5x9.cpp with {count} glyphs")


if __name__ == "__main__":
    main()
