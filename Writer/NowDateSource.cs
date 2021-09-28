using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class NowDateSource : ValueSource
    {
        public static string Name => "now";

        public NowDateSource(List<ValueSource> arguments): base(Name)
        {
            if (arguments.Count > 0)
                throw new Exception("now() does not expected any arguments");
            AddProperty(Create("fseconds", () => {
                var now = DateTime.Now;
                return now.Second + now.Millisecond / 1000f;
            }));
            AddProperty(Create("seconds", () => DateTime.Now.Second));
            AddProperty(Create("minutes", () => DateTime.Now.Minute));
            AddProperty(Create("hours", () => DateTime.Now.Hour));
        }
    }
}