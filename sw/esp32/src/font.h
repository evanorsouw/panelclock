
#ifndef _FONT_H_
#define _FONT_H_

#include <map>
#include <string>

struct textinfo
{
    textinfo() : dx(0.0f), dy(0.0f) {}
    textinfo(float ix, float iy) : dx(ix), dy(iy) {}
    float dx;
    float dy;
};

class Font
{
protected:
    enum class FontType { TTF, WMF };

private:
    static std::map<std::string, Font*> _loadedFonts;
    FontType _type;

protected:
    Font(FontType type) : _type(type) {}

public:
    static Font *getFont(const char *font, float dx, float dy);
    virtual ~Font() {}
    FontType getFontType() const { return _type; }

    virtual int splittext(const char *txt, float maxwidth) const = 0;
    virtual textinfo textsize(const std::string &txt) const { return textsize(txt.c_str()); }
    virtual textinfo textsize(const char *txt) const = 0;
    virtual textinfo charsize(int codepoint) const = 0;

    virtual float sizex() const = 0;
    virtual float sizey() const = 0;
    virtual float ascend() const = 0;
    virtual float descend() const = 0;
    virtual float height() const = 0;
};

#endif
