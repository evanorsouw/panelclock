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
#include <esp_heap_caps.h>

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

        printf("total free DRAM: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        printf("total free IRAM: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
        printf("total free DMA: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_largest_free_block(MALLOC_CAP_DMA));

        assert( _bitmap != nullptr );
    }

    virtual ~Bitmap()
    {
        printf("Bitmap: freed %d bytes\n", _bitmapsize);
        free(_bitmap);
        _bitmap = 0;

        printf("total free DRAM: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        printf("total free IRAM: %d (largest block: %d)\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));    
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

    void add(int x, int y, uint8_t alpha)
    {
        assert( _bpp == 1 );
        auto pt = getptr(x, y);
        pt[0] = clip(pt[0] + alpha);
    }

    void add(int x, int y, const Color color, uint8_t alpha)
    {
        assert( _bpp == 3 );
        auto pt = getptr(x, y);
        if (alpha == 255)
        {
            pt[0] = clip(pt[0] + color.r());
            pt[1] = clip(pt[1] + color.g());
            pt[2] = clip(pt[2] + color.b());
        }
        else
        {
            pt[0] = clip(pt[0] + ((color.r() * alpha) >> 8));
            pt[1] = clip(pt[1] + ((color.g() * alpha) >> 8));
            pt[2] = clip(pt[2] + ((color.b() * alpha) >> 8));
        }
    }

    void set(int x, int y, const Color color)
    {
        assert( _bpp == 3);
        auto pt = getptr(x, y);
        pt[0] = color.r();
        pt[1] = color.g();
        pt[2] = color.b();
    }

    void set(int x, int y, const Color color, uint8_t alpha)
    {
        if (alpha == 255)
        {
            set(x, y, color);
        }
        else if (alpha > 0)
        {
            assert( _bpp = 3 );

            auto pt = getptr(x, y);
            auto ialpha = (uint8_t)255 - alpha;
            pt[0] = clip((pt[0] * ialpha + color.r() * alpha) >> 8);
            pt[1] = clip((pt[1] * ialpha + color.g() * alpha) >> 8);
            pt[2] = clip((pt[2] * ialpha + color.b() * alpha) >> 8);
        }
    }

    Color get(int x, int y)
    {
        assert( _bpp == 3 );
        auto ps = getptr(x, y);
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
    uint8_t clip(int v) { return v > 255 ? 255 : v; };
};

#endif