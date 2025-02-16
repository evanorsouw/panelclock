
#include "setupui.h"

void SetupUI::init()
{
    _resettimer = 0;
}

void SetupUI::render(Graphics &graphics)
{
    auto white = _resettimer != 0 ? Color::darkgray : Color::white;
    auto red = _resettimer != 0 ? Color::darkred : Color::red;
    auto font1 = _appctx.fonttimeLarge();

    if (_system.settings().PanelCount() == 1)
    {
        graphics.rect(0, 0, 64, 64, Color(0x151010));
        graphics.line(66, 2, 126, 62, 2, red);
        graphics.line(126, 2, 66, 62, 2, red);
        drawCentrePanel(graphics, font1, 0, 0, "one\npanel", white);
        drawButtons(graphics, 54, 46, white);
    }
    else
    {
        graphics.rect(0, 0, 128, 64, Color(0x151010));
        drawCentrePanel(graphics, font1, 0, 0, "left", white);
        drawCentrePanel(graphics, font1, 64, 0, "right", white);
        drawButtons(graphics, 118, 46, white);
    }
    drawButtons(graphics, 2, 10, white);

    if (_resettimer)
    {
        auto font = _appctx.fontSettings();
        graphics.text(font, 10, font->height(), "configuration\nsaved\nresetting...", Color::white);

        printf("timer=%llu, elapsed=%d\n", _resettimer, elapsed(_resettimer));

        if (elapsed(_resettimer) > 2000)
        {
            esp_restart();
        }
    }
}

void SetupUI::drawButtons(Graphics &graphics, float x, float y, Color color)
{
    auto font = _appctx.fontIconsM();

    auto bup = _userinput.howLongIsKeyDown(UserInput::KEY_UP) > 0;
    auto bset = _userinput.howLongIsKeyDown(UserInput::KEY_SET) > 0;
    auto bdown = _userinput.howLongIsKeyDown(UserInput::KEY_DOWN) > 0;

    graphics.text(font, x, y, bup ? '0' : '2' , color);
    graphics.text(font, x, y+8, bset ? '0' : '2' , color);
    graphics.text(font, x, y+16, bdown ? '0' : '2' , color);
}

void SetupUI::drawCentrePanel(Graphics &graphics, Font *font, float x, float y, const char *txt, Color color)
{
    auto size = font->textsize(txt);
    graphics.text(font, 
        x + (64 - size.dx) / 2,  
        y + (64 - size.dy) / 2 + font->ascend(), 
        txt, color);
}

int SetupUI::interact()
{
    if (_resettimer != 0)
        return 0;
        
    auto press = _userinput.getKeyPress();
    if (press.presstime > 250)
        return 0;

    auto &settings = _system.settings();
    switch (press.key)
    {
    case UserInput::KEY_UP:
        settings.PanelCount(settings.PanelCount() == 1 ? 2 : 1);
        break;
    case UserInput::KEY_DOWN:
        if (settings.PanelSides() == 1)
        {
            settings.PanelSides(0);
            settings.PanelOrientation(settings.PanelOrientation() == 0 ? 2 : 0);
        }
        else
        {
            settings.PanelSides(1);
        }
        break;
    case UserInput::KEY_SET:   
        settings.FlipKeys(!settings.FlipKeys());
        break;
    case UserInput::KEY_BOOT:
        settings.saveSettings();
        starttimer(_resettimer);
        break;
    }
    _ledPanel.setMode(settings.PanelOrientation() == 2, settings.PanelSides()==1);
    return 0;
}
