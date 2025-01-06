
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "bitmapfont.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "graphics.h"
#include "truetypefont.h"
#include "utf8encoding.h"

#if 0
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

Graphics::Graphics(int dx, int dy, bool origin)
{
    _dx = 0;
    _dy = 0;
    _rasterizeMask = new Bitmap(dx, dy, 1);
}

void Graphics::linkBitmap(Bitmap *bitmap)
{
    assert( bitmap->dx() == _rasterizeMask->dx() && bitmap->dy() == _rasterizeMask->dy());
    _drawingBitmap = bitmap;
    init(0, 0, bitmap->dx(), bitmap->dy(), true, true);
}

void Graphics::init(int x, int y, int dx, int dy, bool setoriginatxy, bool cliptoregion)
{
    dx = std::max(0, dx);
    dy = std::max(0, dy);
    _stride = _drawingBitmap->stride();

    if (setoriginatxy && cliptoregion)
    {   // provided (x,y) in bitmap are (0,0) for this Graphics instance
        // drawing is clipped to (dx,dy) or until bitmap edge
        _sx = std::max(0, -x);
        _sy = std::max(0, -y);
        _ptr = _drawingBitmap->getptr(x, y);
        _ex = std::min(dx, _rasterizeMask->dx() - x);
        _ey = std::min(dy, _rasterizeMask->dy() - y);
        //printf("x,y=%d,%d dx,dy=%d,%d => dx=%d,dy=%d, sx=%d,sy=%d ex=%d,ey=%d\n", x, y, dx, dy, dx, dy, _sx, _sy, _ex, _ey);
    }
    else if (!setoriginatxy && cliptoregion)
    {   // provided (x,y) in bitmap are (x,y) for this graphics instance
        // drawing is clipped to (dx, dy)
        _sx = std::max(0, x);
        _sy = std::max(0, y);
        _ptr = _drawingBitmap->getptr(0, 0);
        _ex = std::min(std::max(_sx, x + dx), _drawingBitmap->dx());
        _ey = std::min(std::max(_sy, y + dy), _drawingBitmap->dy());
        //printf("x,y=%d,%d dx,dy=%d,%d => sx=%d,sy=%d ex=%d,ey=%d\n", x, y, dx, dy, _sx, _sy, _ex, _ey);
    }
    else if (setoriginatxy && !cliptoregion)
    {   // provided (x,y) in bitmap are (0,0) for this Graphics instance
        // drawing is clipped to the parents region
        _sx = std::max(0, -x);
        _sy = std::max(0, -y);
        _ptr = _drawingBitmap->getptr(x, y);
        _ex = _sx + _rasterizeMask->dx();
        _ey = _sy + _rasterizeMask->dy();
        //printf("x,y=%d,%d dx,dy=%d,%d => dx=%d,dy=%d, sx=%d,sy=%d ex=%d,ey=%d\n", x, y, dx, dy, dx, dy, _sx, _sy, _ex, _ey);
    }
    else
    {
        assert(false);
    }
}

Graphics Graphics::offset(int x, int y)
{
    Graphics view;
    view._drawingBitmap = _drawingBitmap;
    view._rasterizeMask = _rasterizeMask;
    view.init(x, y, dx(), dy(), false, false);
    return view;
}

Graphics Graphics::view(int x, int y, int dx, int dy, bool origin)
{
    Graphics view;
    view._drawingBitmap = _drawingBitmap;
    view._rasterizeMask = _rasterizeMask;
    view.init(x, y, dx, dy, origin, true);
    return view;
}

void Graphics::add(int x, int y, const Color color, uint8_t alpha)
{
    auto pt = getptr(x, y);
    if (alpha == 255)
    {
        pt[0] = clip(pt[0] + color.r());
        pt[1] = clip(pt[1] + color.g());
        pt[2] = clip(pt[2] + color.b());
    }
    else if (alpha > 0)
    {
        pt[0] = clip(pt[0] + ((color.r() * alpha) >> 8));
        pt[1] = clip(pt[1] + ((color.g() * alpha) >> 8));
        pt[2] = clip(pt[2] + ((color.b() * alpha) >> 8));
    }
}

void Graphics::set(int x, int y, const Color color)
{
    auto pt = getptr(x, y);
    pt[0] = color.r();
    pt[1] = color.g();
    pt[2] = color.b();
}

void Graphics::set(int x, int y, const Color color, uint8_t alpha)
{
    if (alpha == 255)
    {
        set(x, y, color);
    }
    else if (alpha > 0)
    {
        auto pt = getptr(x, y);
        auto ialpha = (uint8_t)255 - alpha;
        pt[0] = clip((pt[0] * ialpha + color.r() * alpha) >> 8);
        pt[1] = clip((pt[1] * ialpha + color.g() * alpha) >> 8);
        pt[2] = clip((pt[2] * ialpha + color.b() * alpha) >> 8);
    }
}

void Graphics::rect(float x, float y, float dx, float dy, Color color, Mode mode)
{    
    if (!cliprange(x, dx, _sx, _ex) || !cliprange(y, dy, _sy, _ey))
        return;

    auto x2 = x + dx;
    auto y1 = (int)y;

    auto ay = y - TRUNC(y);
    if (ay > 0)
    {
        rectscanline(x, x2, y1++, 1 - ay, color, mode);
    }
    auto y2 = (int)(y + dy);
    while (y1 < y2)
    {
        rectscanline(x, x2, y1++, 1, color, mode);
    }
    ay = y + dy - y1;
    if (ay > 0)
    {
        rectscanline(x, x2, y1, ay, color, mode);
    }
}

void Graphics::rectscanline(float x1, float x2, int y, float ay, Color color, Mode mode)
{
    auto ix1 = (int)x1;
    auto ix2 = (int)x2;

    if (ix1 == ix2)
    {
        float ax = ix2 - ix2;
        mode == Mode::Set
            ? set(ix1, y, color, ALPHA(ax * ay))
            : add(ix1, y, color, ALPHA(ax * ay));
    }
    else
    {
        if (ix1 < x1)
        {
            mode == Mode::Set
                ? set(ix1, y, color, ALPHA((1 - x1 + ix1) * ay))
                : add(ix1, y, color, ALPHA((1 - x1 + ix1) * ay));
            ix1++;
        }
        while (ix1 < ix2)
        {
            mode == Mode::Set
                ? set(ix1++, y, color, ALPHA(ay))
                : add(ix1++, y, color, ALPHA(ay));
        }
        if (ix2 < x2)
        {
            mode == Mode::Set
                ? set(ix1, y, color, ALPHA((x2-ix2) * ay))
                : set(ix1, y, color, ALPHA((x2-ix2) * ay));
        }
    }
}

void Graphics::line(float x1, float y1, float x2, float y2, float thickness, Color color)
{
    auto dx = x2 - x1;
    auto dy = y2 - y1;

    auto px = -dy;
    auto py = dx;

    auto length = sqrt(px * px + py * py);
    if (length == 0.0f)
        return;

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

    auto xl = (int)std::min(px1, std::min(px2, std::min(px3, px4)));
    auto xr = (int)std::ceil(std::max(px1, std::max(px2, std::max(px3, px4))));
    auto yt = (int)std::min(py1, std::min(py2, std::min(py3, py4)));
    auto yb = (int)std::ceil(std::max(py1, std::max(py2, std::max(py3, py4))));

    clearRasterizedMask(xl, yt, xr, yb);

    auto r1 = rasterizeTriangle(px1, py1, px2, py2, px3, py3);
    auto r2 = rasterizeTriangle(px2, py2, px3, py3, px4, py4);

    mergeRasterizedMask(color, r1.join(r2));
}

float Graphics::text(Font *font, float x, float y, const char *txt, int n, Color color, Mode mode)
{    
    if (font->getFontType() == Font::FontType::TTF)
        return textTTF((TrueTypeFont*)font, x, y, txt, n, color, mode);
    return textWMF((BitmapFont*)font, x, y, txt, n, color, mode);
}

float Graphics::text(Font *font, float x, float y, char c, Color color, Mode mode)
{    
    char buf[2] = { c, 0 };
    return text(font, x, y, buf, color, mode);
}

float Graphics::textTTF(TrueTypeFont *font, float x, float y, const char *txt, int n, Color color, Mode mode)
{
    SFT_Image txtMask { .pixels = _rasterizeMask->getptr(0,0) };
    auto sft = font->getSFT();

    int codepoint;
    int idx = 0;
    while((codepoint = UTF8Encoding::nextCodepoint(txt, idx)) != 0)
    {
        SFT_Glyph glyph;
        sft_lookup(&sft, codepoint, &glyph);
        if (glyph == 0)
            continue;

        auto tx = (int)std::floor(x);
        auto ty = (int)std::floor(y);
        sft.xOffset = x - tx;
        sft.yOffset = -(y - ty);

        SFT_GMetrics mtx;
        sft_gmetrics(&sft, glyph, &mtx);
        tx += std::floor(mtx.leftSideBearing);
        ty += mtx.yOffset + 1;

        auto sx = 0;
        auto sy = 0;
        auto dx = mtx.minWidth;
        auto dy = mtx.minHeight;
        if (cliprange(sx, tx, dx, _sx, _ex) && cliprange(sy, ty, dy, _sy, _ey))
        {
            txtMask.width = mtx.minWidth;
            txtMask.height = mtx.minHeight;
            sft_render(&sft, glyph, txtMask);

            auto pm = (uint8_t*)txtMask.pixels + sx + sy * txtMask.width;
            for (auto iy=0; iy < dy; iy++)
            {
                for (auto ix=0; ix < dx; ++ix)
                {
                    auto alpha = pm[ix];
                    mode == Mode::Set
                        ? set(tx + ix, ty + iy, color, alpha)
                        : add(tx + ix, ty + iy, color, alpha);
                }
                pm += txtMask.width;
            }
        }
        x += mtx.advanceWidth;

        if (n > 0 && n-- == 1)
            break;
    }
    return x;
}

float Graphics::textWMF(BitmapFont *font, float x, float y, const char *txt, int n, Color color, Mode mode)
{
    int codepoint;
    int idx = 0;
    while((codepoint = UTF8Encoding::nextCodepoint(txt, idx)) != 0)
    {
        auto glyph = font->getGlyph(codepoint);
        if (glyph == nullptr)
            continue;

        auto sx = 0;
        auto sy = 0;
        auto tx = (int)std::floor(x);
        auto ty = (int)std::floor(y - glyph->height - glyph->baselineOffset);
        auto dx = (int)std::floor(glyph->width);
        auto dy = (int)std::floor(glyph->height);

        if (cliprange(sx, tx, dx, _sx, _ex) && cliprange(sy, ty, dy, _sy, _ey))
        {
            auto stride = (glyph->width + 7) / 8;
            auto pb = &glyph->glyphData1 + sy * stride + sx / 8;

            for (auto iy=0; iy < dy; iy++)
            {
                for (auto ix=0; ix < dx; ++ix)
                {
                    auto ib = sx + ix;
                    auto bit = pb[ib/8] & (0x80 >> (ib&7));
                    if (bit != 0)
                    {
                        mode == Mode::Set
                        ? set(tx + ix, ty + iy, color)
                        : add(tx + ix, ty + iy, color, 255);
                    }
                }
                pb += stride;
            }
        }
        x += glyph->width + font->tracking();

        if (n > 0 && n-- == 1)
            break;
    }
    return x;
}

bool Graphics::cliprange(int &srcx, int &tgtx, int &tgtdx, int min, int max)
{
    if (tgtx < min)
    {
        auto d = min - tgtx;
        srcx += d;
        tgtdx -= d;
        tgtx = min;
    }
    if (tgtx + tgtdx > max)
    {
        tgtdx = max - tgtx;
    }
    return tgtdx > 0.0f;
}

void Graphics::triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color)
{    
    auto xl = (int)std::min(x1, std::min(x2, x3));
    auto xr = (int)std::ceil(std::max(x1, std::max(x2, x3)));
    auto yt = (int)std::min(y1, std::min(y2, y3));
    auto yb = (int)std::ceil(std::max(y1, std::max(y2, y3)));
    clearRasterizedMask(xl, yt, xr, yb);

    auto area = rasterizeTriangle(x1, y1, x2, y2, x3, y3);
    mergeRasterizedMask(color, area);
}

irect Graphics::rasterizeTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
    LOG("rasterizeTriangle(x1=%.2f,y1=%.2f,x2=%.2f,y2=%.2f,x3=%.2f,y3=%.2f)\n",x1,y1,x2,y2,x3,y3);

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
        return irect();
    if (x1 == x2 && x1 == x3)
        return irect();

    if (y1 == y2)
    {
        triangleBaseTop(x1, x2, y1, x3, y3);
    }
    else if (y2 == y3)
    {
        triangleBaseBottom(x1, y1, x2, x3, y2);
    }
    else
    {
        float dy12 = y2 - y1;
        float dy13 = y3 - y1;
        float mx = x1 + (x3 - x1) * dy12 / dy13;
        triangleBaseBottom(x1, y1, x2, mx, y2);
        triangleBaseTop(x2, mx, y2, x3, y3);
    }
    int xl = (int)std::min(x1,std::min(x2,x3));
    int xr = (int)(std::ceil(std::max(x1,std::max(x2,x3))));
    int yt = (int)(std::min(y1,std::min(y2,y3)));
    int yb = (int)(std::ceil(std::max(y1,std::max(y2,y3))));
    return irect(xl, yt, xr, yb);
}

void Graphics::triangleBaseTop(float xbase1, float xbase2, float ybase, float xtop, float ytop)
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
        if (!(y < _sy) && y < _ey)
        {
            trianglescanline(y, xl, xr, dy, dxl, dxr);    
        }
        y = ynext;
    } 
    while (y < ytop);
    LOG("triangleBaseTop() - done\n");
}

void Graphics::triangleBaseBottom(float xtop, float ytop, float xbase1, float xbase2, float ybase)
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
        if (!(y < _sy) && y < _ey)
        {
            trianglescanline(y, xl, xr, dy, dxl, dxr);
        }
        y = ynext;
    } 
    while (y < ybase);
    LOG("triangleBaseBottom() - done\n");
}

void Graphics::trianglescanline(float y, float xl, float xr, float dy, float dxl, float dxr)
{
    float x = std::max((float)_sx, xl);
    float ex = std::min((float)_ex, xr);
    auto ptr = _rasterizeMask->getptr(0, y);
    if (x > ex)
        return;
    LOG("tri-scanline x=%d,y=%d,dx=%.2f => p=%p\n", (int)x, (int)y, ex-x, ptr);
    do
    {
        float xnext = MIN(xr, TRUNC(x + 1));
        float range = dxl == 0 ? 999 : dy / dxl;
        float yl1 = ((x - xl) * range);
        float yl2 = ((xnext - xl) * range);
        float ayl = MIN(dy, (yl1 + yl2) / 2);

        range = dxr == 0 ? 999 : dy / dxr;
        float yr1 = ((xr - x) * range);
        float yr2 = ((xr - xnext) * range);
        float ayr = MIN(dy, (yr1 + yr2) / 2);

        float axl = MIN(1, xnext - x);
        float axr = MIN(1, xr - x);

        float area = MIN(axl, axr) * MIN(ayl, ayr);
        ptr[(int)x] = clip(ptr[(int)x] + (uint8_t)(area * 255));
        x = xnext;
    }
    while (x < ex);
}

void Graphics::clearRasterizedMask(int xl, int yt, int xr, int yb)
{
    clip(xl, 0, _rasterizeMask->dx() - 1);
    clip(xr, 0, _rasterizeMask->dx() - 1);
    clip(yt, 0, _rasterizeMask->dy() - 1);
    clip(yb, 0, _rasterizeMask->dy() - 1);

    for (auto y=yt; y<yb; ++y)
    {
        auto ptr = _rasterizeMask->getptr(xl, y);
        std::memset(ptr, 0, xr - xl);
    }
}

void Graphics::mergeRasterizedMask(const Color color, irect area)
{
    clip(area.x1, 0, _rasterizeMask->dx() - 1);
    clip(area.x2, 0, _rasterizeMask->dx() - 1);
    clip(area.y1, 0, _rasterizeMask->dy() - 1);
    clip(area.y2, 0, _rasterizeMask->dy() - 1);

    for (int y=area.y1; y < area.y2; ++y)
    {
        auto palpha = _rasterizeMask->getptr(0, y);
        for (int x=area.x1; x < area.x2; ++x)
        {
            set(x, y, color, palpha[x]);
        }
    }
}
