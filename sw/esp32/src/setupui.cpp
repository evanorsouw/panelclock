
#include "setupui.h"

void SetupUI::init()
{
    _resettimer = 0;
}

void SetupUI::render(Graphics &graphics)
{
    auto settings = _system.settings();

    switch (settings.PanelMode())
    {
    case 0:
        draw1Panel(graphics);
        break;
    case 1:
        draw2PanelLandscape(graphics);
        break;
    case 2:
        draw2PanelPortrait(graphics);
        break;
    }

    auto d = _resettimer 
        ? std::min(1.0f, elapsed(_resettimer) / 500.0f) * 32
        : phase(1000, true);
    if (d < 32)
    {
        drawBorder(graphics, d);
    }
    else if (!_resetStarted)
    {
        _resetStarted = true;
        graphics.rect(0,0,graphics.dx(), graphics.dy(), Color::black);
    }
    else
    {
        esp_restart();
    }
}

void SetupUI::draw1Panel(Graphics &graphics)
{
    auto txt = "One\nPanel";
    auto font = _appctx.fonttimeLarge();
    auto size = font->textsize(txt);    
    graphics.text(font, (64 - size.dx) / 2, (64 - size.dy) / 2 + font->ascend(), txt, Color::white);
    graphics.rect(0,64,64,64,Color(10,0,0));
}

void SetupUI::draw2PanelLandscape(Graphics &graphics)
{
    auto txt = "Two Panel\nLandscape";
    auto font = _appctx.fonttimeLarge();
    auto size = font->textsize(txt);
    graphics.text(font, (128 - size.dx) / 2, (64 - size.dy) / 2 + font->ascend(), txt, Color::white);
}

void SetupUI::draw2PanelPortrait(Graphics &graphics)
{
    auto txt = "Two\nPanel\nPortrait";
    auto font = _appctx.fonttimeLarge();
    auto size = font->textsize(txt);
    graphics.text(font, (64 - size.dx) / 2, (128 - size.dy) / 2 + font->ascend(), txt, Color::white);
}

void SetupUI::drawBorder(Graphics &graphics, float d)
{
    auto a = 0.5f + d;
    auto b = graphics.dx() - 0.5f - d;
    auto c = graphics.dy() - 0.5f - d;
    graphics.line(a, a, b, a, 1, Color::white);
    graphics.line(a, a, a, c, 1, Color::white);
    graphics.line(b, c, b, a, 1, Color::white);
    graphics.line(b, c, a, c, 1, Color::white);
    graphics.rect(0, 0, graphics.dx(), d, Color::black);
    graphics.rect(0, graphics.dy() - d, graphics.dx(), d, Color::black);
    graphics.rect(0, 0, d, graphics.dy(), Color::black);
    graphics.rect(graphics.dx() - d, 0, d, graphics.dy(), Color::black);
}

int SetupUI::interact()
{
    if (_resettimer != 0)
        return 0;
        
    auto &settings = _system.settings();
    switch (_userinput.getKeyPress().key)
    {
    case UserInput::KEY_UP:
        settings.PanelMode((settings.PanelMode() + 1) % 3);
        break;
    case UserInput::KEY_DOWN:
        settings.PanelSides(!settings.PanelSides());
        if (!settings.PanelSides())
        {
            settings.Panel1Flipped(!settings.Panel1Flipped());
            if (!settings.Panel1Flipped())
            {
                settings.Panel2Flipped(!settings.Panel2Flipped());
            }
        }
        break;
    case UserInput::KEY_SET:   
        settings.saveSettings();
        starttimer(_resettimer);
        _resetStarted = false;
        break;
    }
    return 0;
}

