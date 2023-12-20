using System;
using Microsoft.Extensions.Logging;

namespace WhiteMagic.PanelClock.Components
{
    public class TimestampSource : ValueSource
    {
        private ILogger _logger;
        Func<DateTime> _whenGetter;

        public TimestampSource(ILogger logger, Func<DateTime> whenGetter) : this("", logger, whenGetter)
        {
        }

        public TimestampSource(string name, ILogger logger, Func<DateTime> whenGetter) : base(name, () => whenGetter())
        {
            _whenGetter = whenGetter;
            _logger = logger;

            AddProperty(Create("fseconds", () =>
            {
                var when = _whenGetter();
                return when.Second + when.Millisecond / 1000f;
            }));
            AddProperty(Create("seconds", () => _whenGetter().Second));
            AddProperty(Create("minutes", () => _whenGetter().Minute));
            AddProperty(Create("hours", () => _whenGetter().Hour));
            AddProperty(Create("wday", () => _whenGetter().DayOfWeek));
            AddProperty(Create("mday", () => _whenGetter().Day-1));
            AddProperty(Create("yday", () => _whenGetter().DayOfYear-1));
            AddProperty(Create("month", () => _whenGetter().Month-1));
            AddProperty(Create("year", () => _whenGetter().Year));
        }
    }
}