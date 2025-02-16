
#ifndef _LEDPANEL_H_
#define _LEDPANEL_H_

#include <algorithm>
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
    bool _flipy = false;
    int _sides = 0;

public:
    LedPanel(int dx, int dy, SpiWrapper &spi);

    /// @param flipy false=draw top-down,  true=draw bottom-up
    /// @param sides 0=left,right, 1=right,left
    LedPanel &setMode(bool flipy, int sides)
    {        
        _flipy = flipy;
        _sides = sides;
        return *this;
    }

    // select a screen to copy data to (not neccesarily the same as being written to)
    LedPanel &selectScreen(int iscreen);

    // select a screen that will become visible on the LED panel.
    LedPanel &showScreen(int iscreen);
    
    void clear(int color=0);

    void copyFrom(Bitmap &src, int tgtx, int tgty);

private:
    void copyPixelsReverse(uint8_t *psrc, uint8_t *ptgt, int n);
    void sendScreen();
};

#endif
