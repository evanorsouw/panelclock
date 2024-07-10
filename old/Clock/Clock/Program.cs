using Clock;
using System;
using System.Drawing;


var display = new Display(128, 64);
var bitmap = new Bitmap(128, 64, System.Drawing.Imaging.PixelFormat.Format32bppRgb);

display.Show(bitmap);

for (int i = 0; i < 128; ++i)
    rect(i, 1, 1, 62, (i << 1) | (i << 9) | (i << 17));
display.Show(bitmap);


float angle = 7.19566f;
for (; ; )
{
    float x = 32 + 20 * (float)Math.Cos(angle-0.1);
    float y = 32 + 20 * (float)Math.Sin(angle-0.1);

    bitmap = new Bitmap(128, 64, System.Drawing.Imaging.PixelFormat.Format32bppRgb);
    rect(x, y, 8, 4, 0xFFFF00);
    display.Show(bitmap);

    angle = angle + 0.001f;
}

void SWAP(ref float a, ref float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

void blendpixel(int x, int y, int color, byte alpha)
{
    if (alpha == 0)
        return;


    var r = (color >> 16) & 0xFF;
    var g = (color >>8) & 0xFF;
    var b = (color) & 0xFF;

    if (alpha < 255)
    {
        var pixel = bitmap.GetPixel(x, y);

        r = (byte)Math.Min(255, ((r * alpha) >> 8) + pixel.R);
        g = (byte)Math.Min(255, ((g * alpha) >> 8) + pixel.G);
        b = (byte)Math.Min(255, ((b * alpha) >> 8) + pixel.B);
    }
    bitmap.SetPixel(x, y, Color.FromArgb(255, r, g, b));
    //display.Show(bitmap);
}

float MIN(float a, float b) => a < b ? a : b;
float MAX(float a, float b) => a > b ? a : b;
float ABS(float v) => (float)Math.Abs(v);
float TRUNC(float v) => (float)Math.Floor(v);
byte ALPHA(float a) => (byte)MIN(255, a * 255);

void triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
    if (y2 < y1)
    {
        SWAP(ref y1, ref y2);
        SWAP(ref x1, ref x2);
    }
    if (y3 < y1)
    {
        SWAP(ref y1, ref y3);
        SWAP(ref x1, ref x3);
    }
    if (y3 < y2)
    {
        SWAP(ref y2, ref y3);
        SWAP(ref x2, ref x3);
    }

    if (y1 == y2 && y1 == y3)
        return;
    if (x1 == x2 && x1 == x3)
        return;

    if (y1 == y2)
    {
        triangle_basetop(x1, x2, y1, x3, y3, 0xFF0000);
    }
    else if (y2 == y3)
    {
        triangle_basebottom(x1, y1, x2, x3, y2, 0xFF0000);
    }
    else
    {
        float dy12 = y2 - y1;
        float dy13 = y3 - y1;
        float mx = x1 + (x3 - x1) * dy12 / dy13;
        triangle_basebottom(x1, y1, x2, mx, y2, 0xFF0000);
        triangle_basetop(x2, mx, y2, x3, y3, 0xFF0000);
    }
}

void triangle_basebottom(float x1, float y1, float x2a, float x2b, float y2, int color)
{
    if (x2a > x2b) SWAP(ref x2a, ref x2b);
    float y = y1;
    float height = y2 - y1;

    float dx_a = (x2a - x1) / height;
    float dx_b = (x2b - x1) / height;
    do
    {
        float ynext = MIN(TRUNC(y + 1), y2);

        float dy = y - y1;
        float xtl = x1 + dy * dx_a;
        float xtr = x1 + dy * dx_b;

        dy = ynext - y1;
        float xbl = x1 + dy * dx_a;
        float xbr = x1 + dy * dx_b;

        float dxl = ABS(xbl - xtl);
        float dxr = ABS(xbr - xtr);
        float xl = MIN(xtl, xbl);
        float xr = MAX(xtr, xbr);

        dy = ynext - y;
        innerXLoop(y, xl, xr, dy, dxl, dxr, color);

        y = ynext;
    } 
    while (y < y2);
}

void triangle_basetop(float xbase1, float xbase2, float ybase, float xtop, float ytop, int color)
{
    if (xbase1 > xbase2) SWAP(ref xbase1, ref xbase2);
    float y = ybase;
    float height = ytop - ybase;

    float dx_a = (xbase1 - xtop) / height;
    float dx_b = (xbase2 - xtop) / height;
    do
    {
        float ynext = MIN(TRUNC(y + 1), ytop);

        float dy = ytop - y;
        float xtl = xtop + dy * dx_a;
        float xtr = xtop + dy * dx_b;

        dy = ytop - ynext;
        float xbl = xtop + dy * dx_a;
        float xbr = xtop + dy * dx_b;

        float dxl = ABS(xbl - xtl);
        float dxr = ABS(xbr - xtr);
        float xl = MIN(xtl, xbl);
        float xr = MAX(xtr, xbr);
        dy = ynext - y;

        innerXLoop(y, xl, xr, dy, dxl, dxr, color);    

        y = ynext;

    } while (y < ytop);
}

void innerXLoop(float y, float xl, float xr, float dy, float dxl, float dxr, int color)
{
    float x = xl;
    do
    {
        float xnext = MIN(xr, TRUNC(x + 1));

        float range = dy / dxl;
        float yl1 = ((x - xl) * range);
        float yl2 = ((xnext - xl) * range);
        float ayl = MIN(dy, (yl1 + yl2) / 2);

        range = dy / dxr;
        float yr1 = ((xr - x) * range);
        float yr2 = ((xr - xnext) * range);
        float ayr = MIN(dy, (yr1 + yr2) / 2);

        float axl = MIN(1, xnext - x);
        float axr = MIN(1, xr - x);

        float area = MIN(axl, axr) * MIN(ayl, ayr);
        byte alpha = (byte)TRUNC(area * 255);
        blendpixel((int)x, (int)y, color, alpha);

        x = xnext;
    }
    while (x < xr);
}


void rect(float x, float y, float dx, float dy, int color)
{
    var x2 = x + dx;
    var y1 = (int)y;

    var ay = y - TRUNC(y);
    if (ay > 0)
    {
        rectscanline(x, x2, y1++, 1 - ay, color);
    }
    var y2 = (int)(y + dy);
    while (y1 < y2)
    {
        rectscanline(x, x2, y1++, 1, color);
    }
    ay = y + dy - y1;
    if (ay > 0)
    {
        rectscanline(x, x2, y1, ay, color);
    }
}

void rectscanline(float x1, float x2, int y, float ay, int color)
{
    if (x1 > x2) SWAP(ref x1, ref x2);

    var ix1 = (int)x1;
    var ix2 = (int)x2;

    if (ix1 == ix2)
    {
        float ax = ix2 - ix2;
        blendpixel(ix1, y, color, ALPHA(ax * ay));
    }
    else
    {
        if (ix1 < x1)
        {
            blendpixel(ix1, y, color, ALPHA((1 - x1 + ix1) * ay));
            ix1++;
        }
        while (ix1 < ix2)
        {
            blendpixel(ix1, y, color, ALPHA(ay));
            ix1++;
        }
        if (ix2 < x2)
        {
            blendpixel(ix1, y, color, ALPHA((x2 - ix2) * ay));
        }
    }
}
