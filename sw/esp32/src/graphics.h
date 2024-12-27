
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <cmath>
#include <map>
#
#include "bitmap.h"
#include "color.h"
#include "font.h"

class TrueTypeFont;
class BitmapFont;

struct irect
{
    irect () : irect(0,0,0,0) {}
    irect (int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
    irect join(const irect &rhs)
    {
        return irect(std::min(x1,rhs.x1), std::min(y1,rhs.y1), std::max(x2, rhs.x2), std::max(y2, rhs.y2));
    }
    int x1;
    int y1;
    int x2;
    int y2;
};

enum class Mode { Set, Add };

class Graphics
{
private:
    Bitmap *_drawingBitmap;
    Bitmap *_rasterizeMask;
    int _dx;
    int _dy;
    int _sx;    // start including
    int _sy;    // start including
    int _ex;    // end excluding
    int _ey;    // end excluding
    uint8_t *_ptr;
    int _stride;

private:
    Graphics() {}
    void init(int x, int y, int dx, int dy, bool origin, bool clip);

public:
    Graphics(int dx, int dy, bool origin=true);

    int dx() const { return _ex - _sx; }
    int dy() const { return _ey - _sy; }
    
    void linkBitmap(Bitmap *bitmap);
    /// @brief get a graphics instance where (0,0) is moveed to a different location.
    /// @param x offset for x
    /// @param y offset for y
    /// @return 
    Graphics offset(int x, int y);
    /// @brief get a clipped graphics instance for a specific region
    /// @param x x coordinate for the region
    /// @param y y coordinate for the region
    /// @param dx width of the region
    /// @param dy height of the region
    /// @param origin when true, then the new instance has its (0,0) at (x,y), when false
    ///        (0,0) remains the same, but the region is clipped.
    /// @return a new clipped instance
    Graphics view(int x, int y, int dx, int dy, bool origin);

    void add(int x, int y, const Color color, uint8_t alpha);
    void set(int x, int y, const Color color);
    void set(int x, int y, const Color color, uint8_t alpha);
    void rect(float x, float y, float dx, float dy, Color color, Mode mode = Mode::Set);
    void line(float x1, float y1, float x2, float y2, float thickness, Color color);    
    float text(Font *font, float x, float y, const char *txt, Color color, Mode mode = Mode::Set);
    float text(Font *font, float x, float y, char c, Color color, Mode mode = Mode::Set);
    void triangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color);

private:
    irect rasterizeTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
    void triangleBaseTop(float xbase1, float xbase2, float ybase, float xtop, float ytop);
    void triangleBaseBottom(float xtop, float ytop, float xbase1, float xbase2, float ybase);
    void trianglescanline(float y, float xl, float xr, float dy, float dxl, float dxr);
    void rectscanline(float x1, float x2, int y, float ay, const Color color, Mode mode = Mode::Set);
    void clearRasterizedMask(int xl, int yt, int xr, int yb);
    void mergeRasterizedMask(const Color color, irect area);

    float textTTF(TrueTypeFont *font, float x, float y, const char *txt, Color color, Mode mode);
    float textWMF(BitmapFont *font, float x, float y, const char *txt, Color color, Mode mode);

    void SWAP(float &a, float &b) { float tmp=a; a=b; b=tmp; }
    float MIN(float a, float b) { return a < b ? a : b; }
    float MAX(float a, float b) { return a > b ? a : b; }
    float ABS(float x) { return x < 0 ? -x : x; }
    float TRUNC(float x) { return std::floor(x); }
    uint8_t ALPHA(float alpha) { return (uint8_t)MIN(255, alpha*255); }

    template <class T> void clip(T &v, T min, T max);
    template <class T> bool cliprange(T &x, T &dx, int min, int max);
    bool cliprange(int &srcx, int &tgtx, int &tgtdx, int min, int max);
    uint8_t *getptr(int x, int y) const { return _ptr + (x * 3 + y * _stride); }
    uint8_t clip(int v) { return v > 255 ? 255 : v; };
};

template <class T>
void Graphics::clip(T& v, T min, T max)
{
    v = std::max(min, std::min(max, v));
}

template <class T>
bool Graphics::cliprange(T& x, T& dx, int min, int max)
{
    if (x < min)
    {
        dx -= (min - x);
        x = min;
    }
    if (x + dx > max)
    {
        dx = max - x;
    }
    return dx > 0.0f;
}

#endif