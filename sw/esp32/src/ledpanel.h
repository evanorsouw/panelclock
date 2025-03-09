
#ifndef _LEDPANEL_H_
#define _LEDPANEL_H_

#include <algorithm>
#include "appsettings.h"
#include "bitmap.h"
#include "pixelsquare.h"
#include "settings.h"
#include "spiwrapper.h"

class LedPanel : public PixelSquare
{
private:
    int _screenVisible;
    int _screenWrite;
    SpiWrapper _spi;
    int _mode = 1;
    bool _sides = false;
    bool _panel1flip = false;
    bool _panel2flip = false;

public:
    LedPanel(int dx, int dy, AppSettings &setting, SpiWrapper &spi);

    // select a screen to copy data to (not neccesarily the same as being written to)
    LedPanel &selectScreen(int iscreen);

    // select a screen that will become visible on the LED panel.
    LedPanel &showScreen(int iscreen);
    
    void clear(int color=0);

    void copyFrom(Bitmap &src);

private:
    void copy64x64(Bitmap &src, int srcx, int srcy, int tgtx, bool flip);
    void copyPixelsReverse(uint8_t *psrc, uint8_t *ptgt, int n);
    void drawCross(Bitmap &src, int tgty);
    void sendScreen();
};

#endif
