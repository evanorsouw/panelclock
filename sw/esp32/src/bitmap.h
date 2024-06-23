#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <algorithm>
#include <cstdint>

#include "pixelsquare.h"
#include "copyjob.h"

class LedPanel;

class Bitmap : public PixelSquare
{
private:
    int _stride;
    uint8_t *_bitmap;

public:
    Bitmap(int dx, int dy) : PixelSquare(dx, dy)
    {
        _stride = ((dx * 3 + 3) / 4) * 4;
        _bitmap = new uint8_t[dy * stride()];
    }

    virtual ~Bitmap()
    {
        delete [] _bitmap;
        _bitmap = 0;
    }

    int stride() const { return _stride; }
    uint8_t *getPtr(int x, int y) { return _bitmap + (x * 3 + y * _stride); }

    void fill(int color)
    {
        uint8_t r = color >> 16;
        uint8_t g = color >> 8;
        uint8_t b = color;
        for (auto y = 0 ; y < dy(); ++y)
        {
            auto p = _bitmap + _stride * y;
            for (auto x = dx(); x-- > 0; )
            {
                *p++ = r;
                *p++ = g;
                *p++ = b;
            }
        }
    }

    void set(int x, int y, int rgb)
    {
        auto pt = getPtr(x, y);
        uint8_t *ps = (uint8_t*)&rgb;
        pt[0] = ps[0];
        pt[1] = ps[1];
        pt[2] = ps[2];
    }

    int get(int x, int y)
    {
        auto ps = getPtr(x, y);
        int color = 0;
        uint8_t *pt = (uint8_t*)&color;
        pt[0] = ps[0];
        pt[1] = ps[1];
        pt[2] = ps[2];
        return color;
    }

    void copyTo(Bitmap &tgt, int tgtx, int tgty)
    {
        CopyJob job(*this, tgt, tgtx, tgty);

        auto psrc = getPtr(job.srcx, job.srcy);
        auto ptgt = tgt.getPtr(job.tgtx, job.tgty);
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

#include "ledpanel.h"

void Bitmap::copyTo(LedPanel &tgt, int tgtx, int tgty)
{
    tgt.copyFrom(*this, tgtx, tgty);
}

#endif