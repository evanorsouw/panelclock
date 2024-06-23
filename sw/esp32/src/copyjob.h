
#ifndef _COPYJOB_H_
#define _COPYJOB_H_

#include "pixelsquare.h"

class CopyJob
{
public:
    // initialize to copy entire src to the target
    CopyJob(PixelSquare &src, PixelSquare &tgt, int tgtx, int tgty)
    {
        // assume everyhting is can be copied
        srcx = 0;
        srcy = 0;
        dx = src.dx();
        dy = src.dy();

        this->tgtx = tgtx;
        this->tgty = tgty;

        // clip to reality
        clipRange(src, tgt);
    }

    int srcx;
    int srcy;
    int tgtx;
    int tgty;
    int dx;
    int dy;

private:
    void clipRange(PixelSquare &src, PixelSquare &tgt)
    {
        if (tgtx >= tgt.dx() || tgtx + dx <= 0 || tgty >= tgt.dy() || tgty + dy <= 0)
        {
            dx = dy = 0; // request totally out of target range
            return;
        }

        // clip to part that is visible
        clipRange(tgtx, tgt.dx(), srcx, dx);
        clipRange(tgty, tgt.dy(), srcy, dy);
    }

    void clipRange(int &tgtpos, int tgtmax, int &srcpos, int &srcmax)
    {
        if (tgtpos < 0)
        {
            srcpos += tgtpos;
            srcmax += tgtpos;
            tgtpos = 0;
        }
        auto ov = (tgtpos + srcmax) - tgtmax;
        if (ov > 0)
        {
            srcmax -= ov;            
        }       
    }
};

#endif
