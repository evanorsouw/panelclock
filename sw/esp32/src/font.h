
#ifndef _FONT_H_
#define _FONT_H_

#include <map>
#include "schrift.h"

struct textinfo
{
    float dx;
    float dy;
};

struct fontinfo
{
    SFT_Font *font;
    int usecount;
};

class Font
{
friend class Graphics;

private:
    static std::map<const char *, fontinfo> _loadedFonts;
    const char *_fontname;
    SFT _sft;
    float _dx;
    float _dy;
    SFT_LMetrics _lmetrics;

    Font(const char *fontname, SFT sft, SFT_LMetrics lmetrics, int dx, int dy) 
        : _fontname(fontname)
        , _sft(sft)
        , _dx(dx)
        , _dy(dy)
        , _lmetrics(lmetrics) 
        {}

public:
    static Font *getFont(const char *font, float dx, float dy);

    virtual ~Font();

    textinfo textsize(const char *txt);

    float sizex() const { return _dx; }
    float sizey() const { return _dy; }
    float ascend() const { return _lmetrics.ascender; }
    float descend() const { return _lmetrics.descender; }
    float height() const { return ascend() - descend(); }

private:
    SFT getSFT() { return _sft; }
    static SFT_Font *loadfont(const char *fontname);
};

#endif
