using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class NowDateSource : ValueSource
    {
        public NowDateSource(): base("now")
        {
            AddProperty(Create("seconds", () => DateTime.Now.Second));
            AddProperty(Create("minutes", () => DateTime.Now.Minute));
            AddProperty(Create("hours", () => DateTime.Now.Hour));
        }
    }
}