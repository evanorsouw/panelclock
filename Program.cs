using System;
using System.Threading;
using System.Drawing;
using System.Collections.Generic;
using Microsoft.Extensions.Configuration;
using NLog.Extensions.Logging;
using Microsoft.Extensions.Logging;
using System.Linq;
using writer;

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
            items = new List<IDrawable>();

            var analog = new AnalogClockModern(64, 0, 0);
            analog.SmoothSeconds = true;
            analog.AnimationTime = 0.66f;

            var digital = new VarLabel();
            digital.X = 128;
            digital.HorizontalAlignment = Alignment.Right;
            digital.Format = "hours(24H):minutes()";
            digital.FontName = "Tahoma:Bold:14";

            var ticker = new Ticker(128, 16);
            ticker.Y = 48;
            ticker.X = 54;
            ticker.Text = "Hello world! abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            ticker.Speed = 7f;

            var label1 = new VarLabel();
            label1.FontName = "Tahoma:9";
            label1.X = 128;
            label1.Y = 15;
            label1.VerticalAlignment = Alignment.Top;
            label1.HorizontalAlignment = Alignment.Right;
            label1.Format = "wday([Maandag,Dinsdag,Woensdag,Donderdag,Vrijdag,Zaterdag,Zondag])";

            var label2 = new VarLabel();
            label2.FontName = "Tahoma:9";
            label2.X = 128;
            label2.Y = 25;
            label2.VerticalAlignment = Alignment.Top;
            label2.HorizontalAlignment = Alignment.Right;
            label2.Format = "mday(#) month([Jan,Feb,Mar,Apr,Mei,Jun,Jul,Aug,Sep,Okt,Nov,Dec])";

            items.Add(analog);
            items.Add(digital);
//            items.Add(ticker);
            items.Add(label1);
            items.Add(label2);

            var lastTime = DateTime.Now;
            var updateInterval = 40;

            logger.LogInformation("starting loop");

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

                //label1.Visible =
                //label2.Visible =
                //analog.Visible = (now.Second % 10) > 5;

                var bitmap = RenderDisplay(display.Width, display.Height);
                display.Show(bitmap);
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
