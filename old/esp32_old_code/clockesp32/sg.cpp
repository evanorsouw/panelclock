
#include <iostream>
#include <functional>
#include <cmath>
#include <algorithm>
#include "sg.h"
#include "display.h"
#include "linetracker.h"
#include "schrift.h"

Color Color::transparant = Color(0x00000000);
Color Color::black = Color(0xFF000000);
Color Color::white = Color(0xFFFFFFFF);
Color Color::red = Color(0xFFFF0000);
Color Color::lime = Color(0xFF00FF00);
Color Color::blue = Color(0xFF0000FF);
Color Color::yellow = Color(0xFFFFFF00);
Color Color::cyan = Color(0xFF00FFFF);
Color Color::magenta = Color(0xFFFF00FF);
Color Color::silver = Color(0xFFC0C0C0);
Color Color::gray = Color(0xFF808080);
Color Color::maroon = Color(0xFF800000);
Color Color::olive = Color(0xFF808000);
Color Color::green= Color(0xFF008000);
Color Color::purple = Color(0xFF800080);
Color Color::teal = Color(0xFF008080);
Color Color::navy = Color(0xFF000080);
Color Color::brown = Color(0xFFA52A2A);
Color Color::orangered = Color(0xFFFF4500);
Color Color::skyblue = Color(0xFF87CEEB);

Bitmap* Bitmap::create(int dx, int dy, unsigned char bytesperpixel, unsigned int flags = FLG_COMPACT)
{
    if (dx < 0 || dy < 0)
        return nullptr;

    if (bytesperpixel != 3 && bytesperpixel != 1)
        return nullptr;     // only 24 bpp supported for now

    auto stride = dx * bytesperpixel;
    if (flags & FLG_COMPACT)
    {
        stride = ((stride + 3) / 4) * 4;
    }

    auto bitmapsize = stride * dy;
    auto buf = new unsigned char[bitmapsize];
    if (buf == nullptr)
        return nullptr;

    auto instance = new Bitmap();
    instance->_ptr = buf;
    instance->_maxptr = buf + bitmapsize;
    instance->_bytesperpixel = bytesperpixel;
    instance->_stride = stride;
    instance->_width = dx;
    instance->_height = dy;

    return instance;
}

void Bitmap::clear(Color color)
{
    if (color.r() == color.g() && color.r() == color.b())
    {
        memset(_ptr, color.r(), bitmapsize());
    }
    else
    {
        auto p = pixptr();
        for (int n = _width * _height; n-- > 0; )
        {
            p.setright(color);
        }
    }
}

PixelPtr Bitmap::pixptr() const 
{ 
    return PixelPtr(this); 
}

PixelPtr Bitmap::pixptr(int x, int y) const 
{ 
    return PixelPtr(ptr(x, y), this); 
}

struct scanline
{
    int y, x1, x2, x3, x4, i1, i23, i4;
};

class Bresenham
{
private:
    int _x;
    int _y;
    int _sx;
    int _sy;
    int _dx;
    int _dy;
    int _err;
    int _n;

public:
    Bresenham(int x1, int y1, int x2, int y2, int n = 1)
    {
        _x = x1;
        _y = y1;
        _dx = std::abs(x2 - x1) * n;
        _dy = -std::abs(y2 - y1) * n;
        _sx = x1 < x2 ? n : -n;
        _sy = y1 < y2 ? n : -n;
        _err = _dx + _dy;
        _n = std::max(_dx, abs(_dy)) / n;
    }

    int x() const { return _x; }
    int y() const { return _y; }

    bool step()
    {
        if (_n == 0)
            return false;
        _n--;
        int e = 2 * _err;
        if (e >= _dy)
        {
            _err += _dy;
            _x += _sx;
        }
        if (e <= _dx)
        {
            _err += _dx;
            _y += _sy;
        }
        return true;
    }
};

void Graphics::line1(int x0, int y0, int x1, int y1, Color color)
{
    Bresenham b(x0, y0, x1, y1);

    do
    {
        _bitmap->setpixel(b.x(), b.y(), color);
    } while (b.step()); 
}

Graphics& Graphics::setfont(float fontsize, Color color)
{
    _activeFont = getfont("d:\\download\\FiraGO-Regular.ttf");
    _activeFontSize = fontsize;
    _activeTextColor = color;
    return *this;
}

void Graphics::text(float x, float y, const std::string& txt)
{
    SFT sft;
    sft.font = _activeFont;
    sft.xScale = _activeFontSize;
    sft.yScale = _activeFontSize;
    sft.flags = SFT_DOWNWARD_Y;

    auto txtmask = Bitmap::create(_bitmap->width(), _bitmap->height(), 1);

    SFT_Image sftmask;
    sftmask.width = txtmask->width();
    sftmask.height = txtmask->height();
    sftmask.pixels = txtmask->ptr();

    SFT_LMetrics lmtx;
    sft_lmetrics(&sft, &lmtx); 

    auto color = _activeTextColor;
    for (int i=0; i<txt.size(); ++i)
    {
        SFT_Glyph glyph;
        sft_lookup(&sft, txt[i], &glyph);

        sft.xOffset = x;
        sft.yOffset = -y;

        SFT_GMetrics gmtx;
        sft_gmetrics(&sft, glyph, &gmtx);

        if (gmtx.minWidth <= sftmask.width && gmtx.minHeight < sftmask.height)
        {
            sft_render(&sft, glyph, sftmask);

            int bx1 = gmtx.leftSideBearing;
            int by1 = gmtx.yOffset + lmtx.ascender - lmtx.descender - sft.yScale / 2;
            int bx2 = bx1 + gmtx.minWidth;
            int by2 = by1 + gmtx.minHeight;
            int mx1 = 0;
            int my1 = 0;
            int mx2 = bx2 - bx1;
            int my2 = by2 - by1;
            int dx = bx2 - bx1;
            int dy = by2 - by1;

            if (bx1 < 0)
            {
            mx1 -= bx1;
            dx += bx1;
            bx1 = 0;
            }
            else if (bx2 > _bitmap->width())
            {
            dx -= bx2 - _bitmap->width();
            }
            if (by1 < 0)
            {
                my1 -= by1;
                dy += by1;
                by1 = 0;
            }
            else if (by2 > _bitmap->height())
            {
                dy -= by2 - _bitmap->height();
            }

            for (int iy = 0; iy < dy; ++iy)
            {
                unsigned char* pm = txtmask->ptr(mx1, my1 + iy);
                unsigned char* pb = _bitmap->ptr(bx1, by1 + iy);
                for (int ix = 0; ix < dx; ++ix)
                {
                    _bitmap->blendpixel(pb, color.a(pm[ix]));
                    pb += 3;
                }
            }
        }
        x += gmtx.advanceWidth;
    }

    delete txtmask;
}

SFT_Font* Graphics::getfont(const std::string& path)
{
    SFT_Font* font;
    auto it = _fonts.find(path);
    if (it == _fonts.end())
    {
        font = sft_loadfile("d:\\download\\FiraGO-Regular.ttf");
        if (font)
        {
            _fonts[path] = font;
        }
    }
    else
    {
        font = it->second;
    }
    return font;
}

#define B    4              // number of bits for fraction
#define S    (1<<B)         // number to add for integer + 1
#define M    (S-1)          // mask for fraction
#define F(x) ((x)&M)        // fraction part of number
#define I(x) ((x)>>B)       // integer part of number
#define N(x) (M-(x))        // invert number by mask, e.g. 0->15, 1->14, ...


void Graphics::line(float xs, float ys, float xe, float ye, Color color, float thickness, display* disp)
{
    typedef struct { float x; float y; } coor2;
    coor2 xy[4];
    float x1, y1, x2, y2, x3, y3, x4, y4;

    {
        float dx = (xe - xs);
        float dy = (ye - ys);
        float len = std::sqrt(dx * dx + dy * dy);
        float sx = dx / len;
        float sy = dy / len;

        xy[0].x = xs - sy * thickness / 2;
        xy[0].y = ys + sx * thickness / 2;
        xy[1].x = xs + sy * thickness / 2;
        xy[1].y = ys - sx * thickness / 2;
        xy[2].x = xe + sy * thickness / 2;
        xy[2].y = ye - sx * thickness / 2;
        xy[3].x = xe - sy * thickness / 2;
        xy[3].y = ye + sx * thickness / 2;
        std::sort(std::begin(xy), std::end(xy), [](coor2 a, coor2 b) { return a.y == b.y ? a.x < b.x : a.y < b.y; });

        x2 = xy[0].x;
        y2 = xy[0].y;
        if (xy[1].x < xy[2].x)
        {
            x1 = xy[1].x;
            y1 = xy[1].y;
            x3 = xy[2].x;
            y3 = xy[2].y;
        }
        else
        {
            x1 = xy[2].x;
            y1 = xy[2].y;
            x3 = xy[1].x;
            y3 = xy[1].y;
        }
        x4 = xy[3].x;
        y4 = xy[3].y;
    }

        //_bitmap->setpixel((int)xy[0].x, (int)xy[0].y, Color::lime);
        //_bitmap->setpixel((int)xy[1].x, (int)xy[1].y, Color::blue);
        //_bitmap->setpixel((int)xy[2].x, (int)xy[2].y, Color::green);
        //_bitmap->setpixel((int)xy[3].x, (int)xy[3].y, Color::orangered);
        //disp->send(_bitmap);

    LineTracker tl(x2, y2, x1, y1, x4, y4, false);
    LineTracker tr(x2, y2, x3, y3, x4, y4, true);
    scanpoint pla, plb, pra, prb;

    tl.next(&pla);
    tr.next(&pra);

    bool show = false;

    while (tl.next(&plb) && tr.next(&prb))
    {
        int dx = std::abs(pla.x - pra.x) + 1;
        int intensity;
        if (dx == 1)
        {
            intensity = ((pra.ix - pla.ix) * pla.iy) >> 8;
            _bitmap->blendpixel(pla.x, pla.y, color.a(intensity));
            if (show) disp->send(_bitmap);
        }
        else if (dx == 2)
        {
            intensity = (pla.ix * pla.iy) >> 8;
            _bitmap->blendpixel(pla.x, pla.y, color.a(intensity));
            if (show) disp->send(_bitmap);
            intensity = (pra.ix * pra.iy) >> 8;
            _bitmap->blendpixel(pra.x, pla.y, color.a(intensity));
            if (show) disp->send(_bitmap);
        }
        else if (pla.x >= plb.x)
        {
            intensity = (pla.ix * pla.iy) >> 8;
            _bitmap->blendpixel(pla.x++, pla.y, color.a(intensity));
            if (show) disp->send(_bitmap);
            while (pla.x < pra.x)
            {
                _bitmap->blendpixel(pla.x++, pla.y, color.a(255));
                if (show) disp->send(_bitmap);
            }
            if (pla.x <= pra.x)
            {
                intensity = (pra.ix * pra.iy) >> 8;
                _bitmap->blendpixel(pra.x, pra.y, color.a(intensity));
                if (show) disp->send(_bitmap);
            }
        }
        else
        {
            intensity = (pla.ix * pla.iy) >> 8;
            _bitmap->blendpixel(pla.x++, pla.y, color.a(intensity));
            if (show) disp->send(_bitmap);
            dx = plb.x - pla.x;
            for (int i = 1; i < dx; ++i)
            {
                intensity = ((pla.ix + (255 - pla.ix) * i / dx) * pla.iy) >> 8;
                _bitmap->blendpixel(pla.x++, pla.y, color.a(intensity));
                if (show) disp->send(_bitmap);
            }
            while (pla.x < pra.x)
            {
                _bitmap->blendpixel(pla.x++, pla.y, color.a(255));
                if (show) disp->send(_bitmap);
            }
            if (pla.x <= pra.x)
            {
                intensity = (pra.ix * pra.iy) >> 8;
                _bitmap->blendpixel(pra.x, pra.y, color.a(intensity));
                if (show) disp->send(_bitmap);
            }
        }
        pla = plb;
        pra = prb;
        if (show) disp->send(_bitmap);
    }
}

