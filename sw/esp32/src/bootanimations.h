#ifndef _BOOTANIMATIONS_H_
#define _BOOTANIMATIONS_H_

#include <cstdlib>
#include <vector>

#include "animation.h"
#include "spline.h"
#include "renderbase.h"

class AnimationBackground : public Animation
{
public:
    AnimationBackground(float start, float end)        
        : Animation(start, end) { }

private:
    void animate(Graphics &graphics, float when)
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

        graphics.rect(0,0,128,0,backcol);
        graphics.rect(0x06, 0x27, 0x13, 0x01, Color(0x00, 0x00, 0x00));
        graphics.rect(0x05, 0x28, 0x15, 0x13, Color(0x00, 0x00, 0x00));
        graphics.rect(0x06, 0x3B, 0x13, 0x01, Color(0x00, 0x00, 0x00));
        graphics.rect(0x01, 0x2D, 0x04, 0x04, Color(0x00, 0x00, 0x00));
        graphics.rect(0x01, 0x34, 0x04, 0x04, Color(0x00, 0x00, 0x00));        
        graphics.rect(0x02, 0x2E, 0x06, 0x02, backcol);
        graphics.rect(0x02, 0x35, 0x06, 0x02, backcol);
    }
};

class AnimationDissolveWhiteSquares : public Animation
{
private:
    std::vector<Spline> _splines;

public:
    AnimationDissolveWhiteSquares(float start, float end)        
        : Animation(start, end) 
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
    void animate(Graphics &graphics, float when)
    {
        if (when < 0.0f || when > 1.0f)
            return;

        auto intensity = (uint8_t)(255 * (1.0 - when));
        auto size = 4 - 3 * when;
        for (auto &spline : _splines)
        {
            auto p = spline.get(when);
            graphics.rect(p.x, p.y, size, size, Color(intensity, intensity, intensity));
        }
    }
};

class AnimationColoredSquares : public Animation
{
private:
    std::vector<Spline> _splines;

public:
    AnimationColoredSquares(float start, float end)        
        : Animation(start, end) 
    { 
        std::vector<point> pred = { point(18,32), point(30,2), point(40, 55), point(6, 40), point(3,31.5) };
        _splines.push_back(Spline(pred, 2));

        std::vector<point> pgreen = { point(24,34), point(18,40), point(10, 35), point(3,31.5) };
        _splines.push_back(Spline(pgreen, 2));

        std::vector<point> pblue = { point(23,28), point(32,0), point(3,31.5) };
        _splines.push_back(Spline(pblue, 2));
    }

private:
    void animate(Graphics &graphics, float when)
    {
        if (when < 0.0f || when > 1.0f)
            return;

        for (auto i=0; i<3; ++i)
        {
            auto p = _splines[i].get(when);
            auto color = i == 0 ? Color::red : (i == 1 ? Color::green : Color::blue);
            graphics.rect(p.x, p.y, 4, 4, color, Mode::Add);
        }
    }
};

class AnimationWhiteMagic : public Animation
{
private:
    Font *_font;

public:
    AnimationWhiteMagic(Font *font, float start, float end)        
        : Animation(start, end) 
    { 
        _font = font;
    }

protected:
    void drawWhiteMagic(Graphics &graphics) { drawWhiteMagic(graphics, Color::white, irect(0,0,128,64), 0.0f, Mode::Set); }
    void drawWhiteMagic(Graphics &graphics, Color color, irect cliparea, float xoffset, Mode mode)
    {
        auto txt = "White|Magic";
        auto size = _font->textsize(txt);
        //_graphics.setcliparea(cliparea);
        graphics.text(_font, (128 - size.dx)/2 + xoffset, (64 - size.dy) / 2 + _font->ascend(), txt, color, mode);
        //_graphics.clearcliparea();
    }
};

class AnimationWhiteMagicText : public AnimationWhiteMagic
{
private:
    Spline _spline;

public:
    AnimationWhiteMagicText(Font *font, float start, float end)        
        : AnimationWhiteMagic(font, start, end) 
    { 
        std::vector<point> points = { point(3,0), point(3,0), point(50,0), point(0,0), point(200,0), point(200,0) };
        _spline = Spline(points, 2);
    }

private:
    void animate(Graphics &graphics, float when)
    {
        if (when < 0.0f)
            return;

        auto x = _spline.get(when).x;
        if (when < 1.0f)
        {
            auto dy = std::min(when * 150, 40.0f);
            graphics.rect(x, 31.5 -dy / 2, 3, dy, Color::white);
        }
        drawWhiteMagic(graphics, Color::white, irect(0,0,x,64),0.0f,Mode::Set);
    }
};

class AnimationWhiteMagicColorShift : public AnimationWhiteMagic
{
public:
    AnimationWhiteMagicColorShift(Font *font, float start, float end)        
        : AnimationWhiteMagic(font, start, end) { }

private:
    void animate(Graphics &graphics, float when)
    {
        if (when < 0.0f)
            return;

        auto d = std::sin(when * (float)std::numbers::pi) * 1.5;
        if (when > 1.0f)
            d = 0;

        auto cliparea = irect(0,0,128,64);
        drawWhiteMagic(graphics, Color::red, cliparea, -d * 1.5, Mode::Add);
        drawWhiteMagic(graphics, Color::green, cliparea, d, Mode::Add);
        drawWhiteMagic(graphics, Color::blue, cliparea, -d, Mode::Add);
    }
};

class AnimationWhiteMagicRemove : public AnimationWhiteMagic
{
public:
    AnimationWhiteMagicRemove(Font *font, float start, float end)        
        : AnimationWhiteMagic(font, start, end) 
    { 
    }

private:
    void animate(Graphics &graphics, float when)
    {     
        if (when < 0.0f)
            return;

        auto dx = ((int)(when * 128) / 4) * 4;
        drawWhiteMagic(graphics);
        graphics.rect(0, 0, dx, 64, Color::black);

        for (int ix=0; ix<5; ++ix)
        {
            for (auto iy=0; iy<16; ++iy)
            {
                auto x = dx + ix * 4;
                auto y = iy * 4;
                if (std::rand() % 6 > ix)
                {
                    graphics.rect(x, y, 4, 4, Color::black);
                }
            }
        }
    }
};

class BootAnimations : public RenderBase
{
private:
    std::vector<Animation*> _bootAnimations;
    int64_t _bootStart;
    bool _bootCompleted;

public:
    BootAnimations(ApplicationContext &appdata, Environment &env, System &sys, UserInput &userinput)
        : RenderBase(appdata, env, sys, userinput)
    {
    }

    void init()
    {
        _bootAnimations.push_back(new AnimationBackground(0.0f, 0.4f));
        _bootAnimations.push_back(new AnimationDissolveWhiteSquares(.0f, 1.6f));
        _bootAnimations.push_back(new AnimationColoredSquares(0.0f, 2.0f));
        _bootAnimations.push_back(new AnimationWhiteMagicText(_appctx.fontWhiteMagic(), 2.0f, 3.8f));
        _bootAnimations.push_back(new AnimationWhiteMagicColorShift(_appctx.fontWhiteMagic(), 3.8f, 4.5f));
        _bootAnimations.push_back(new AnimationWhiteMagicRemove(_appctx.fontWhiteMagic(), 4.5f, 5.5f));

        _bootStart = esp_timer_get_time();
        _bootCompleted = false;
    }

    void render(Graphics& graphics)
    {   
        auto busy = false;
        float when = (esp_timer_get_time() - _bootStart) / 1000000.0f;
        if (!_bootCompleted)
        {
            for (auto it : _bootAnimations) 
            {
                busy |= it->run(graphics, when); 
            }
            if (!busy)
            {
                while (!_bootAnimations.empty())
                {
                    delete _bootAnimations.front();
                    _bootAnimations.erase(_bootAnimations.begin());
                }
                _bootCompleted = true;
            }
        }
    }

    bool interact() { return _bootCompleted; } 
};

#endif