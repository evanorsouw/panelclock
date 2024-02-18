
using Clock;

const int N = 64;
const int MASK = 63;

var bitmap = new Bitmap(64, 64);

void swap<T>(ref T v1, ref T v2)
{
    var tmp = v1;
    v1 = v2;
    v2 = tmp;
}

double clip(double v, double min, double max)
{
    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

void drawtriangle(Bitmap bitmap, double x1, double y1, double x2, double y2, double x3, double y3, int color)
{
    if (y2 < y1)
    {
        swap(ref x1, ref x2);
        swap(ref y1, ref y2);
    }
    if (y3 < y1)
    {
        swap(ref x1, ref x3);
        swap(ref y1, ref y3);
    }
    if (y3 < y2)
    {
        swap(ref x2, ref x3);
        swap(ref y2, ref y3);
    }

    if (y1 == y2)
    {
        drawtriangledown_i(bitmap, x3, y3, x1 < x2 ? x1 : x2, x1 < x2 ? x2 : x1, y1, color);
    }
    else if (y2 == y3)
    {
        drawtriangleup_i(bitmap, x1, y1, x2 < x3 ? x2 : x3, x2 < x3 ? x3 : x2, y2, color);
    }
    else
    {
        var x = x1 + (x3 - x1) * (y2 - y1) / (y3 - y1);
        drawtriangleup_i(bitmap, x1, y1, x2 < x ? x2 : x, x2 < x ? x : x2, y2, color);
        drawtriangledown_i(bitmap, x3, y3, x2 < x ? x2 : x, x2 < x ? x : x2, y2, color);
    }
}

void drawtriangleup_i(Bitmap bitmap, double xtop, double ytop, double xbottomleft, double xbottomright, double ybottom, int color)
{
    var dxl = xbottomleft - xtop;
    var dxr = xbottomright - xtop;
    var dy = ybottom - ytop;

    var y = ytop;
    var xtl = xtop;
    var xtr = xtop;

    while (y<ybottom)
    {
        var ynext = Math.Min(ybottom, (int)y + 1.0);

        var yy = ynext - ytop;
        var xbl = xtop + dxl * yy / dy;
        var xbr = xtop + dxr * yy / dy;
        drawscanline(bitmap, xtl, xtr, y, xbl, xbr, ynext, color);

        xtl = xbl;
        xtr = xbr;
        y = ynext;
    }
}

void drawtriangledown_i(Bitmap bitmap, double xbottom, double ybottom, double xtopleft, double xtopright, double ytop, int color)
{
    var dxl = xbottom - xtopleft;
    var dxr = xbottom - xtopright;
    var dy = ybottom - ytop;

    var y = ytop;
    var xtl = xtopleft;
    var xtr = xtopright;

    while (y < ybottom)
    {
        var ynext = Math.Min(ybottom, (int)y + 1.0);

        var yy = ynext - ytop;
        var xbl = xtopleft + dxl * yy / dy;
        var xbr = xtopright + dxr * yy / dy;
        drawscanline(bitmap, xtl, xtr, y, xbl, xbr, ynext, color);

        xtl = xbl;
        xtr = xbr;
        y = ynext;
    }
}

void drawscanline(Bitmap bitmap, double xtl, double xtr, double yt, double xbl, double xbr, double yb, int color)
{
    double lxl, lxr, lyl, lyr;
    if (xbl < xtl)
    {
        lxl = xbl;
        lyl = yb;
        lxr = xtl;
        lyr = yt;
    }
    else
    {
        lxl = xtl;
        lyl = yt;
        lxr = xbl;
        lyr = yb;
    }
    double rxl, rxr, ryl, ryr;
    if (xbr < xtr)
    {
        rxl = xbr;
        ryl = yb;
        rxr = xtr;
        ryr = yt;
    }
    else
    {
        rxl = xtr;
        ryl = yt;
        rxr = xbr;
        ryr = yb;
    }

    var dy = yb - yt;
    var dyl = (lyr - lyl) / (lxr - lxl);
    var dyr = (ryr - ryl) / (rxr - rxl);

    var x = lxl;
    while (x < rxr)
    {
        var xnext = Math.Min((int)x + 1.0, rxr);
        var xc = (x + xnext) / 2;

        var yl = clip(lyl + dyl * (xc - lxl) / dy, yt, yb);
        var yr = clip(ryl + dyr * (xc - rxl) / dy, yt, yb);

        // individual situations in left and right lines that all need separate handling
        //
        //     /  /      / |    /       / \      \  \      \    | \      \         /
        //    /  /      /  |   /       /   \      \  \      \   |  \      \       /
        //  L/  /R    L/   |  /R      /     \      \  \      \  |   \      \     /
        //  /| /      /    | /       /       \      \ |\      \ |    \      \   /
        // / |/      /     |/       /         \      \| \      \|     \      \ /
        //   (1)        (2)            (3)            (4)        (5)         (6)
        //

        var ayt = yt;
        var ayb = yb;
        if (lyl > lyr)
        {
            if (rxl < lxr)
            {   // 1
                if (xc < rxl)
                {
                    ayt = yl;
                }
                else if (xc < lxr)
                {
                    ayt = yl;
                    ayb = yr;
                }
                else
                {
                    ayb = yr;
                }

            }
            else if (ryl > ryr)
            {   // 2
                if (xc < lxr)
                {
                    ayt = yl;
                }
                else if (xc > rxl)
                {
                    ayb = yr;
                }
            }
            else
            {   // 3
                if (xc < lxr)
                {
                    ayt = yl;
                }
                else if (xc > rxl)
                {
                    ayt = yr;
                }
            }
        }
        else if (rxl < lxr)
        {   // 4
            if (xc < rxl)
            {
                ayb = yl;
            }
            else if (xc < rxl)
            {
                ayt = yr;
                ayb = yl;
            }
            else
            {
                ayt = yr;
            }
        }
        else if (ryl < ryr)
        {   // 5
            if (xc < lxr)
            {
                ayb = yl;
            }
            else if (xc > rxl)
            {
                ayt = yr;
            }
        }
        else
        {   // 6
            if (xc < lxr)
            {
                ayb = yl;
            }
            else if (xc > rxl)
            {
                ayb = yr;
            }
        }
        var alpha = (xnext - x) * (ayb - ayt);
        alpha = Math.Pow(alpha, 1.5);

        bitmap.SetPixel((int)x, (int)yt, color, alpha);

        x = xnext;
    }
}

void drawline(Bitmap bitmap, double sx, double sy, double ex, double ey, double thickness, int color)
{
    var dx = ex - sx;
    var dy = ey - sy;
    var l = Math.Sqrt(dx * dx + dy * dy);
    var ux = dx / l;
    var uy = dy / l;
    var px = -uy;
    var py = ux;

    var ox = thickness * px / 2;
    var oy = thickness * py / 2;

    var x1 = (double)(sx + ox);
    var y1 = (double)(sy + oy);
    var x2 = (double)(sx - ox);
    var y2 = (double)(sy - oy);
    var x3 = (double)(ex + ox);
    var y3 = (double)(ey + oy);
    var x4 = (double)(ex - ox);
    var y4 = (double)(ey - oy);

    drawtriangle(bitmap, x1, y1, x2, y2, x4, y4, color);
    drawtriangle(bitmap, x1, y1, x3, y3, x4, y4, color);
}

var l = 1;
for (; ; )
{
    //if (++l == 3) l = 0;
    //bitmap.SelectLut(l == 0 ? 0x11 : 0x12);

    bitmap.WriteBytes(0x10);
    for (int i = 0; i < 256; ++i)
    {
        bitmap.WriteBytes((byte)(255 - i));
    }
    //bitmap.WriteBytes(0x12);

    for (int x = 0; x < 64; ++x)
    {
        for (int y = 0; y < 10; ++y)
        {
            var c = x * 2;
            bitmap.SetPixel(x, y, c | (c << 8) | (c << 16));
        }
    }
}

int sw = 0;
for (; ; )
{
    bitmap.WriteBytes(0x11);
    bitmap.Clear();

    for (int i = 0; i < 12; ++i)
    {
        var a = Math.PI * 2 * i / 12f;
        var (x1, y1) = RotatedCenter(30 * 0.9f, a);
        var (x2, y2) = RotatedCenter(30 * 1f, a);
        drawline(bitmap, x1, y1, x2, y2, 1.5, 0xFFFFFF);
    }

    var now = DateTime.Now;
    var seconds = (now.Second + now.Millisecond / 1000f) / 60;
    var minutes = (float)((now.Minute + seconds) / 60.0);
    var hours = (float)((now.Hour % 12 + minutes) / 12.0);

    var angle1 = (hours - 0.25) * Math.PI * 2;
    var (sx, sy) = RotatedCenter(30 * 0.2, angle1);
    var (ex, ey) = RotatedCenter(30 * 0.6, angle1);
    drawline(bitmap, sx, sy, ex, ey, 3, 0xFFFFFF);

    var angle2 = (minutes - 0.25) * Math.PI * 2;
    (sx, sy) = RotatedCenter(30 * 0.2, angle2);
    (ex, ey) = RotatedCenter(30 * 0.8, angle2);
    drawline(bitmap, sx, sy, ex, ey, 2, 0xFFFFFF);

    var angle3 = (seconds - 0.25) * Math.PI * 2;
    (sx, sy) = RotatedCenter(30 * 0.1, angle3);
    (ex, ey) = RotatedCenter(30 * 0.9, angle3);
    drawline(bitmap, sx, sy, ex, ey, 1, 0xFF0000);

    var sd = sw++;
    bitmap.SelectScreen(sd, sw);
    //bitmap.SelectScreen(0,0);
}

(double, double) RotatedCenter(double distance, double angle)
{
    var x = (double)(32 + distance * Math.Cos(angle));
    var y = (double)(32 + distance * Math.Sin(angle));
    return (x, y);
}