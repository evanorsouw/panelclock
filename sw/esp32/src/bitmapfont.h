
#ifndef _BITMAPFONT_H_
#define _BITMAPFONT_H_

#include <map>

#include "font.h"

class BitmapFont : public Font
{
friend class Graphics;
private:
    struct bitmapfont 
    {
        uint8_t version;              // 0x0: version '1'
        uint8_t flags;                // 0x1: flags describing font
        uint8_t ascend;               // 0x2: how many pixels from baseline upwards.
        uint8_t descend;              // 0x3: how many pixels underneeth baseline
        uint8_t linespacing;          // 0x4: how many additional pixels between successive lines
        uint8_t tracking;             // 0x5: how many pixels between successive letters
        uint16_t glyphcount;          // 0x6:
        uint16_t offsetGlyphTable;    // 0x8: each entry points to a glyph object
        uint16_t offsetDescription;   // 0xA: points to textual description of font
    };

    struct glyphInfo
    {
        uint16_t unicode;
        uint8_t width;             // max number of horizontal pixels in this glyph
        uint8_t height;            // max number of vertical scanlines in this glyph
        int8_t baselineOffset;     // offset for glyph relative to baseline (0:on-baseline, <0:below baseline, >0 above baseline) 
        uint8_t glyphData1;        // first byte of glyphdata, (width/8) * height
    };

private:
    const char* _fontname;
    bitmapfont* _info;
    void* _filedata;
    int _maxWidth;
    int _maxHeight;
    std::map<uint32_t, glyphInfo*> _glyphs;

    BitmapFont() : Font(FontType::WMF) {}

public:
    static BitmapFont *getFont(const char *font);
    ~BitmapFont() override;

    int splittext(const char *txt, float maxwidth) const override;
    textinfo textsize(const char *txt) const override;
    textinfo charsize(int codepoint) const override;

    float sizex() const override { return _maxWidth; }
    float sizey() const override { return _maxHeight; }
    float ascend() const override { return _info->ascend; }
    float descend() const override { return -_info->descend; }  // in accordance with 'schrift' we expose a negative descend
    float height() const override { return _info->ascend + _info->descend + _info->linespacing; }
    float tracking() const { return _info->tracking; }

private:
    static BitmapFont *loadFont(const char *fontname, void *filedata, int filesize);
    static void fromBE(uint16_t &data);
    glyphInfo *getGlyph(uint32_t codepoint) const;
};

#endif
