#ifndef _BOOTANIMATIONS_H_
#define _BOOTANIMATIONS_H_

#include "animation.h"
#include "color.h"
#include "spline.h"

class AnimationBackground : public Animation
{
public:
    AnimationBackground(Graphics &graphics, float start, float end)        
        : Animation(graphics, start, end) { }

private:
    void animate(Bitmap &screen, float when)
    {
        uint8_t iback;
        if (when < 0)
        {
            iback = 31;
        }
        else if (when < 1.0f)
        {
            iback = (uint8_t)((1.0f - when) * 31);
        }
        else
        {
            iback = 0;
        }
        Color backcol(iback, iback, iback);

        screen.fill(backcol);
        _graphics.rect(screen, 0x06, 0x27, 0x13, 0x01, Color(0x00, 0x00, 0x00));
        _graphics.rect(screen, 0x05, 0x28, 0x15, 0x13, Color(0x00, 0x00, 0x00));
        _graphics.rect(screen, 0x06, 0x3B, 0x13, 0x01, Color(0x00, 0x00, 0x00));
        _graphics.rect(screen, 0x01, 0x2D, 0x04, 0x04, Color(0x00, 0x00, 0x00));
        _graphics.rect(screen, 0x01, 0x34, 0x04, 0x04, Color(0x00, 0x00, 0x00));        
        _graphics.rect(screen, 0x02, 0x2E, 0x06, 0x02, backcol);
        _graphics.rect(screen, 0x02, 0x35, 0x06, 0x02, backcol);
    }
};

class AnimationDissolveWhiteSquares : public Animation
{
private:
    std::vector<Spline> _splines;

public:
    AnimationDissolveWhiteSquares(Graphics &graphics, float start, float end)        
        : Animation(graphics, start, end) 
    { 
        std::vector<point> points1 = { point(10,50), point(16,30), point(8, 5), point(12,-30) };
        _splines.push_back(Spline(points1, 2));

        std::vector<point> points2 = { point(18,50), point(4,40), point(32,32), point(16,25) };
        _splines.push_back(Spline(points2, 2));

        std::vector<point> points3 = { point(14,45), point(18,40), point(10, 35), point() };
        _splines.push_back(Spline(points3, 2));

        std::vector<point> points4 = { point(20,40), point(24,48) };
        _splines.push_back(Spline(points4, 2));

        std::vector<point> points5 = { point(13,37), point(10,35), point(16, 33), point(3,38) };
        _splines.push_back(Spline(points5, 2));

    }

private:
    void animate(Bitmap &screen, float when)
    {
        if (when < 0.0f || when > 1.0f)
            return;

        auto intensity = (uint8_t)(255 * (1.0 - when));
        auto size = 4 - 3 * when;
        for (auto &spline : _splines)
        {
            auto p = spline.get(when);
            _graphics.rect(screen, p.x, p.y, size, size, Color(intensity, intensity, intensity));
        }
    }
};

class AnimationColoredSquares : public Animation
{
private:
    std::vector<Spline> _splines;

public:
    AnimationColoredSquares(Graphics &graphics, float start, float end)        
        : Animation(graphics, start, end) 
    { 
        std::vector<point> pred = { point(18,32), point(30,2), point(40, 55), point(6, 40), point(3,31.5) };
        _splines.push_back(Spline(pred, 2));

        std::vector<point> pgreen = { point(24,34), point(18,40), point(10, 35), point(3,31.5) };
        _splines.push_back(Spline(pgreen, 2));

        std::vector<point> pblue = { point(23,28), point(32,0), point(3,31.5) };
        _splines.push_back(Spline(pblue, 2));
    }

private:
    void animate(Bitmap &screen, float when)
    {
        if (when < 0.0f || when > 1.0f)
            return;

        for (auto i=0; i<3; ++i)
        {
            auto p = _splines[i].get(when);
            auto color = i == 0 ? Color::red : (i == 1 ? Color::green : Color::blue);
            _graphics.rect(screen, p.x, p.y, 4, 4, color, Mode::Add);
        }
    }
};

class AnimationWhiteMagicText : public Animation
{
private:
    Spline _spline;
    Font *_font;

public:
    AnimationWhiteMagicText(Graphics &graphics, Font *font, float start, float end)        
        : Animation(graphics, start, end) 
    { 
        std::vector<point> points = { point(3,0), point(3,0), point(50,0), point(0,0), point(200,0), point(200,0) };
        _spline = Spline(points, 2);
        _font = font;
    }

private:
    void animate(Bitmap &screen, float when)
    {
        if (when < 0.0f)
            return;

        auto x = _spline.get(when).x;
        if (when < 1.0f)
        {
            auto dy = std::min(when * 150, 40.0f);
            _graphics.rect(screen, x, 31.5 -dy / 2, 3, dy, Color::white);
        }

        auto txt = "White|Magic";
        auto size = _font->textsize(txt);
        _graphics.setcliparea(irect(0,0,x,64));
        _graphics.text(screen, _font, 9, 40, txt, Color::white);
        _graphics.clearcliparea();
    }
};

class AnimationWhiteMagicColorShift : public Animation
{
private:
    Font *_font;

public:
    AnimationWhiteMagicColorShift(Graphics &graphics, Font *font, float start, float end)        
        : Animation(graphics, start, end) 
    { 
        _font = font;
    }

private:
    void animate(Bitmap &screen, float when)
    {
        if (when < 0.0f)
            return;

        auto d = std::sin(when * (float)std::numbers::pi) * 1.5;
        if (when > 1.0f)
            d = 0;

        auto txt = "White|Magic";
        _graphics.text(screen, _font, 9 - d * 2, 40, txt, Color::red, Mode::Add);
        _graphics.text(screen, _font, 9 + d * 1.5, 40, txt, Color::green, Mode::Add);
        _graphics.text(screen, _font, 9 - d, 40, txt, Color::blue, Mode::Add);
    }
};

#endif