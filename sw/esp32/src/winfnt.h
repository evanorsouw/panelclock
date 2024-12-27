#ifndef _WINFNT_H_
#define _WINFNT_H_

#include <stdint.h>

// Font file header (starts the FNT file)
typedef struct {
    uint16_t dfVersion;     // Version of the font (0x0200 or 0x0300)
    uint32_t dfSize;        // Total size of the font file in bytes
    uint8_t dfCopyright[60];// Copyright notice (60 bytes)
    uint16_t dfType;        // Type flags (e.g., device or raster font)
    uint16_t dfPoints;      // Point size of the font
    uint16_t dfVertRes;     // Vertical resolution of the target device
    uint16_t dfHorizRes;    // Horizontal resolution of the target device
    uint16_t dfAscent;      // Height of the character cell above the baseline
    uint16_t dfInternalLeading; // Space above the tallest character
    uint16_t dfExternalLeading; // Space below the bottom line of characters
    uint8_t dfItalic;       // Italic flag (0 = no, 1 = yes)
    uint8_t dfUnderline;    // Underline flag (0 = no, 1 = yes)
    uint8_t dfStrikeOut;    // Strikeout flag (0 = no, 1 = yes)
    uint16_t dfWeight;      // Font weight (e.g., 400 = normal, 700 = bold)
    uint8_t dfCharSet;      // Character set identifier (e.g., ANSI = 1)
    uint16_t dfPixWidth;    // Width of the font (0 for proportional)
    uint16_t dfPixHeight;   // Height of the font
    uint8_t dfPitchAndFamily; // Pitch and family information
    uint16_t dfAvgWidth;    // Average width of characters
    uint16_t dfMaxWidth;    // Maximum width of characters
    uint8_t dfFirstChar;    // ASCII code of the first character
    uint8_t dfLastChar;     // ASCII code of the last character
    uint8_t dfDefaultChar;  // ASCII code of the default character
    uint8_t dfBreakChar;    // ASCII code of the break character
    uint16_t dfWidthBytes;  // Bytes per row of the bitmap
    uint32_t dfDevice;      // Offset to device name string (0 if none)
    uint32_t dfFace;        // Offset to typeface name string (0 if none)
    uint32_t dfBitsPointer; // Offset to bitmap data
    uint32_t dfBitsOffset;  // Offset to start of bitmap
    uint8_t dfReserved[4];  // Reserved for future use
} FontFileHeader;

// Character metrics for each glyph
typedef struct {
    uint8_t width;          // Width of the character bitmap
    uint8_t height;         // Height of the character bitmap
    int8_t xOffset;         // X offset of the glyph
    int8_t yOffset;         // Y offset of the glyph
    int8_t xAdvance;        // Cursor advance after rendering
    uint8_t bitmap[];       // Actual bitmap data (variable length)
} Glyph;

class WinFont {
private:

public:
    WinFont()
}

#endif