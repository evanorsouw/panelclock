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

            var analog = new AnalogClock(44, 64, 0);
            var digital = new DigitalClock(20, 62, 0);
            var segment = new SegmentClock(20, 20, 20);
            var panel = new TextPanel(0,0,96,64);
            digital.IncludeSeconds = false;
            analog.SmoothSeconds = false;
            segment.Thickness = 0.4f;
            segment.Skew = 0.5f;
            segment.X = 128 - segment.Width;
            segment.Y = 63 - segment.Height;
            segment.IncludeSeconds = true;

            panel.Title = "Verjaardagen";
            panel.TitleHeight = 11;
            panel.AddItem("eric")
                .AddItem("08:00", "Stage s")
                .AddItem("09:00", "J Fysio Fit")
                .AddItem("Jeanette koken")
                .AddItem("17:30", "Dominos")
                .AddItem("15:20", "555555")
                .AddItem("Frank Bielschovsky '66")
                .AddItem("19:00", "Koelkast schoonmaken voordat hij weer verstopt raakt!")
                .AddItem("08:30", "Inge Ctac BE in NL")
                .AddItem("18:10", "Afspraakbevestiging")
                .AddItem("18:10", "J vaccin 1");

            items.Add(panel);
            //items.Add(analog);
            items.Add(digital);
            //items.Add(segment);

            var lastTime = DateTime.Now;
            var updateInterval = 25;
            while (true)
            {
                var now = DateTime.Now;
                var elapsed = (int)(now.Subtract(lastTime).TotalSeconds * 1000);
                if (elapsed < updateInterval)
                {
                    var delay = updateInterval - elapsed;
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
                digital.Height = 12 + anim * 7;

                segment.Height = 18;
                segment.IncludeSeconds =  (now.Second % 6) > 2;
                segment.X = 126 - segment.Width;
                segment.Y = 62 - segment.Height;
                segment.Thickness = 0.5f;
                segment.Skew = 0.1f;
            }
        }

        static List<IDrawable> items = new List<IDrawable>();

        static Bitmap RenderDisplay(int width, int height)
        {
            var bitmap = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            foreach(var item in items)
            {
                Graphics g = Graphics.FromImage(bitmap);
                item.Draw(g);
                g.Flush();
            }

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
