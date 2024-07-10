using System;
using System.Drawing;

namespace WhiteMagic.PanelClock.Extensions
{
    public static class ColorExtensions
    {
        public static Color Scale(this Color c, float scale)
        {
            var r = (byte)Math.Min(255, c.R * scale);
            var g = (byte)Math.Min(255, c.G * scale);
            var b = (byte)Math.Min(255, c.B * scale);
            return Color.FromArgb(c.A, r, g, b);
        }

        public static Color ScaleAlpha(this Color c, float scale)
        {
            var a = (byte)Math.Min(255, c.A * scale);
            return Color.FromArgb(a, c.R, c.G, c.B);
        }
    }
}
