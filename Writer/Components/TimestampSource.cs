using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;

namespace WhiteMagic.PanelClock
{
    public class TimestampSource : ValueSource
    {
        private ILogger _logger;
        Func<DateTime> _whenGetter;

        public TimestampSource(ILogger logger, Func<DateTime> whenGetter) : this("", logger, whenGetter)
        {
        }

        public TimestampSource(string name, ILogger logger, Func<DateTime> whenGetter) : base(name)
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
        }
    }
}