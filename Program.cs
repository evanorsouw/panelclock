using System;
using System.Threading;
using System.Drawing;
using System.Collections.Generic;
using Microsoft.Extensions.Configuration;
using NLog.Extensions.Logging;
using Microsoft.Extensions.Logging;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    class Program
    {
        static void Main(string[] args)
        {
            string port = null;

            int iArg = 0;
            for (iArg = 0; iArg < args.Length; ++iArg)
            {
                if (args[iArg] == "--port")
                {
                    AssertArgument(args, iArg);
                    port = args[++iArg];
                }
                else
                {
                    Usage($"unsupported argument '{args[iArg]}'");
                }
            }

            var config = new ConfigurationBuilder()
                .SetBasePath(System.IO.Directory.GetCurrentDirectory())
                .AddJsonFile("appsettings.json", optional: true, reloadOnChange: true)
                .Build();

            var factory = LoggerFactory.Create(builder =>
            {
                builder
                    .ClearProviders()
                    .AddNLog("nlog.config");
            });

            var logger = factory.CreateLogger<Program>();
            logger.LogInformation("ledpanelwriter started");

#if SIMULATION
            IDisplay display = new Display(128, 64);
#else
            if (port == null)
                Usage("port not specified");
            IDisplay display = new LedPanelDisplay(port, 128, 64);
#endif
            var images = new FileImageSource("NAS", @"\\nas\photo", logger);
            
            items = new List<IDrawable>();

            //var analog = new AnalogClock(46, 127 - 46, 63 - 46);
            var analog = new AnalogClock(64, 0, 0);
            var digital = new DigitalClock(12, 86, 0);
            var segment = new SegmentClock(14, 86, 0);
            var textpanel = new TextPanel(0,0,128,64);
            var imageviewer = new ImageViewer(0, 0, 128, 64, logger);

            digital.IncludeSeconds = false;
            analog.SmoothSeconds = false;
            segment.IncludeSeconds = true;
            segment.Thickness = 0.5f;
            segment.Skew = 0.5f;
            segment.X = 127 - segment.Width;
            segment.Y = 1;

            imageviewer.ImageSource = images;
            
            textpanel.Title = "Verjaardg."; 
            textpanel.AllFonts.Typeface = "Tahoma" ;
            textpanel.AllFonts.Height = 10;

            var calendar = new Calendar();
            var events = calendar.GetEventsForDate(DateTime.Now);

            foreach(var evt in events.OrderBy(e=>e.When).ThenByDescending(e=>e.AllDay))
            {
                if (evt.AllDay)
                {
                    textpanel.AddItem("", evt.Message);
                }
                else 
                {
                    textpanel.AddItem($"{evt.When.Hour:D2}:{evt.When.Minute:D2}", evt.Message);
                }
            }

            //items.Add(imageviewer);
            //items.Add(textpanel);
            items.Add(analog);
            items.Add(digital);
            //items.Add(segment);

            var lastTime = DateTime.Now;
            var updateInterval = 40;
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


                var anim = ((now.Ticks / 20000) % 4000) / 1000f;
                if (anim < 2f)
                    anim = Math.Min(anim, 1f);
                else
                    anim = 3f - Math.Min(anim, 3f);

                //digital.Rotation = anim / 4f;
                //digital.Y = anim * 108;
                //digital.X = 128 - digital.BWidth;
                //digital.Height = 12 + anim * 7;
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

        private static void AssertArgument(string[] args, int iArg)
        {
            if (iArg + 1 >= args.Length)
                Usage($"Missing argument for option '{args[iArg]}'");

        }

        private static void Usage(string message=null)
        {
            Console.WriteLine($"Error: {message}");
            Console.WriteLine("Usage:");
            Console.WriteLine("  writer [options]");
            Console.WriteLine("Options:");
            Console.WriteLine("  --port <port>     - the USB port that the LED displays are connected");
            Environment.Exit(1);
        }
    }
}
