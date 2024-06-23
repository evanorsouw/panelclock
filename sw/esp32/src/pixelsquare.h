
#ifndef _PIXELSQUARE_H_
#define _PIXELSQUARE_H_

class PixelSquare 
{
private:
    int _dx;
    int _dy;

public:
    PixelSquare(int dx, int dy) : _dx(dx), _dy(dy) {}
    virtual ~PixelSquare() {}

    int dx() const { return _dx; }
    int dy() const { return _dy; }
};

#endif
