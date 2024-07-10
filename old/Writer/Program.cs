
using System;
using System.Threading;
using System.Drawing;
using Microsoft.Extensions.Configuration;
using NLog.Extensions.Logging;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Primitives;
using System.Threading.Tasks;
using WhiteMagic.PanelClock.Components;
using WhiteMagic.PanelClock.Display;
using WhiteMagic.PanelClock.Engine;
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

            var factory = LoggerFactory.Create(builder =>
            {
                builder
                    .ClearProviders()
                    .AddNLog("nlog.config");
            });

            var logger = factory.CreateLogger<Program>();
            logger.LogInformation("ledpanelwriter started");

            IConfiguration config = new ConfigurationBuilder()
                .SetBasePath(System.IO.Directory.GetCurrentDirectory())
#if true
                .AddJsonFile("d:\\projects\\fpgaclock\\sw\\Writer\\appsettings.json", optional: true, reloadOnChange: true)
#else
                .AddJsonFile("appsettings.json", optional: true, reloadOnChange: true)
#endif
                .Build();

            var environment = new EnvironmentSource(logger, config);
            IFunctionFactory functions = new FunctionFactory(environment, logger);
            var stock = new ConfigurationParser(config, functions, environment, logger).Parse();
            Animator animator = new Animator(stock);

            ChangeToken.OnChange(
                () => config.GetReloadToken(),
                async () => {
                    await Task.Delay(1000);
                    stock = new ConfigurationParser(config, functions, environment, logger).Parse();
                    animator = new Animator(stock);
                });

#if SIM_DISPLAY
            IDisplay display = new Display.Display(128, 64);
#else
            if (port == null)
                Usage("port not specified");
            IDisplay display = new LedPanelDisplay(port, 128, 64);
#endif
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

                var bitmap = new Bitmap(display.Width, display.Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
                animator.Render(bitmap, now);

                display.Show(bitmap);
            }
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
