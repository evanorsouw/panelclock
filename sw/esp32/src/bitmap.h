#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <assert.h>
#include <cstdint>

#include "color.h"
#include "pixelsquare.h"

class Bitmap : public PixelSquare
{
private:
    int _bpp;
    int _stride;
    int _bitmapsize;
    uint8_t *_bitmap;

public:
    Bitmap(int dx, int dy, int bpp);
    Bitmap(Bitmap &parent, int ox, int oy, int dx, int dy);
    virtual ~Bitmap();

    void restructure(int dx, int dy);

    int stride() const { return _stride; }
    uint8_t *getptr() const { return _bitmap; }
    uint8_t *getptr(int x, int y) const { return _bitmap + (x * _bpp + y * _stride); }

    void fill(const Color &color);
};

#endif