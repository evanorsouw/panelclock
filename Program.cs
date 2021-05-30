using System;
using System.IO.Ports;
using System.Threading;
using System.Drawing;
using System.Collections.Generic;
using System.Windows.Forms;

namespace WhiteMagic.PanelClock
{
    class Program
    {
        static void Main(string[] args)
        {
            IDisplay display = new Display(128,64);

            items = new List<IDrawable>();

            var analog = new AnalogClock(64, 0, 0);
            var digital = new DigitalClock(20, 62, 0);
            digital.IncludeSeconds = false;
            analog.SmoothSeconds = true;

            items.Add(analog);
            items.Add(digital);

            var lastTime = DateTime.Now;
            var updateInterval = 25;
            while (true)
            {
                var now = DateTime.Now;
                var elapsed = (int)(now.Subtract(lastTime).TotalSeconds * 1000);
                if (elapsed < updateInterval)
                {
                    var delay = updateInterval - elapsed;
                    Console.WriteLine($"wait={delay}");
                    Thread.Sleep(delay);
                }
                lastTime = now;

                var bitmap = RenderDisplay(display.Width, display.Height);
                display.Show(bitmap);

                var anim = ((now.Ticks / 10000) % 10000) / 1000f;
                if (anim < 5f)
                    anim = Math.Min(anim, 1f);
                else
                    anim = 6f - Math.Min(anim, 6f);
                digital.Rotation = anim / 4f;
                digital.Y = 0;
                digital.X = 128 - digital.BWidth;
                digital.Height = 12 + (1f - anim) * 6;
            }
        }

            static List<IDrawable> items = new List<IDrawable>();

        static Bitmap RenderDisplay(int width, int height)
        {
            var bitmap = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
            Graphics g = Graphics.FromImage(bitmap);

            foreach(var item in items)
            {
                item.Draw(g);
            }

            g.Flush();
            return bitmap;
        }

        static string ToBin8(int v)
        {
            var s = "00000000" + Convert.ToString(v, 2);
            return s.Substring(s.Length - 8, 8);
        }

        static byte[] _logmap;
        static byte MapIntensity(int b)
        {
            if (_logmap == null)
            {
                _logmap = new byte[256];
                for (int i=0; i<256; ++i)
                {
                    _logmap[i] = (byte)(127 * (1 - Math.Log(256 - i) / Math.Log(256)));
                }
            }
            return _logmap[(byte)b];
        }
    }
}
