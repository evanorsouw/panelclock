using Microsoft.Extensions.Logging;
using System;

namespace WhiteMagic.PanelClock
{
    public class NowSource : TimestampSource
    {
        public static string Name => "now";

        public NowSource(ILogger logger): base(Name, logger, () => DateTime.Now)
        {
        }
    }
}