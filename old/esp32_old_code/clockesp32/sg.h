
#ifndef _SG_H_
#define _SG_H_

#include <memory.h>
#include <map>
#include <string>
#undef min
#undef max
#include <algorithm>
#include "schrift.h"

class display;

class Color
{
private:
    union {
        unsigned char bw;
        unsigned long rgba;
        struct rgba {
            unsigned char b;
            unsigned char g;
            unsigned char r;
            unsigned char a;
        } cmp;
    } _color;

public:
    Color(const Color& rhs) { _color = rhs._color; }
    Color(unsigned long rgba) { _color.rgba = rgba; }
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
    {
        _color.cmp.r = r;
        _color.cmp.g = g;
        _color.cmp.b = b;
        _color.cmp.a = a;
    }
    unsigned char r() const { return _color.cmp.r; }
    unsigned char g() const { return _color.cmp.g; }
    unsigned char b() const { return _color.cmp.b; }
    unsigned char a() const { return _color.cmp.a; }
    Color& a(unsigned char alpha) { _color.cmp.a = alpha; return *this; }
    unsigned char bw() const { return _color.bw; }

    Color at75() const { return Color(r() * 3 / 4, g() * 3 / 4, b() * 3 / 4, a()); }
    Color at50() const { return Color(r() / 2, g() / 2, b() / 2, a()); }
    Color at25() const { return Color(r() / 4, g() / 4, b() / 4, a()); }

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
    static Color maroon;
    static Color olive;
    static Color green;
    static Color purple;
    static Color teal;
    static Color navy;
    static Color brown;
    static Color orangered;
    static Color skyblue;

    bool operator == (const Color& rhs) { return _color.rgba == rhs._color.rgba; }
    bool operator != (const Color& rhs) { return _color.rgba != rhs._color.rgba; }
};

class PixelPtr;

class Bitmap
{
friend PixelPtr;
private:
    unsigned char* _ptr;
    unsigned char* _maxptr;
    int _bytesperpixel;
    int _width;
    int _height;
    int _stride;

    Bitmap() {}

public:
    static const unsigned int FLG_COMPACT = 0x01;

public:
    static Bitmap* create(int dx, int dy, unsigned char bytesperpixel, unsigned int flags);
    int width() const { return _width; }
    int height() const { return _height; }
    int stride() const { return _stride; }
    int bitmapsize() const { return _maxptr - _ptr; }

    void clear(Color color);
    void setpixel(int x, int y, Color color) { setpixel(ptr(x, y), color); }
    void blendpixel(int x, int y, Color color) { blendpixel(ptr(x, y), color); }
    unsigned char *setpixel(unsigned char* ptr, Color color)
    {
        if (_bytesperpixel == 3)
        {
            ptr[0] = color.r();
            ptr[1] = color.g();
            ptr[2] = color.b();
        }
        else
        {
            ptr[0] = color.bw();
        }
        return ptr + _bytesperpixel;
    }
    unsigned char *blendpixel(unsigned char* ptr, Color color)
    {
        if (color.a() == 255)
        {
            setpixel(ptr, color);
        }
        else if (_bytesperpixel == 3)
        {
            ptr[0] = (ptr[0] * (255 - color.a()) + color.r() * color.a()) / 256;
            ptr[1] = (ptr[1] * (255 - color.a()) + color.g() * color.a()) / 256;
            ptr[2] = (ptr[2] * (255 - color.a()) + color.b() * color.a()) / 256;
        }
        else
        {
            ptr[0] = color.bw();
        }
        return ptr + _bytesperpixel;
    }
    Color getpixel(int x, int y) const
    { 
        auto p = ptr(x, y);
        if (_bytesperpixel == 3)
            return Color(p[0], p[1], p[2]);
        return Color(p[0], p[0], p[0]);
    }
    PixelPtr pixptr() const;
    PixelPtr pixptr(int x, int y) const;
    unsigned char* ptr() const { return _ptr; }
    unsigned char* ptr(int x, int y) const { return _ptr + x * _bytesperpixel + y * _stride; }

    virtual ~Bitmap()
    {
        delete _ptr;
        _ptr = _maxptr = nullptr;
    }
};

class PixelPtr
{
private:
    unsigned char* _minptr;
    unsigned char* _maxptr;
    int _bytesperpixel;
    int _stride;
    unsigned char* _ptr;

public:
    PixelPtr(const PixelPtr& rhs) : _ptr(rhs._ptr), _minptr(rhs._minptr), _maxptr(rhs._maxptr), _bytesperpixel(rhs._bytesperpixel), _stride(rhs._stride) {}
    PixelPtr(const Bitmap* bitmap) : PixelPtr(bitmap->_ptr, bitmap) {}
    PixelPtr(unsigned char *ptr, const Bitmap* bitmap) : _ptr(ptr), _minptr(bitmap->_ptr), _maxptr(bitmap->_maxptr), _bytesperpixel(bitmap->_bytesperpixel), _stride(bitmap->_stride) {}

    void set(Color color)
    {
        if (_ptr >= _minptr && _ptr < _maxptr)
        {
            if (_bytesperpixel == 3)
            {
                _ptr[0] = color.r();
                _ptr[1] = color.g();
                _ptr[2] = color.b();
            }
            else
            {
                _ptr[0] = color.b();
            }
        }
    }
    int copy(unsigned char* targetptr)
    {
        if (_ptr >= _minptr && _ptr < _maxptr)
        {
            if (_bytesperpixel == 3)
            {
                targetptr[0] = _ptr[0];
                targetptr[1] = _ptr[1];
                targetptr[2] = _ptr[2];
            }
            else
            {
                targetptr[0] = _ptr[0];
            }
        }
        return _bytesperpixel;
    }
    int copyright(unsigned char* ptr) { copy(ptr); right(); return _bytesperpixel; }
    void setright(Color color) { set(color);  right(); }
    void left() { _ptr -= _bytesperpixel; }
    void right() { _ptr += _bytesperpixel; }
    void down() { _ptr += _stride; }
    void up() { _ptr += _stride; }
};

class Graphics
{
private:
    Bitmap* _bitmap;
    std::map<std::string, SFT_Font*> _fonts;
    SFT_Font* _activeFont;
    Color _activeTextColor;
    float _activeFontSize = 12;

public:
    Graphics(Bitmap* bitmap) : _bitmap(bitmap), _activeTextColor(Color::white) {}

    void clear(Color color) { _bitmap->clear(color); }

    void line1(int x0, int y0, int x1, int y1, Color color);
    void line(float xs, float ys, float xe, float ye, Color color, float thickness, display *disp);
    void scanline(float x1, float x2, float x3, int y, float yh1, float yh2, float yh3, Color color, display* disp);
    Graphics& setfont(float sizex, Color color);
    void text(float x, float y, const std::string &txt);

private:
    SFT_Font* getfont(const std::string& path);
};

#endif
