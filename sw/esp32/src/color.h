
#ifndef _COLOR_H_
#define _COLOR_H_

#include <cstdlib>

class Color
{
private:
    union {
        unsigned long argb;
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        } cmp;
    } _color;

public:
    Color(const Color& rhs) { _color = rhs._color; }
    Color(unsigned long argb) { _color.argb = argb; }
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        _color.cmp.r = r;
        _color.cmp.g = g;
        _color.cmp.b = b;
        _color.cmp.a = a;
    }

    static Color gradient(const Color &from, const Color &to, float p);

    Color &operator=(const Color &rhs) { _color.argb = rhs._color.argb; return *this; }

    bool isgray() const { return _color.cmp.r == _color.cmp.g && _color.cmp.r == _color.cmp.b; }
    uint8_t r() const { return _color.cmp.r; }
    uint8_t g() const { return _color.cmp.g; }
    uint8_t b() const { return _color.cmp.b; }
    uint8_t a() const { return _color.cmp.a; }
    Color& a(uint8_t alpha) { _color.cmp.a = alpha; return *this; }

    static Color transparant;
    static Color black;
    static Color white;
    static Color red;
    static Color blue;
    static Color lime;
    static Color yellow;
    static Color cyan;
    static Color magenta;
    static Color silver;
    static Color gray;
    static Color lightgray;
    static Color darkgray;
    static Color maroon;
    static Color olive;
    static Color green;
    static Color purple;
    static Color teal;
    static Color navy;
    static Color brown;
    static Color orangered;
    static Color skyblue;

    bool operator == (const Color& rhs) { return _color.argb == rhs._color.argb; }
    bool operator != (const Color& rhs) { return _color.argb != rhs._color.argb; }
};

#endif
