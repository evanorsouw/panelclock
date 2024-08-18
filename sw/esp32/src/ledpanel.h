
#ifndef _LEDPANEL_H_
#define _LEDPANEL_H_

#include <algorithm>
#include "bitmap.h"
#include "pixelsquare.h"
#include "spiwrapper.h"

class LedPanel : public PixelSquare
{
private:
    int _screenVisible;
    int _screenWrite;
    SpiWrapper _spi;

public:
    LedPanel(int dx, int dy, SpiWrapper &spi) 
        : PixelSquare(dx, dy) 
        , _spi(spi)
    {
        _screenVisible = 0;
        _screenWrite = 0;
        _spi.start();
    }

    // select a screen to copy data to (not neccesarily the same as being written to)
    LedPanel &selectScreen(int iscreen)
    {
        _screenWrite = iscreen & 0xF;
        sendScreen();
        return *this;
    }

    // select a screen that will become visible on the LED panel.
    LedPanel &showScreen(int iscreen)
    {
        _screenVisible = iscreen & 0xF;
        sendScreen();
        return *this;    
    }

    void clear(int color=0)
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

    void copyFrom(Bitmap &src, int tgtx, int tgty)
    {
        CopyJob job(src, *this, tgtx, tgty);
        //printf("copy: src:%d,%d, tgt:%d,%d, size:%d,%d\n", job.srcx, job.srcy, job.tgtx, job.tgty, job.dx, job.dy);

        if (job.dx == 0 || job.dy == 0)
            return;

        // send the bitmap data per line;
        int bufsize = 5 + job.dx * 3;
        uint8_t buf[bufsize];

        auto psrc = src.getptr(job.srcx, job.srcy);
        while (job.dy-- > 0)
        {                              
            buf[0] = 1;
            buf[1] = job.tgtx;
            buf[2] = job.tgty++;
            buf[3] = job.dx;
            buf[4] = 1;
            std::copy(psrc, psrc + job.dx * 3, buf + 5);           
            psrc += src.stride();
            
            _spi.write(buf, bufsize);
        }
    }

private:
    void sendScreen()
    {
        uint8_t buf[2] = { 0x18, (uint8_t)(_screenVisible | (_screenWrite << 4)) };
        _spi.write(buf, sizeof(buf));
    }
};

#endif
