
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <esp_heap_caps.h>

#include "truetypefont.h"
#include "utf8encoding.h"

#if 0
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

std::map<const char *, fontinfo> TrueTypeFont::_loadedFonts;

TrueTypeFont::~TrueTypeFont()
{
    auto sftfont = _sft.font;

    auto it = std::find_if(_loadedFonts.begin(), _loadedFonts.end(), [=](std::pair<const char *, fontinfo> kv) { return kv.second.font == sftfont;});
    assert( it != _loadedFonts.end() );

    it->second.usecount--;
    if (it->second.usecount == 0)
    {
        delete it->second.font;
        _loadedFonts.erase(it);
        printf("unloaded font='%s' from memory\n", _fontname);
    }
}

TrueTypeFont *TrueTypeFont::getFont(const char *fontname, float dx, float dy)
{
    auto  it = _loadedFonts.find(fontname);
    if (it == _loadedFonts.end())
    {
        auto sftfont = loadFont(fontname);
        if (sftfont != nullptr)
        {
            _loadedFonts[fontname] = fontinfo { 
                .font = sftfont,
                .usecount = 0
            };
            it = _loadedFonts.find(fontname);
        }
        else
        {
            return nullptr;
        }
    }

    it->second.usecount++;
    SFT sft = {
	    .font = it->second.font,
        .xScale = dx,
        .yScale = dy,
        .flags = SFT_DOWNWARD_Y
    };

    SFT_LMetrics lmetrics;
    sft_lmetrics(&sft, &lmetrics);

    auto font = new TrueTypeFont(fontname, sft, lmetrics, dx, dy);

    printf("usecount for font %s: %d, height=%.1f, ascend=%.1f, descend=%.1f\n", 
        it->first, it->second.usecount, 
        font->height(), font->ascend(), font->descend());
    SFT_GMetrics gmetrics;
    sft_gmetrics(&sft, 0x20, &gmetrics);

    return font;
}

SFT_Font *TrueTypeFont::loadFont(const char *fontname)
{
    FILE *fp = nullptr;
    void *memory = nullptr;
    SFT_Font *font = nullptr;

    printf("loading font='%s'", fontname);
    do
    {
        auto path = std::string("/spiffs/") + fontname;
        struct stat info;
        if (stat(path.c_str(), &info) < 0)
        {
            printf(" - not found\n");
            break;
        }
        auto memory = heap_caps_malloc_prefer(info.st_size, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT);
        if (memory == nullptr)
        {
            printf(" - not enough memory\n");
            break;
        }
        fp = fopen(path.c_str(), "rb");
        if (fp == nullptr)
        {
            printf(" - cannot open\n");
            break;
        }
        auto n = fread(memory, 1, info.st_size, fp);
        if (n != info.st_size)
        {
            printf(" - failed to read all %ld bytes\n", info.st_size);
            break;
        }
        font = sft_loadmem(memory, info.st_size);
        if (font == nullptr)
        {
            printf(" - parse font data failed\n");
        }
    }
    while (false);

    if (fp != nullptr)
    {
        fclose(fp);
    }

    if (font != nullptr)
    {
        printf(" (size=%d, @%p)\n", font->size, font->memory);
    }
    else if (memory != nullptr)
    {
        free(memory);
    }
    return font;
}

int TrueTypeFont::splittext(const char *txt, float maxwidth) const
{
    auto dx = 0;
    auto codepoint = 0;
    auto i = 0;
    while ((codepoint = UTF8Encoding::nextCodepoint(txt, i)) != 0)
    {
        SFT_Glyph glyph;
        sft_lookup(&_sft, codepoint, &glyph);
        if (glyph == 0)
            continue;

        SFT_GMetrics mtx = {0};
        if (sft_gmetrics(&_sft, glyph, &mtx) == -1)
            continue;

        if (dx + mtx.advanceWidth > maxwidth)
            break;
        dx += mtx.advanceWidth;
    }
    return i;
}


textinfo TrueTypeFont::textsize(const char *txt) const
{
    auto codepoint = 0;
    auto i = 0;
    auto maxwidth = 0.0f;
    auto width = 0.0f;
    auto lines = 1;
    while ((codepoint = UTF8Encoding::nextCodepoint(txt, i)) != 0)
    {
        if (codepoint == '\n')
        {
            lines++;
            maxwidth = std::max(maxwidth, width);
            width = 0.0f;
        }
        else
        {
            SFT_Glyph glyph;
            sft_lookup(&_sft, codepoint, &glyph);
            if (glyph == 0)
                continue;

            SFT_GMetrics mtx = {0};
            if (sft_gmetrics(&_sft, glyph, &mtx) == -1)
                continue;

            width += mtx.advanceWidth;
        }
    }
    return textinfo(std::max(maxwidth, width), lines * (_lmetrics.ascender - _lmetrics.descender));
}

textinfo TrueTypeFont::charsize(int codepoint) const
{
    textinfo size(0, _lmetrics.ascender - _lmetrics.descender);

    SFT_Glyph glyph;
    sft_lookup(&_sft, codepoint, &glyph);
    if (glyph != 0)
    {
        SFT_GMetrics mtx;
        sft_gmetrics(&_sft, glyph, &mtx);
        size.dx += mtx.advanceWidth;
    }
    return size;
}
