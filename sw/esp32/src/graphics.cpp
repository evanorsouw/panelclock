
# include <algorithm>
# include <cmath>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>

#include "esp_heap_caps.h"
#include "esp_system.h"
#include "graphics.h"

#if 0
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

std::map<const char *, SFT_Font*> Graphics::_fonts;

void Graphics::rect(Bitmap &tgt, float x, float y, float dx, float dy, Color color)
{

}

void Graphics::line(Bitmap &tgt, float x1, float y1, float x2, float y2, float thickness, Color color)
{
    if (thickness == 1.0f)
    {

    }
    else
    {

    }
}

void Graphics::setfont(const char *name, float sizex, float sizey)
{
    _activeFont = getfont(name);
    _activeFontSizeX = std::max(4.0f, sizex);
    _activeFontSizeY = std::max(4.0f, sizey - 1);
}

float Graphics::text(Bitmap &tgt, float x, float y, const char *txt, Color color)
{
    SFT sft;
    sft.font = _activeFont;
    sft.xScale = _activeFontSizeX;
    sft.yScale = _activeFontSizeY;
    sft.flags = SFT_DOWNWARD_Y;

    SFT_LMetrics lmtx;
    sft_lmetrics(&sft, &lmtx); 

    LOG("active font: size:%fx%f, scale=%fx%f, asc=%f, dsc=%f, gap=%f\n", 
        _activeFontSizeX, _activeFontSizeY, sft.xScale, sft.yScale, 
        lmtx.ascender, lmtx.descender, lmtx.lineGap);

    auto len = strlen(txt);
    for (int i=0; i<len; ++i)
    {
        SFT_Glyph glyph;
        sft_lookup(&sft, txt[i], &glyph);
        LOG("@x=%f: character='%c' converts to glyph='%d'\n", x, txt[i], glyph);
        if (glyph == 0)
            continue;

        auto tx = (int)std::floor(x);
        auto ty = (int)std::floor(y);
        sft.xOffset = x - tx;
        sft.yOffset = -(y - ty);

        SFT_GMetrics mtx;
        sft_gmetrics(&sft, glyph, &mtx);
        LOG("glyph=%d, has size='%dx%d', advancex=%f, leftbearing=%f, yoffset=%d\n", 
            glyph, mtx.minWidth, mtx.minHeight, mtx.advanceWidth, mtx.leftSideBearing, mtx.yOffset);
        assureMaskSize(mtx.minWidth,  mtx.minHeight);
        
        tx += std::floor(mtx.leftSideBearing);
        ty = ty + mtx.yOffset + 1;

        auto sx = 0;
        auto sy = 0;
        auto dx = mtx.minWidth;
        auto dy = mtx.minHeight;
        clip(sx, tx, dx, tgt.dx());
        clip(sy, ty, dy, tgt.dy());

        if (dx > 0 && dy > 0)
        {
            auto result = sft_render(&sft, glyph, _mask);
            LOG("render glyph=%d, offset=%fx%f onto bitmap={%dx%d,%p} => %d\n", 
                glyph, sft.xOffset, sft.yOffset, _mask.width, _mask.height, _mask.pixels, result);

            LOG("draw at %dx%d\n", tx, ty);
            if (dx > 0 && dy > 0)
            {
                auto pm = (uint8_t*)_mask.pixels;
                for (auto iy=0; iy < dy; iy++)
                {
                    for (auto ix=0; ix < dx; ++ix)
                    {
                        auto alpha = pm[ix];
                        tgt.set((int)tx + ix, ty + iy, color, alpha);
                    }
                    pm += _mask.width;
                }
            }
        }
        x += mtx.advanceWidth;
    }
    return x;
}

void Graphics::assureMaskSize(int dx, int dy)
{
    if (dx > _mask.width || dy > _mask.height)
    {
        if (_mask.pixels != nullptr)
        {
            delete [] _mask.pixels;
        }
        _mask.width = std::max((dx + 3) & ~3, _mask.width);
        _mask.height = std::max(dy, _mask.height);
        _mask.pixels = new uint8_t[_mask.width * _mask.height];
        LOG("reconfigured rendermask to size=%dx%d\n", _mask.width, _mask.height);
    }
}

void Graphics::clip(int &srcoffset, int &tgtoffset, int &size, int max)
{
    if (tgtoffset < 0)
    {
        size += tgtoffset;
        srcoffset -= tgtoffset;
        tgtoffset = 0;
    }
    if (tgtoffset + size > max)
    {
        size = max - tgtoffset;
    }
}

SFT_Font *Graphics::getfont(const char *fontname)
{
    SFT_Font *font = nullptr;
    auto  it = _fonts.find(fontname);
    if (it == _fonts.end())
    {
        font = loadfont(fontname);
        if (font == nullptr)
        {
            if (strrchr(fontname, '.') == nullptr)
            {
                font = loadfont((std::string(fontname) + ".ttf").c_str());
            } 
        }
        if (font != nullptr)
        {
            printf("loaded font='%s', size=%d @%p\n", fontname, font->size, font->memory);
            _fonts[fontname] = font;
        }
        else
        {
            printf("failed to load font='%s'\n", fontname);
        }
    }
    else
    {
        font = it->second;
    }
    return font;
}

SFT_Font *Graphics::loadfont(const char *fontname)
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
        auto memory = malloc(info.st_size);
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