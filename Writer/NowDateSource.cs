using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class NowDateSource : ValueSource
    {
        public NowDateSource(): base("now", typeof(DateTime))
        {
            AddProperty(Create("seconds", typeof(int), () => DateTime.Now.Second));
            AddProperty(Create("minutes", typeof(int), () => DateTime.Now.Minute));
            AddProperty(Create("hours", typeof(int), () => DateTime.Now.Hour));
        }
    }
}