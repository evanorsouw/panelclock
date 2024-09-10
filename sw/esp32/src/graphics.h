
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <cmath>
#include <map>
#
#include "bitmap.h"
#include "color.h"
#include "font.h"

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
    Bitmap _rasterizeMask;
    irect _cliparea;
    bool _clipping;

public:
    Graphics(int dx, int dy)
        : _rasterizeMask(dx, dy, 1)
    { }

    void clearcliparea() { _clipping = false; }
    void setcliparea(const irect &area) { _cliparea = area; _clipping = true; }
    void rect(Bitmap &tgt, float x, float y, float dx, float dy, Color color, Mode mode = Mode::Set);
    void line(Bitmap &tgt, float x1, float y1, float x2, float y2, float thickness, Color color);    
    float text(Bitmap &tgt, Font *font, float x, float y, const char *txt, Color color, Mode mode = Mode::Set);
    void triangle(Bitmap &tgt, float x1, float y1, float x2, float y2, float x3, float y3, Color color);
    void ellipse(Bitmap &tgt, float x, float y, float dx, float dy, float thickness, Color color);

private:
    bool clip(float &x, float &dx, float max);
    void clip(int &srcoffset, int &tgtoffset, int &size, int max);

    irect rasterizeTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
    void triangleBaseTop(float xbase1, float xbase2, float ybase, float xtop, float ytop);
    void triangleBaseBottom(float xtop, float ytop, float xbase1, float xbase2, float ybase);
    void trianglescanline(float y, float xl, float xr, float dy, float dxl, float dxr);
    void rectscanline(Bitmap &tgt, float x1, float x2, int y, float ay, const Color color, Mode mode = Mode::Set);
    void clearRasterizedMask(irect area);
    void mergeRasterizedMask(Bitmap &tgt, const Color color, irect area);

    void SWAP(float &a, float &b) { float tmp=a; a=b; b=tmp; }
    float MIN(float a, float b) { return a < b ? a : b; }
    float MAX(float a, float b) { return a > b ? a : b; }
    float ABS(float x) { return x < 0 ? -x : x; }
    float TRUNC(float x) { return std::floor(x); }
    uint8_t ALPHA(float alpha) { return (uint8_t)MIN(255, alpha*255); }
};

#endif