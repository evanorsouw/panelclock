
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <assert.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdio.h>

#include "bitmap.h"
#include "diagnostic.h"

Bitmap::Bitmap(int dx, int dy, int bpp) 
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

Bitmap::Bitmap(Bitmap &parent, int ox, int oy, int dx, int dy)
    : PixelSquare(
        std::min(std::max(0, parent.dx() - ox), dx), 
        std::min(std::max(0, parent.dy() - oy), dy))
{
    _bpp = parent._bpp;
    _stride = parent._stride;
    _bitmap = parent.getptr(ox, oy);
}

Bitmap::~Bitmap()
{
    printf("Bitmap: freed %d bytes\n", _bitmapsize);
    free(_bitmap);
    _bitmap = 0;
    Diagnostic::printmeminfo();
}

void Bitmap::restructure(int dx, int dy)
{
    _stride = ((dx * _bpp + 3) / 4) * 4;
    _dx = dx;
    _dy = dy;
    if (_stride * dy > _bitmapsize)
    {
        printf("***** restructure overflow!\n");
    }
}

void Bitmap::fill(const Color &color)
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
