
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>

#include "font.h"

#if 0
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

std::map<const char *, fontinfo> Font::_loadedFonts;

Font::~Font()
{
    auto sftfont = _sft.font;

    auto it = std::find_if(_loadedFonts.begin(), _loadedFonts.end(), [=](std::pair<const char *, fontinfo> kv) { return kv.second.font == sftfont;});
    assert( it != _loadedFonts.end());

    it->second.usecount--;
    if (it->second.usecount == 0)
    {
        delete it->second.font;
        _loadedFonts.erase(it);
        printf("unloaded font='%s' from memory\n", _fontname);
    }
}

Font *Font::getFont(const char *fontname, float dx, float dy)
{
    auto  it = _loadedFonts.find(fontname);
    if (it == _loadedFonts.end())
    {
        auto sftfont = loadfont(fontname);
        if (sftfont == nullptr)
        {
            if (std::strrchr(fontname, '.') == nullptr)
            {
                sftfont = loadfont((std::string(fontname) + ".ttf").c_str());
            } 
        }
        if (sftfont != nullptr)
        {
            printf("loaded font='%s' (size=%d, @%p)\n", fontname, sftfont->size, sftfont->memory);
            _loadedFonts[fontname] = fontinfo { 
                .font = sftfont,
                .usecount = 0
            };
            it = _loadedFonts.find(fontname);
        }
        else
        {
            printf("failed to load font='%s'\n", fontname);
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

    vTaskDelay(10);
    auto font = new Font(fontname, sft, lmetrics, dx, dy);

    printf("usecount for font %s: %d, height=%.1f, ascend=%.1f, descend=%.1f\n", 
        it->first, it->second.usecount, 
        font->height(), font->ascend(), font->descend());

    return font;
}

SFT_Font *Font::loadfont(const char *fontname)
{
    FILE *fp = nullptr;
    void *memory = nullptr;
    SFT_Font *font = nullptr;

    LOG("loading font='%s' into memory", fontname);
    do
    {
        auto path = std::string("/spiffs/") + fontname;
        struct stat info;
        if (stat(path.c_str(), &info) < 0)
        {
            LOG(" - not found\n");
            break;
        }
        auto memory = heap_caps_malloc_prefer(info.st_size, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT);
        if (memory == nullptr)
        {
            LOG(" - not enough memory\n");
            break;
        }
        FILE *fp = fopen(path.c_str(), "rb");
        if (fp == nullptr)
        {
            LOG(" - cannot open\n");
            break;
        }
        auto n = fread(memory, 1, info.st_size, fp);
        if (n != info.st_size)
        {
            LOG(" - failed to read all %ld bytes\n", info.st_size);
            break;
        }
        font = sft_loadmem(memory, info.st_size);
        if (font == nullptr)
        {
            LOG(" - parse font data failed\n");
        }
    }
    while (false);

    if (fp != nullptr)
    {
        fclose(fp);
    }
    if (font == nullptr && memory != nullptr)
    {
        free(memory);
    }
    return font;
}

textinfo Font::textsize(const char *txt, int n)
{
    struct textinfo size = { 
        .dy = _lmetrics.ascender - _lmetrics.descender,
    };

    auto len = (int)strlen(txt);
    if (n > 0 && n > len)
    {
        len = n;
    }
    for (int i=0; i<len; ++i)
    {
        SFT_Glyph glyph;
        sft_lookup(&_sft, txt[i], &glyph);
        if (glyph == 0)
            continue;

        SFT_GMetrics mtx;
        sft_gmetrics(&_sft, glyph, &mtx);

        size.dx += mtx.advanceWidth;
    }
    return size;
}

textinfo Font::charsize(char c)
{
    struct textinfo size = { 
        .dx = 0,
        .dy = _lmetrics.ascender - _lmetrics.descender
    };

    SFT_Glyph glyph;
    sft_lookup(&_sft, c, &glyph);
    if (glyph != 0)
    {
        SFT_GMetrics mtx;
        sft_gmetrics(&_sft, glyph, &mtx);
        size.dx += mtx.advanceWidth;
    }
    return size;
}
