
#include "ledpanel.h"

LedPanel::LedPanel(int dx, int dy, SpiWrapper &spi) 
    : PixelSquare(dx, dy) 
    , _spi(spi)
{
    _screenVisible = 0;
    _screenWrite = 0;
    _spi.start();
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

void LedPanel::copyFrom(Bitmap &src, int tgtx, int tgty)
{
    CopyJob job(src, *this, tgtx, tgty);
    //printf("copy: src:%d,%d, tgt:%d,%d, size:%d,%d\n", job.srcx, job.srcy, job.tgtx, job.tgty, job.dx, job.dy);

    if (job.dx == 0 || job.dy == 0)
        return;

    // send the bitmap data per line;
    uint8_t buf[5 + dx() * 3];

    auto y1 = job.tgty;
    auto y2 =  y1 + job.dy;
    auto sy = 1;
    if (_flipy)
    {        
        y1 = job.tgty + job.dy - 1;
        y2 = job.tgty - 1;
        sy = -1;
    }

    auto ox1 = job.tgtx;
    auto dx1 = job.dx;
    auto ox2 = 0;
    auto dx2 = 0;
    if (_sides)
    {
        if (ox1 < 64 && ox1 + dx1 > 64)
        {
            ox1 = ox1 + 64;
            dx1 = 128 - ox1;
            ox2 = 0;
            dx2 = job.dx - dx1;
        }
        else if (ox1 < 64)
        {
            ox1 = ox1 + 64;
        }
        else
        {
            ox1 = ox1 - 64;
        }
        if (_flipy)
        {
            std::swap(ox1, ox2);            
            std::swap(dx1, dx2);            
        }
    }

    auto psrc = src.getptr(job.srcx, job.srcy);
    for (auto y = y1; y != y2; y += sy)
    {
        if (_flipy)
        {
            buf[0] = 1;
            buf[1] = ox1;
            buf[2] = y;
            buf[3] = dx1;
            buf[4] = 1;
            copyPixelsReverse(psrc, buf + 5, dx1);
            _spi.write(buf, dx1*3+5);
            if (dx2 > 0)
            {
                buf[0] = 1;
                buf[1] = ox2;
                buf[2] = y;
                buf[3] = dx2;
                buf[4] = 1;
                copyPixelsReverse(psrc + dx1 * 3, buf + 5, dx2);
                _spi.write(buf, dx2*3+5);
            }
            psrc += src.stride();
        }
        else
        {
            buf[0] = 1;
            buf[1] = ox1;
            buf[2] = y;
            buf[3] = dx1;
            buf[4] = 1;
            std::copy(psrc, psrc + dx1 * 3, buf + 5);
            _spi.write(buf, dx1*3+5);
            if (dx2 > 0)
            {
                buf[0] = 1;
                buf[1] = ox2;
                buf[2] = y;
                buf[3] = dx2;
                buf[4] = 1;
                std::copy(psrc + dx1 * 3, psrc + (dx1 + dx2) * 3, buf + 5);
                _spi.write(buf, dx2*3+5);
            }
            psrc += src.stride();
        }
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
