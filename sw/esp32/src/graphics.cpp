
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

void Graphics::rect(Bitmap &tgt, float x, float y, float dx, float dy, Color color, Mode mode)
{
    if (!clip(x, dx, tgt.dx()) || !clip(y, dy, tgt.dy()))
        return;

    auto x2 = x + dx;
    auto y1 = (int)y;

    auto ay = y - TRUNC(y);
    if (ay > 0)
    {
        rectscanline(tgt, x, x2, y1++, 1 - ay, color, mode);
    }
    auto y2 = (int)(y + dy);
    while (y1 < y2)
    {
        rectscanline(tgt, x, x2, y1++, 1, color, mode);
    }
    ay = y + dy - y1;
    if (ay > 0)
    {
        rectscanline(tgt, x, x2, y1, ay, color, mode);
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

    auto xl = (int)std::min(px1, std::min(px2, std::min(px3, px4)));
    auto xr = (int)std::ceil(std::max(px1, std::max(px2, std::max(px3, px4))));
    auto yt = (int)std::min(py1, std::min(py2, std::min(py3, py4)));
    auto yb = (int)std::ceil(std::max(py1, std::max(py2, std::max(py3, py4))));
    clearRasterizedMask(irect(xl, yt, xr, yb));

    auto r1 = rasterizeTriangle(px1, py1, px2, py2, px3, py3);
    auto r2 = rasterizeTriangle(px2, py2, px3, py3, px4, py4);

    mergeRasterizedMask(tgt, color, r1.join(r2));
}

float Graphics::text(Bitmap &tgt, Font *font, float x, float y, const char *txt, Color color, Mode mode)
{    
    SFT_Image txtMask { .pixels = _rasterizeMask.getptr(0,0) };

    auto sft = font->getSFT();

    auto len = strlen(txt);
    for (int i=0; i<len; ++i)
    {
        SFT_Glyph glyph;
        sft_lookup(&sft, txt[i], &glyph);
        if (glyph == 0)
            continue;

        auto tx = (int)std::floor(x);
        auto ty = (int)std::floor(y);
        sft.xOffset = x - tx;
        sft.yOffset = -(y - ty);

        SFT_GMetrics mtx;
        sft_gmetrics(&sft, glyph, &mtx);
        tx += std::floor(mtx.leftSideBearing);
        ty = ty + mtx.yOffset + 1;

        auto sx = 0;
        auto sy = 0;
        auto dx = mtx.minWidth;
        auto dy = mtx.minHeight;
        clip(sx, tx, dx, tgt.dx());
        clip(sy, ty, dy, tgt.dy());
        if (_clipping)
        { 
            if (tx < _cliparea.x1)           
            {
                auto d = _cliparea.x1 - tx;
                tx += d;
                sx += d;
                dx -= d;
            }
            if (tx + dx > _cliparea.x2)
            {
                auto d = tx + dx - _cliparea.x2;
                dx -= d;                
            }
            if (ty < _cliparea.y1)           
            {
                auto d = _cliparea.y1 - ty;
                ty += d;
                sy += d;
                dy -= d;
            }
            if (ty + dy > _cliparea.y2)
            {
                auto d = ty + dy - _cliparea.y2;
                dy -= d;                
            }
        }

        if (dx > 0 && dy > 0)
        {
            txtMask.width = dx;
            txtMask.height = dy;
            sft_render(&sft, glyph, txtMask);

            auto pm = (uint8_t*)txtMask.pixels;
            for (auto iy=sy; iy < dy; iy++)
            {
                for (auto ix=sx; ix < dx; ++ix)
                {
                    auto alpha = pm[ix];
                    mode == Mode::Set
                        ? tgt.set((int)tx + ix, ty + iy, color, alpha)
                        : tgt.add((int)tx + ix, ty + iy, color, alpha);
                }
                pm += txtMask.width;
            }
        }
        x += mtx.advanceWidth;
    }
    return x;
}

void Graphics::ellipse(Bitmap &tgt, float x, float y, float dx, float dy, float thickness, Color color)
{

}

bool Graphics::clip(float &x, float &dx, float max)
{
    if (x < 0)
    {
        dx += x;
        x = 0;
    }
    if (x + dx > max)
    {
        dx = max - x;
    }
    return dx > 0.0f;
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

void Graphics::triangle(Bitmap &tgt, float x1, float y1, float x2, float y2, float x3, float y3, Color color)
{    
    auto xl = (int)std::min(x1, std::min(x2, x3));
    auto xr = (int)std::ceil(std::max(x1, std::max(x2, x3)));
    auto yt = (int)std::min(y1, std::min(y2, y3));
    auto yb = (int)std::ceil(std::max(y1, std::max(y2, y3)));
    clearRasterizedMask(irect(xl, yt, xr, yb));

    auto area = rasterizeTriangle(x1, y1, x2, y2, x3, y3);
    mergeRasterizedMask(tgt, color, area);
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

        trianglescanline(y, xl, xr, dy, dxl, dxr);    

        y = ynext;

    } while (y < ytop);
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
        trianglescanline(y, xl, xr, dy, dxl, dxr);

        y = ynext;
    } 
    while (y < ybase);
}

void Graphics::trianglescanline(float y, float xl, float xr, float dy, float dxl, float dxr)
{
    //printf("scanline: y=%f xl=%f, xr=%f, dy=%f, dxl=%f, dxr=%f\n", y, xl, xr, dy, dxl, dxr);
    float x = xl;
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
        uint8_t alpha = (uint8_t)(area * 255);
        _rasterizeMask.add(x, y, alpha);

        x = xnext;
    }
    while (x < xr);
}

void Graphics::rectscanline(Bitmap &tgt, float x1, float x2, int y, float ay, Color color, Mode mode)
{
    if (x1 > x2) SWAP(x1, x2);

    auto ix1 = (int)x1;
    auto ix2 = (int)x2;

    if (ix1 == ix2)
    {
        float ax = ix2 - ix2;
        mode == Mode::Set
            ? tgt.set(ix1, y, color, ALPHA(ax * ay))
            : tgt.add(ix1, y, color, ALPHA(ax * ay));
    }
    else
    {
        if (ix1 < x1)
        {
            mode == Mode::Set
                ? tgt.set(ix1, y, color, ALPHA((1 - x1 + ix1) * ay))
                : tgt.add(ix1, y, color, ALPHA((1 - x1 + ix1) * ay));
            ix1++;
        }
        while (ix1 < ix2)
        {
            mode == Mode::Set
                ? tgt.set(ix1++, y, color, ALPHA(ay))
                : tgt.add(ix1++, y, color, ALPHA(ay));
        }
        if (ix2 < x2)
        {
            mode == Mode::Set
                ? tgt.set(ix1, y, color, ALPHA((x2-ix2) * ay))
                : tgt.set(ix1, y, color, ALPHA((x2-ix2) * ay));
        }
    }
}

void Graphics::clearRasterizedMask(irect area)
{
    for (auto y=area.y1; y<=area.y2; ++y)
    {
        auto ptr = _rasterizeMask.getptr(0, y);
        for (auto x=area.x1; x<=area.x2; ++x)        
        {
            ptr[x] = 0;
        }
    }
}

void Graphics::mergeRasterizedMask(Bitmap &tgt, const Color color, irect area)
{
    for (int y=area.y1; y <= area.y2; ++y)
    {
        auto palpha = _rasterizeMask.getptr(0, y);
        for (int x=area.x1; x <= area.x2; ++x)
        {
            auto alpha = palpha[x];
            tgt.set(x, y, color, alpha);
        }
    }
}

