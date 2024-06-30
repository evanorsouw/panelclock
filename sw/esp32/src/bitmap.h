#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <assert.h>
#include <algorithm>
#include <cstdint>
#include <cstring>

#include "color.h"
#include "copyjob.h"
#include "pixelsquare.h"

class LedPanel;

class Bitmap : public PixelSquare
{
private:
    int _bpp;
    int _stride;
    int _bitmapsize;
    uint8_t *_bitmap;

public:
    Bitmap(int dx, int dy, int bpp) 
        : PixelSquare(dx, dy) 
    {
        assert( dx > 0 && dy > 0);
        assert( bpp == 1 || bpp == 3);

        _bpp = bpp;
        _stride = ((dx * bpp + 3) / 4) * 4;
        _bitmapsize = dy * _stride;
        _bitmap = (uint8_t*)malloc(_bitmapsize);
        printf("Bitmap(%d,%d,%d): allocated %d bytes @%p\n", dx, dy, bpp, _bitmapsize, _bitmap);
        assert( _bitmap != nullptr );
    }

    virtual ~Bitmap()
    {
        printf("Bitmap: freed %d bytes\n", _bitmapsize);
        free(_bitmap);
        _bitmap = 0;
    }

    int stride() const { return _stride; }

    uint8_t *getptr() const { return _bitmap; }
    uint8_t *getptr(int x, int y) const { return _bitmap + (x * _bpp + y * _stride); }

    void fill(const Color &color)
    {
        if (_bpp == 1 || color.isgray())
        {
            std::memset(_bitmap, color.b(), _bitmapsize);
        }
        else 
        {
            for (auto y=0; y < dy(); ++y)
            {
                auto p = getptr(0, y);
                for (auto x=dx(); x-- > 0; )
                {
                    *p++ = color.r();
                    *p++ = color.g();
                    *p++ = color.b();
                }                
            }
        }
    }

    void set(int x, int y, const Color color)
    {
        auto pt = getptr(x, y);
        if (_bpp == 1)
        {
            *pt = color.b();
        }
        else
        {
            pt[0] = color.r();
            pt[1] = color.g();
            pt[2] = color.b();
        }
    }

    void set(int x, int y, const Color color, uint8_t alpha)
    {
        if (alpha == 255)
        {
            set(x, y, color);
        }
        else
        {
            assert( _bpp = 3 );

            auto pt = getptr(x, y);
            pt[0] = (pt[0] * (255 - alpha) + color.r() * alpha) / 256;
            pt[1] = (pt[1] * (255 - alpha) + color.g() * alpha) / 256;
            pt[2] = (pt[2] * (255 - alpha) + color.b() * alpha) / 256;
        }
    }

    Color get(int x, int y)
    {
        auto ps = getptr(x, y);
        if (_bpp == 1)
            return *ps;
        return Color(ps[0], ps[1], ps[2]);
    }

    void copyTo(Bitmap &tgt, int tgtx, int tgty)
    {
        CopyJob job(*this, tgt, tgtx, tgty);

        auto psrc = getptr(job.srcx, job.srcy);
        auto ptgt = tgt.getptr(job.tgtx, job.tgty);
        auto copyx = job.dx * 3;

        while (job.dy-- > 0)
        {
            std::copy(psrc, psrc + copyx, ptgt);
            psrc += stride();
            ptgt += tgt.stride();
        }
    }

    void copyTo(LedPanel &tgt, int tgtx, int tgty);
};

#endif