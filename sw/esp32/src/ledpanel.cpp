
#include "ledpanel.h"

LedPanel::LedPanel(int dx, int dy, AppSettings &settings, SpiWrapper &spi) 
    : PixelSquare(dx, dy) 
    , _spi(spi)
{
    _screenVisible = 0;
    _screenWrite = 0;
    _spi.start();

    settings.onChanged([&](const Setting *s) {
        _mode = settings.PanelMode();
        _sides = settings.PanelSides();
        _panel1flip = settings.Panel1Flipped();
        _panel2flip = settings.Panel2Flipped();
        printf("mode=%d, sides=%d, panel1flip=%d, panel2flip=%d\n", _mode, _sides, _panel1flip, _panel2flip);
    }, true);
}

// select a screen to copy data to (not neccesarily the same as being written to)
LedPanel &LedPanel::selectScreen(int iscreen)
{
    _screenWrite = iscreen & 0xF;
    sendScreen();
    return *this;
}

// select a screen that will become visible on the LED panel.
LedPanel &LedPanel::showScreen(int iscreen)
{
    _screenVisible = iscreen & 0xF;
    sendScreen();
    return *this;    
}

void LedPanel::clear(int color)
{
    uint8_t buf[8];
    
    buf[0] = 2;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = dx();
    buf[4] = dy();
    buf[5] = color >> 16;
    buf[6] = color >> 8;
    buf[7] = color;
    _spi.write(buf, 8);
}

void LedPanel::copyFrom(Bitmap &src)
{
    switch (_mode)
    {
    case 1: // 2 panel landscape
        if (_sides)
        {
            copy64x64(src, 64, 0, 0, _panel1flip);
            copy64x64(src, 0, 0, 64, _panel2flip);
        }
        else
        {
            copy64x64(src, 0, 0, 0, _panel1flip);
            copy64x64(src, 64, 0, 64, _panel2flip);
        }
        break;
    case 0: // single panel
    case 2: // 2 panel portrait
        if (_sides)
        {
            copy64x64(src, 0, 0, 64, _panel1flip);
            copy64x64(src, 0, 64, 0, _panel2flip);
        }
        else
        {
            copy64x64(src, 0, 0, 0, _panel1flip);
            copy64x64(src, 0, 64, 64, _panel2flip);
        }
        break;
    }

}

void LedPanel::copy64x64(Bitmap &src, int srcx, int srcy, int tgtx, bool flip)
{
    auto psrc = src.getptr(srcx, srcy);

    // send the bitmap per 64 pixels line;
    uint8_t buf[5 + 64 * 3];
    buf[0] = 1;
    buf[1] = tgtx;
    buf[3] = 64;
    buf[4] = 1;

    for (int iy=0; iy<64; ++iy)
    {
        if (flip)
        {
            buf[2] = 63 - iy;
            copyPixelsReverse(psrc, buf + 5, 64);
        }
        else
        {
            buf[2] = iy;
            std::copy(psrc, psrc + 64 * 3, buf + 5);
        }
        psrc += src.stride();
        _spi.write(buf, 64 * 3 + 5);
    }
}

void LedPanel::copyPixelsReverse(uint8_t *psrc, uint8_t *ptgt, int n)
{
    psrc += n * 3;
    while (n-- > 0)
    {
        psrc -= 3;
        ptgt[0] = psrc[0];
        ptgt[1] = psrc[1];
        ptgt[2] = psrc[2];
        ptgt += 3;
    }
}

void LedPanel::sendScreen()
{
    uint8_t buf[2] = { 0x18, (uint8_t)(_screenVisible | (_screenWrite << 4)) };
    _spi.write(buf, sizeof(buf));
}
