using Microsoft.Extensions.Logging;
using System;
using System.Drawing;
using WhiteMagic.PanelClock;

namespace WhiteMagic.PanelClock
{
    public class Ticker : Label
    {
        private int _loops = 1;
        private double _scrollSpeed = 1;
        private int _loopCountdown;
        private DateTime _loopStart;

        public Ticker(string id, ILogger logger) : base(id, logger)
        {
            AddProperty(ValueSource.Create("scrollspeed", () => _scrollSpeed, (obj) => _scrollSpeed = obj));
            AddProperty(ValueSource.Create("loops", () => _loops, (obj) => _loops = obj));
        }

        public void Draw(Graphics g)
        {
            base.Draw(g);
            //var now = DateTime.Now;
            //if (_bitmap != null)
            //{
            //    var elapsed = now.Subtract(_loopStart).TotalSeconds;
            //    var offset = (float)(elapsed * _speed);
            //    if (offset < _width)
            //    {
            //        g.DrawImage(_bitmap, new RectangleF(_x + _width - offset, _y, _width, _height), new RectangleF(0, 0, _width, _bitmap.Height), GraphicsUnit.Pixel);
            //    }
            //    else
            //    {
            //        g.DrawImage(_bitmap, new RectangleF(_x, _y, _width, _height), new RectangleF(offset - _width, 0, _width, _bitmap.Height), GraphicsUnit.Pixel);
            //    }
            //    if (offset < _bitmap.Width + _width)
            //        return;

            //    _loopStart = now;
            //    if (--_loopCountdown > 0)
            //        return;
            //}
            //if (_nextBitmap != null)
            //{
            //    _bitmap = _nextBitmap;
            //    _nextBitmap = null;
            //    _loopCountdown = _loops;
            //    _loopStart = now;
            //}
        }
    }
}