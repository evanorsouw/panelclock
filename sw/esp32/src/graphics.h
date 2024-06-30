
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <map>

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

private:
    SFT_Font *getfont(const char *name);
    SFT_Font *loadfont(const char *fontname);
    void assureMaskSize(int dx, int dy);
    void clip(int &srcoffset, int &tgtoffset, int &size, int max);
};

#endif