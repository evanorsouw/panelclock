
#ifndef _TRUETYPEFONT_H_
#define _TRUETYPEFONT_H_

#include <map>

#include "font.h"
#include "schrift.h"

struct fontinfo
{
    SFT_Font *font;
    int usecount;
};

class TrueTypeFont : public Font
{
friend class Graphics;
private:
    static std::map<const char *, fontinfo> _loadedFonts;
    const char *_fontname;
    SFT _sft;
    float _dx;
    float _dy;
    SFT_LMetrics _lmetrics;

    TrueTypeFont(const char *fontname, SFT sft, SFT_LMetrics lmetrics, int dx, int dy) 
        : Font(FontType::TTF)
        , _fontname(fontname)
        , _sft(sft)
        , _dx(dx)
        , _dy(dy)
        , _lmetrics(lmetrics) 
        {}

public:
    static TrueTypeFont *getFont(const char *fontname, float dx, float dy);
    ~TrueTypeFont() override;

    int splittext(const char *txt, float maxwidth) const override;
    textinfo textsize(const char *txt) const override;
    textinfo charsize(int codepoint) const override;

    float sizex() const override { return _dx; }
    float sizey() const override{ return _dy; }
    float ascend() const override{ return _lmetrics.ascender; }
    float descend() const override{ return _lmetrics.descender; }
    float height() const override{ return ascend() - descend(); }

private:
    SFT getSFT() { return _sft; }
    static SFT_Font *loadFont(const char *fontname);
};

#endif
