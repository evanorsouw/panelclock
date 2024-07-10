// See https://aka.ms/new-console-template for more information
using Microsoft.VisualBasic;
using System;
using System.ComponentModel.DataAnnotations;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.Intrinsics;
using System.Xml;
using WhiteMagic.Clock;

var bitmap = new Bitmap(128, 64, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
var panel = new LedPanelDisplay("com9", 128, 64);

int[,] CreateColorArray(int color)
{
    var colors = new int[8, 8];
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            float scale = (float)Math.Pow((8 - i) * (8 - j) / 64.0, 1.5);
            var r = (byte)((color >> 16) & 0xFF);
            var g = (byte)((color >> 8) & 0xFF);
            var b = (byte)(color & 0xFF);
            r = (byte)(r * scale);
            g = (byte)(g * scale);
            b = (byte)(b * scale);
            colors[i, j] = r << 16 | g << 8 | b;

            bitmap.SetPixel(56 + i, 56 + j, Color.FromArgb(colors[i, j]));
        }
    }
    return colors;
}

void swap<T>(ref T v1, ref T v2)
{
    var tmp = v1;
    v1 = v2;
    v2 = tmp;
}

void drawline(Bitmap bitmap, float x1, float y1, float x2, float y2, float thickness, int color)
{
    var colors = CreateColorArray(color);

    if ((y1 - y2) * 8 == 0)
    {
        if (x1 > x2)
        {
            swap(ref x1, ref x2);
        }
        // horizontal line
        var ax = (int)(x1 * 8);
        var ay = (int)((y1 - thickness / 2) * 8);
        var bx = (int)(x2 * 8);
        var by = (int)(ay + thickness * 8);

        drawrect8(bitmap, ax, ay, bx, by, colors);
    }
    else if ((x1 - x2) * 8 == 0)
    {
        if (y1 > y2)
        {
            swap(ref y1, ref y2);
        }
        // vertical line
        var ax = (int)((x1 - thickness / 2) * 8);
        var ay = (int)(y1 * 8);
        var bx = (int)(ax + thickness * 8);
        var by = (int)(y2 * 8);

        drawrect8(bitmap, ax, ay, bx, by, colors);
    }
    else
    {
        var w = x2 - x1;
        var h = y2 - y1;
        var l = Math.Sqrt(w * w + h * h);
        var deltax = thickness * -(h / l) / 2;
        var deltay = thickness * (w / l) / 2;

        var px = new int[] { (int)((x1 - deltax) * 8), (int)((x2 - deltax) * 8), (int)((x1 + deltax) * 8), (int)((x2 + deltax) * 8) };
        var py = new int[] { (int)((y1 - deltay) * 8), (int)((y2 - deltay) * 8), (int)((y1 + deltay) * 8), (int)((y2 + deltay) * 8) };

        for (int i=0; i<4; ++i)
        {
            for (int j = i+1; j < 4; ++j)
            {
                if (py[i] > py[j])
                {
                    swap(ref px[i], ref px[j]);
                    swap(ref py[i], ref py[j]);
                }
            }
        }

        int sxl, sxr, syl, syr, dxl, dxr, dyl, dyr;
        for (var phase = 0; phase < 3; ++phase)
        {
            var yt = py[phase];
            var yb = py[phase + 1];

            if (phase == 0)
            {
                sxl = sxr = px[phase];
                syl = syr = py[phase];
                var il = px[1] < px[2] ? 1 : 2;
                var ir = px[1] < px[2] ? 2 : 1;
                dxl = px[il] - px[0];
                dxr = px[ir] - px[0];
                dyl = py[il] - py[0];
                dyr = py[ir] - py[0];
            }
            else 
            {
                if (px[1] < px[2])
                {
                    sxl = px[3];
                    syl = py[3];
                    dxl = px[3] - py[phase];
                    dyl = py[3] - py[phase];
                }
                else
                {
                    sxr = px[3];
                    syr = py[3];
                    dxr = px[3] - py[phase];
                    dyr = py[3] - py[phase];
                }
            }

            while (yt <= yb)
            {


                yt = (yt + 8) & ~7;
            }
        }
    }
}

void drawscanlines(Bitmap bitmap, int phase, int ys, int ye, int sxl, int sxr, int syl, int syr, int dxl, int dxr, int dyl, int dyr, int[,] colors)
{
    var y = ys;
    if (phase == 2)
    {
        y = (y + 7) & ~7;
    }
    else if (phase == 3)
    {
        y &= ~7;
    }
    var endy = ye;
    if (phase == 2)
    {
        endy = (ye & ~7);
    }
    while (y < endy)
    {
        var yb = (y + 8) & ~7;
        var yb2 = Math.Min(yb, ye);
        var xlt = sxl + dxl * Math.Max(0, y - syl) / dyl;
        var xlb = sxl + dxl * Math.Max(0, yb2 - syl) / dyl;
        var xrt = sxr + dxr * Math.Max(0, y - syr) / dyr;
        var xrb = sxr + dxr * Math.Max(0, yb2 - syr) / dyr;

        var x1 = xlt < xlb ? xlt : xlb;
        var x2 = xlt < xlb ? xlb : xlt;

        // beginning of scanline, increment intensity.
        if (x1 < x2)
        {
            if (x1 / 8 == x2 / 8)
            {
                // within same pixel
                var ox = x1 - (x1 & ~7);
                var oy = y & 7;
                bitmap.SetPixel(x1 / 8, y / 8, Color.FromArgb(colors[ox, oy]));
                show();
                x1 = (x1 + 8) & ~7;
            }
            else
            {
                var dx = x2 - x1;
                var dy = yb - y;
                while (x1 < x2)
                {
                    var ysteps = dy * (dx - x2 + x1) / dx;
                    var ox = x1 & 7;
                    var oy = 7 - ysteps;

                    bitmap.SetPixel(x1 / 8, y / 8, Color.FromArgb(colors[ox, oy]));
                    show();
                    x1 = (x1 + 8) & ~7;
                }
            }
        }
        // middle of scanline, constant intensity
        var xe = (xrt < xrb ? xrt : xrb) & ~7;
        while (x1 < xe)
        {
            var ox = 0;
            int oy = 0;
            if (phase == 3)
            {
            }
            bitmap.SetPixel(x1 / 8, y / 8, Color.FromArgb(colors[ox, oy]));
            show();
            x1 = (x1 + 8) & ~7;
        }
        // end of scanline, decreasing intensity.
        x2 = xrt < xrb ? xrb : xrt;
        if (x1 < x2)
        {
            if (x1 / 8 == x2 / 8)
            {
                // end is within 1 pixel
                var ox = 7 - Math.Abs(x1 - x2);
                var oy = y & 7;
                bitmap.SetPixel(x1 / 8, y / 8, Color.FromArgb(colors[ox, oy]));
                show();
                x1 = (x1 + 8) & ~7;
            }
            else
            {
                // end is a range of pixels decreasing in y
                var dx = x2 - x1;
                var dy = yb - y - 1;
                while (x1 < x2)
                {
                    var ox = (x2 - x1) >= 8 ? 0 : 7 - (x2 - x1);
                    var oy = (yb - 1 - dy * (x2 - x1) / dx) & 7;
                    if (phase == 3)
                    {
                    }

                    bitmap.SetPixel(x1 / 8, y / 8, Color.FromArgb(colors[ox, oy]));
                    show();
                    x1 = (x1 + 8) & ~7;
                }
            }
        }
        y = (y + 8) & ~7;
    }
}

void drawrect8(Bitmap bitmap, int x1, int y1, int x2, int y2, int[,] colors)
{
    while (y1 < (y2 & ~7))
    {
        var x = x1;
        while (x < (x2 & ~7))
        {
            var ox = x1 & 7;
            var oy = y1 & 7;
            var color = colors[ox, oy];

            bitmap.SetPixel(x / 8, y1 / 8, Color.FromArgb(color));
            x = (x + 8) & ~7;
        }
        if (x < x2)
        {
            var ox = 7 - x1 & 7;
            var oy = y1 & 7;
            var color = colors[ox, oy];

            bitmap.SetPixel(x / 8, y1 / 8, Color.FromArgb(color));
        }
        y1 = (y1 + 8) & ~7;
    }
    if (y1 < y2)
    {
        var x = x1;
        var oy = 7 - y2 & 7;
        while (x < (x2 & ~7))
        {
            var ox = x1 & 7;
            var color = colors[ox, oy];

            bitmap.SetPixel(x / 8, y2 / 8, Color.FromArgb(color));
            x = (x + 8) & ~7;
        }
        if (x < x2)
        {
            var ox = 7 - x1 & 7;
            oy = 7 - y2 & 7;
            var color = colors[ox, oy];

            bitmap.SetPixel(x / 8, y2 / 8, Color.FromArgb(color));
        }
    }
}

void show()
{
    //panel.Show(bitmap, 0, 0, 64, 64);
}

drawline(bitmap, 12, 31.3f, 52, 32.7f, 3, 0xFF0000);
//drawline(bitmap, 30, 10, 34, 40, 5, 0xff0000);
show();

for (int i = 0; i <= 360; i += 1)
{
    bitmap = new Bitmap(128, 64);
    var angle = (float)(i / 180f * Math.PI);

    var (cx, cy) = RotatedCenter(32f * 0.2f, angle);
    var (ex, ey) = RotatedCenter(32f * 0.6f, angle);
    drawline(bitmap, cx, cy, ex, ey, 3, 0xff0000);
    panel.Show(bitmap, 0, 0, 64, 64);
}

(float, float) RotatedCenter(float distance, float angle)
{
    var x = (float)(32 + distance * Math.Cos(angle));
    var y = (float)(32 + distance * Math.Sin(angle));
    return (x, y);
}
