using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class ColorSource : ValueSource
    {
        public static string Name => "now";
        public ValueSource _red;
        public ValueSource _green;
        public ValueSource _blue;

        public ColorSource(List<ValueSource> arguments): base(Name)
        {
            if (arguments.Count != 3)
                throw new Exception("color(red,green,blue) expects 3 arguments");

            _red = arguments[0];
            _green = arguments[1];
            _blue = arguments[2];
            SetGetter(() => Color.FromArgb(ScaleChannel(_red.Value), ScaleChannel(_green.Value), ScaleChannel(_blue.Value)));
        }

        private int ScaleChannel(double v)
        {
            var channel = (int)(v *= 255);
            if (channel < 0)
                return 0;
            if (channel > 255)
                return 255;
            return channel;
        }
    }
}