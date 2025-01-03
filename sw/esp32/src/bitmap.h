#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <assert.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstring>

#include "color.h"
#include "copyjob.h"
#include "pixelsquare.h"
#include "diagnostic.h"

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
        Diagnostic::printmeminfo();

        assert( _bitmap != nullptr );
    }

    Bitmap(Bitmap &parent, int ox, int oy, int dx, int dy)
        : PixelSquare(
            std::min(std::max(0, parent.dx() - ox), dx), 
            std::min(std::max(0, parent.dy() - oy), dy))
    {
        _bpp = parent._bpp;
        _stride = parent._stride;
        _bitmap = parent.getptr(ox, oy);
    }


    virtual ~Bitmap()
    {
        printf("Bitmap: freed %d bytes\n", _bitmapsize);
        free(_bitmap);
        _bitmap = 0;
        Diagnostic::printmeminfo();
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