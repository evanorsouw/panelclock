using System;
using System.Threading;
using System.Drawing;
using System.Collections.Generic;

namespace WhiteMagic.PanelClock
{
    class Program
    {
        static void Main(string[] args)
        {
            IDisplay screen = new Display(128, 64);
            var ledpanel = new LedPanelDisplay("COM4", 64, 64);
            IDisplay display = screen;

            var images = new FileImageSource("NAS", @"\\nas\photo");

            items = new List<IDrawable>();

            var analog = new AnalogClock(64, 0, 0);
            var digital = new DigitalClock(20, 62, 0);
            var segment = new SegmentClock(20, 20, 20);
            var textpanel = new TextPanel(0,0,64,64);
            var imageviewer = new ImageViewer(0, 0, 128, 64);

            digital.IncludeSeconds = false;
            analog.SmoothSeconds = false;
            segment.Thickness = 0.4f;
            segment.Skew = 0.5f;
            segment.X = 128 - segment.Width;
            segment.Y = 63 - segment.Height;
            segment.IncludeSeconds = true;

            imageviewer.ImageSource = images;
            
            textpanel.Title = "Verjaardg."; 
            textpanel.AllFonts.Typeface = "Tahoma" ;
            textpanel.AllFonts.Height = 10;
            //panel.ItemFont.Height = 9;
            textpanel.AddItem("07:15", "eric")
                .AddItem("08:00", "Stage s")
                .AddItem("08:30", "Inge Ctac BE in NL")
                .AddItem("09:00", "J Fysio Fit")
                .AddItem("17:30", "Dominos")
                .AddItem("18:10", "Afspraakbevestiging")
                .AddItem("19:00", "Koelkast schoonmaken voordat het niet meer lukt!")
                .AddItem("Jeanette koken")
                .AddItem("Frank Bielschovsky '66")
                ;

            items.Add(imageviewer);
            //items.Add(textpanel);
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

                ledpanel.SetBrightness((now.Second & 1)==0?64:255);
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
