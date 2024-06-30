
#include "bitmap.h"
#include "ledpanel.h"

void Bitmap::copyTo(LedPanel &tgt, int tgtx, int tgty)
{
    tgt.copyFrom(*this, tgtx, tgty);
}
