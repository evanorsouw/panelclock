
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
    auto x2 = x + dx;
    auto y1 = (int)y;

    auto ay = y - TRUNC(y);
    if (ay > 0)
    {
        rectscanline(tgt, x, x2, y1++, 1 - ay, color);
    }
    auto y2 = (int)(y + dy);
    while (y1 < y2)
    {
        rectscanline(tgt, x, x2, y1++, 1, color);
    }
    ay = y + dy - y1;
    if (ay > 0)
    {
        rectscanline(tgt, x, x2, y1, ay, color);
    }
}

void Graphics::line(Bitmap &tgt, float x1, float y1, float x2, float y2, float thickness, Color color)
{
    auto dx = x2 - x1;
    auto dy = y2 - y1;

    auto px = -dy;
    auto py = dx;

    auto length = sqrt(px * px + py * py);

    auto upx = px / length;
    auto upy = py / length;

    auto w = thickness / 2;

    auto px1 = x1 + w * upx;
    auto py1 = y1 + w * upy;
    auto px2 = x1 - w * upx;
    auto py2 = y1 - w * upy;
    auto px3 = x2 + w * upx;
    auto py3 = y2 + w * upy;
    auto px4 = x2 - w * upx;
    auto py4 = y2 - w * upy;

    triangle(tgt, px1, py1, px2, py2, px3, py3, color);
    triangle(tgt, px2, py2, px3, py3, px4, py4, color);
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

textinfo Graphics::textsize(const char *txt)
{
    SFT sft;
    sft.font = _activeFont;
    sft.xScale = _activeFontSizeX;
    sft.yScale = _activeFontSizeY;
    sft.flags = SFT_DOWNWARD_Y;

    SFT_LMetrics lmtx;
    sft_lmetrics(&sft, &lmtx); 

    struct textinfo size = { 
        .dy = lmtx.ascender + lmtx.descender,
        .ybase = lmtx.ascender };

    auto len = strlen(txt);
    for (int i=0; i<len; ++i)
    {
        SFT_Glyph glyph;
        sft_lookup(&sft, txt[i], &glyph);
        if (glyph == 0)
            continue;

        SFT_GMetrics mtx;
        sft_gmetrics(&sft, glyph, &mtx);

        size.dx += mtx.advanceWidth;
    }
    return size;
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

void Graphics::triangle(Bitmap &tgt, float x1, float y1, float x2, float y2, float x3, float y3, Color color)
{    
    LOG("triangle(x1=%.2f,y1=%.2f,x2=%.2f,y2=%.2f,x3=%.2f,y3=%.2f)\n",x1,y1,x2,y2,x3,y3);
    if (y2 < y1)
    {
        SWAP(y1, y2);
        SWAP(x1, x2);
    }
    if (y3 < y1)
    {
        SWAP(y1, y3);
        SWAP(x1, x3);
    }
    if (y3 < y2)
    {
        SWAP(y2, y3);
        SWAP(x2, x3);
    }

    if (y1 == y2 && y1 == y3)
        return;
    if (x1 == x2 && x1 == x3)
        return;

    if (y1 == y2)
    {
        triangleBaseTop(tgt, x1, x2, y1, x3, y3, color);
    }
    else if (y2 == y3)
    {
        triangleBaseBottom(tgt, x1, y1, x2, x3, y2, color);
    }
    else
    {
        float dy12 = y2 - y1;
        float dy13 = y3 - y1;
        float mx = x1 + (x3 - x1) * dy12 / dy13;
        triangleBaseBottom(tgt, x1, y1, x2, mx, y2, color);
        triangleBaseTop(tgt, x2, mx, y2, x3, y3, color);
    }
}

void Graphics::triangleBaseTop(Bitmap &tgt, float xbase1, float xbase2, float ybase, float xtop, float ytop, Color color)
{
    LOG("triangleBaseTop(xb1=%.2f,xb2=%.2f,yb=%.2f,xt=%.2f,yt=%.2f)\n",xbase1,xbase2,ybase,xtop,ytop);

    if (xbase1 > xbase2) SWAP(xbase1, xbase2);

    float y = ybase;
    float height = ytop - ybase;

    float dx_a = (xbase1 - xtop) / height;
    float dx_b = (xbase2 - xtop) / height;
    do
    {
        float ynext = MIN(TRUNC(y + 1), ytop);

        float dy = ytop - y;
        float xtl = xtop + dy * dx_a;
        float xtr = xtop + dy * dx_b;

        dy = ytop - ynext;
        float xbl = xtop + dy * dx_a;
        float xbr = xtop + dy * dx_b;

        float dxl = ABS(xbl - xtl);
        float dxr = ABS(xbr - xtr);
        float xl = MIN(xtl, xbl);
        float xr = MAX(xtr, xbr);
        dy = ynext - y;

        trianglescanline(tgt, y, xl, xr, dy, dxl, dxr, color);    

        y = ynext;

    } while (y < ytop);
}

void Graphics::triangleBaseBottom(Bitmap &tgt, float xtop, float ytop, float xbase1, float xbase2, float ybase, Color color)
{
    LOG("triangleBaseBottom(xt=%.2f,yt=%.2f,xb1=%.2f,xb2=%.2f,yb=%.2f)\n", xtop,ytop,xbase1,xbase2,ybase);

    if (xbase1 > xbase2) SWAP(xbase1, xbase2);

    float y = ytop;
    float height = ybase - ytop;

    float dx_a = (xbase1 - xtop) / height;
    float dx_b = (xbase2 - xtop) / height;
    do
    {
        float ynext = MIN(TRUNC(y + 1), ybase);

        float dy = y - ytop;
        float xtl = xtop + dy * dx_a;
        float xtr = xtop + dy * dx_b;

        dy = ynext - ytop;
        float xbl = xtop + dy * dx_a;
        float xbr = xtop + dy * dx_b;

        float dxl = ABS(xbl - xtl);
        float dxr = ABS(xbr - xtr);
        float xl = MIN(xtl, xbl);
        float xr = MAX(xtr, xbr);

        dy = ynext - y;
        trianglescanline(tgt, y, xl, xr, dy, dxl, dxr, color);

        y = ynext;
    } 
    while (y < ybase);
}

void Graphics::trianglescanline(Bitmap &tgt, float y, float xl, float xr, float dy, float dxl, float dxr, Color color)
{
    float x = xl;
    do
    {
        float xnext = MIN(xr, TRUNC(x + 1));

        float range = dy / dxl;
        float yl1 = ((x - xl) * range);
        float yl2 = ((xnext - xl) * range);
        float ayl = MIN(dy, (yl1 + yl2) / 2);

        range = dy / dxr;
        float yr1 = ((xr - x) * range);
        float yr2 = ((xr - xnext) * range);
        float ayr = MIN(dy, (yr1 + yr2) / 2);

        float axl = MIN(1, xnext - x);
        float axr = MIN(1, xr - x);

        float area = MIN(axl, axr) * MIN(ayl, ayr);
        uint8_t alpha = (uint8_t)(area * 255);

        tgt.set(x, y, color, alpha);

        x = xnext;
    }
    while (x < xr);
}

void Graphics::rectscanline(Bitmap &tgt, float x1, float x2, int y, float ay, Color color)
{
    if (x1 > x2) SWAP(x1, x2);

    auto ix1 = (int)x1;
    auto ix2 = (int)x2;

    if (ix1 == ix2)
    {
        float ax = ix2 - ix2;
        tgt.set(ix1, y, color, ALPHA(ax * ay));
    }
    else
    {
        if (ix1 < x1)
        {
            tgt.set(ix1, y, color, ALPHA((1 - x1 + ix1) * ay));
            ix1++;
        }
        while (ix1 < ix2)
        {
            tgt.set(ix1++, y, color, ALPHA(ay));
        }
        if (ix2 < x2)
        {
            tgt.set(ix1, y, color, ALPHA((x2-ix2) * ay));
        }
    }
}

