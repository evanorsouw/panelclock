using System;
using Microsoft.Extensions.Logging;

namespace WhiteMagic.PanelClock.Components
{
    public class NowSource : TimestampSource
    {
        public static string Name => "now";

        public NowSource(ILogger logger): base(Name, logger, () => DateTime.Now)
        {
        }
    }
}