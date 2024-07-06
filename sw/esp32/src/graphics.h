
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <map>
#include <cmath>

#include "bitmap.h"
#include "color.h"
#include "schrift.h"

class Graphics
{
private:
    static std::map<const char *, SFT_Font *> _fonts;
    SFT_Font *_activeFont;
    float _activeFontSizeX;
    float _activeFontSizeY;
    SFT_Image _mask;

public:
    Graphics()
    {
        _mask = {0};
        setfont("modern.ttf", 10, 12);
    }

    void rect(Bitmap &tgt, float x, float y, float dx, float dy, Color color);
    void line(Bitmap &tgt, float x1, float y1, float x2, float y2, float thickness, Color color);
    void setfont(const char *name, float sizex, float sizey);
    float text(Bitmap &tgt, float x, float y, const char *txt, Color color);
    void triangle(Bitmap &tgt, float x1, float y1, float x2, float y2, float x3, float y3, Color color);

private:
    SFT_Font *getfont(const char *name);
    SFT_Font *loadfont(const char *fontname);
    void assureMaskSize(int dx, int dy);
    void clip(int &srcoffset, int &tgtoffset, int &size, int max);

    void triangleBaseTop(Bitmap &tgt, float xbase1, float xbase2, float ybase, float xtop, float ytop, Color color);
    void triangleBaseBottom(Bitmap &tgt, float xtop, float ytop, float xbase1, float xbase2, float ybase, Color color);
    void trianglescanline(Bitmap &tgt, float y, float xl, float xr, float dy, float dxl, float dxr, Color color);
    void rectscanline(Bitmap &tgt, float x1, float x2, int y, float ay, Color color);

    void SWAP(float &a, float &b) { float tmp=a; a=b; b=tmp; }
    float MIN(float a, float b) { return a < b ? a : b; }
    float MAX(float a, float b) { return a > b ? a : b; }
    float ABS(float x) { return x < 0 ? -x : x; }
    float TRUNC(float x) { return std::floor(x); }
    uint8_t ALPHA(float alpha) { return (uint8_t)MIN(255, alpha*255); }
};

#endif