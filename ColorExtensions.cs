using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public static class ColorExtensions
    {
        public static Color Scale(this Color c, float scale)
        {
            if (scale >= 1f)
                return c;
            return Color.FromArgb((byte)(c.R * scale), (byte)(c.G * scale), (byte)(c.B * scale));
        }
    }
}
