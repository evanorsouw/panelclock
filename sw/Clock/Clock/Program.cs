
using Clock;
using System.Runtime.Intrinsics.Arm;

const int N = 64;
const int MASK = 63;

var bitmap = new Bitmap(64, 64);

void swap<T>(ref T v1, ref T v2)
{
    var tmp = v1;
    v1 = v2;
    v2 = tmp;
}

int clip(int v, int min, int max)
{
    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

void drawtriangle(Bitmap bitmap, float x1, float y1, float x2, float y2, float x3, float y3, int color)
{
    drawtriangle_i(bitmap, (int)(x1 * N), (int)(y1 * N), (int)(x2 * N), (int)(y2 * N), (int)(x3 * N), (int)(y3 * N), color);
    //bitmap.SetPixel((int)x1*N, (int)y1*N, 0xFFFF00);
    //bitmap.SetPixel((int)x2*N, (int)y2*N, 0xFFFF00);
    //bitmap.SetPixel((int)x3*N, (int)y3*N, 0xFFFF00);
}

void drawtriangle_i(Bitmap bitmap, int x1, int y1, int x2, int y2, int x3, int y3, int color)
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
        var x = x1 + (x3 - x1) * (y2 - y1) / (y3 - y1) ;
        drawtriangleup_i(bitmap, x1, y1, x2 < x ? x2 : x, x2 < x ? x : x2, y2, color);
        drawtriangledown_i(bitmap, x3, y3, x2 < x ? x2 : x, x2 < x ? x : x2, y2, color);
    }
}

void drawtriangleup_i(Bitmap bitmap, int xt, int yt, int xb1, int xb2, int yb, int color)
{
    var dxl = xb1 - xt;
    var dxr = xb2 - xt;
    var dy = yb - yt;

    var y1 = yt;
    while (y1 < yb)
    {
        var y2 = Math.Min(yb, (y1 + N) & (~MASK));

        var xl1 = xt + dxl * ((dxl < 0 ? y2 : y1) - yt) / dy;
        var xl2 = xt + dxl * ((dxl < 0 ? y1 : y2) - yt) / dy;
        var xr1 = xt + dxr * ((dxr < 0 ? y2 : y1) - yt) / dy;
        var xr2 = xt + dxr * ((dxr < 0 ? y1 : y2) - yt) / dy;
        var x = xl1;
        while (x < xr2)
        {
            var xnext = (x + N) & (~MASK);
            var tx = (x + xnext) / 2;
            var yl = clip(dxl == 0 ? y1 : yt + dy * (xt - tx) / -dxl, y1, y2);
            var yr = clip(dxr == 0 ? y2 : yt + dy * (xt - tx) / -dxr, y1, y2);

            var tdy = 0;
            if (xr1 > xl2)
            {
                if (x < xl2)
                {
                    tdy = (dxl < 0) ? y2 - yl : yl - y1;
                }
                else if (x < xr1)
                {
                    tdy = y2 - y1;
                }
                else
                {
                    tdy = (dxr < 0) ? yr - y1 : y2 - yr;
                }
            }
            else
            { 
                if (x < xr1)
                {
                    tdy = (dxl < 0) ? y2 - yl : yl - y1; 
                }
                else if (x < xl2)
                {
                    tdy = (dxl < 0) ? yr - yl : yl - yr;
                }
                else
                {
                    tdy = (dxl < 0) ? yr - y1 : y2 - yr;
                }
            }
            var ox = (x & (~MASK)) == (xr2 & (~MASK)) ? (xr2 - x) : MASK - (x & MASK);
            var oy = Math.Min(MASK, tdy);

            var alpha = ox * oy / 63;
            bitmap.SetPixel(x, y1, color, alpha);

            x = xnext;
        }
        y1 = y2;
    }
}

void drawtriangledown_i(Bitmap bitmap, int xb, int yb, int xt1, int xt2, int yt, int color)
{
    var dxl = xt1 - xb;
    var dxr = xt2 - xb;
    var dy = yb - yt;

    var y1 = yt;
    while (y1 < yb)
    {
        var y2 = Math.Min(yb, (y1 + N) & (~MASK));

        var xl1 = xb + dxl * (yb - (dxl < 0 ? y1 : y2)) / dy;
        var xl2 = xb + dxl * (yb - (dxl < 0 ? y2 : y1)) / dy;
        var xr1 = xb + dxr * (yb - (dxr < 0 ? y1 : y2)) / dy;
        var xr2 = xb + dxr * (yb - (dxr < 0 ? y2 : y1)) / dy;
        var x = xl1;
        while (x < xr2)
        {
            var xnext = Math.Min(xr2, (x + N) & (~MASK));
            var tx = (x + xnext) / 2;
            var yl = clip(dxl == 0 ? y1 : yb - dy * (xb - tx) / -dxl, y1, y2);
            var yr = clip(dxr == 0 ? y2 : yb - dy * (xb - tx) / -dxr, y1, y2);

            var tdy = 0;
            if (xr1 >= xl2)
            {
                if (x < xl2)
                {
                    tdy = (dxl < 0) ? yl - y1 :y2 - yl;
                }
                else if (x < xr1)
                {
                    tdy = y2 - y1;
                }
                else
                {
                    tdy = (dxr < 0) ? y2 - yr : yr - y1;
                }
            }
            else
            { 
                if (x < xr1)
                {
                    tdy = (dxl < 0) ? y2 - yl : yl - y1; 
                }
                else if (x < xl2)
                {
                    tdy = (dxl < 0) ? yr - yl : yl - yr;
                }
                else
                {
                    tdy = (dxl < 0) ? yr - y1 : y2 - yr;
                }
            }
            var ox = (x & (~MASK)) == (xr2 & (~MASK)) ? (xr2 - x) : MASK - (x & MASK);
            var oy = Math.Min(MASK, tdy);

            var alpha = ox * oy / 63;
            bitmap.SetPixel(x, y1, color, alpha);

            x = xnext;
        }
        y1 = y2;
    }
}

//for (float x = 0; x < 64; x += 0.5f)
//{
//    bitmap.Clear();
//    drawtriangle(bitmap, 32, 14f, 18f, 0, x, 0, 0xFF0000);
//    Thread.Sleep(50);
//}

for (; ; )
{
    var sw = 0;
    for (double i = 0; i <= 360; i += 0.5)
    {
        bitmap.Clear();
        bitmap.Rect(0, 0, 10, 10, 0xFF0000);
        bitmap.Rect(10, 0, 10, 10, 0x00FF00);
        bitmap.Rect(20, 0, 10, 10, 0x0000FF);

        var angle = (float)(i / 180f * Math.PI);

        var (x1, y1) = RotatedCenter(32f * 0.5f, angle);
        var (x2, y2) = RotatedCenter(32f * 0.5f, angle - Math.PI * 2 / 3);
        var (x3, y3) = RotatedCenter(32f * 0.5f, angle + Math.PI * 2 / 3);
        drawtriangle(bitmap, x1, y1, x2, y2, x3, y3, 0xFF0050);

        var sd = sw++;
        bitmap.SelectScreen(sd, sw);
    }
}

(float, float) RotatedCenter(double distance, double angle)
{
    var x = (float)(32 + distance * Math.Cos(angle));
    var y = (float)(32 + distance * Math.Sin(angle));
    return (x, y);
}