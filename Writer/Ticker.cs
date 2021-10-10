using Microsoft.Extensions.Logging;
using System;
using System.Drawing;
using System.Linq;
using WhiteMagic.PanelClock;

namespace WhiteMagic.PanelClock
{
    public class Ticker : Label
    {
        private int _nBars = -1;
        private int _loopCountdown;
        private DateTime _loopStart;
        private float[] _barWidths;
        private Color[] _barColors;
        private static Color[] _palette;

        static Ticker()
        {
            _palette = new Color[] {
                Color.FromArgb(251,200,33),
                Color.FromArgb(167,234,95),
                Color.FromArgb(3,67,33),
                Color.FromArgb(0,128,127),
                Color.FromArgb(74,102,202),
                Color.FromArgb(22,196,173),
                Color.FromArgb(97,64,81),
                Color.FromArgb(188,53,23),
                Color.FromArgb(210,31,11),
                Color.FromArgb(252,58,31),
                Color.FromArgb(255,141,105),
                Color.FromArgb(255,229,180),
                Color.FromArgb(205,128,50),
                Color.FromArgb(211,175,55),
                Color.FromArgb(153,100,20),
                Color.FromArgb(101,66,34)
            };
        }

        public Ticker(string id, ILogger logger) : base(id, logger)
        {
            ShowOrHideTime = 1.0f;
            Bars = 13;
            BarAnimateOverlap = 0.7;
            DividerColor = Color.White.Scale(0.5);

            AddProperty(Create("scrollspeed", () => ScrollSpeed, (obj) => ScrollSpeed = obj));
            AddProperty(Create("loops", () => Loops, (obj) => Loops = obj));
            AddProperty(Create("bars", () => Bars, (obj) => Bars = obj));
            AddProperty(Create("baranimateoverlap", () => BarAnimateOverlap, (obj) => BarAnimateOverlap = obj));
            AddProperty(Create("dividercolor", () => DividerColor, (obj) => DividerColor = obj));
        }

        #region

        public int ScrollSpeed { get; set; }
        public int Loops { get; set; }
        public int Bars { get { return _nBars; } set { SetBars(value); } }
        public double BarAnimateOverlap { get; set; }
        public Color DividerColor { get; set; }

        #endregion

        public override void Draw(Graphics graphics)
        {
            if (!Visible && !ShowingOrHiding)
                return;

            if (ShowingOrHiding)
            {
                var x = X;
                var h = (float)Height;
                var elapsed = ShowOrHideAnimationElapsed;
                for (int i=0; i<_nBars; ++i)
                {
                    var w = _barWidths[i] * Width;
                    var relStart = (1 - BarAnimateOverlap) / (Bars / 2) * Math.Abs(i - Bars / 2);
                    double relative = 0;
                    Relative(elapsed, relStart, relStart + BarAnimateOverlap, ref relative);
                    var y = Y - h;
                    var rel = 0.0;
                    if (Relative(relative, 0.0, 0.3, ref rel))
                    {
                        var pen = new Pen(DividerColor.Scale(rel));
                        pen.EndCap = System.Drawing.Drawing2D.LineCap.NoAnchor;
                        pen.StartCap = System.Drawing.Drawing2D.LineCap.NoAnchor;
                        graphics.DrawLine(pen, x, y + h / 2, x + w, y + h / 2);
                    }
                    else if (Relative(relative, 0.3f, 0.6f, ref rel))
                    {
                        var y1 = (float)(y + h / 2 - rel * h / 2);
                        var y2 = (float)(y + h / 2 + rel * h / 2);
                        graphics.FillRectangle(new SolidBrush(_barColors[i]), x, y1, w, y2 - y1);
                        var pen = new Pen(DividerColor);
                        pen.EndCap = System.Drawing.Drawing2D.LineCap.NoAnchor;
                        pen.StartCap = System.Drawing.Drawing2D.LineCap.NoAnchor;
                        graphics.DrawLine(pen, x, y1, x + w, y1);
                    }
                    else
                    {
                        Relative(relative, 0.6f, 1.0f, ref rel);
                        var y1 = (float)(y + h / 2 - rel * h / 2);
                        var y2 = (float)(y + h / 2 + rel * h / 2);
                        graphics.FillRectangle(Brushes.Black, x, y, w, h);
                        var brush = new SolidBrush(_barColors[i].Scale(1 - rel));
                        graphics.FillRectangle(brush, x, y, w, y1 - y);
                        graphics.FillRectangle(brush, x, y2, w, y + h - y2);
                        var pen = new Pen(DividerColor);
                        pen.EndCap = System.Drawing.Drawing2D.LineCap.NoAnchor;
                        pen.StartCap = System.Drawing.Drawing2D.LineCap.NoAnchor;
                        graphics.DrawLine(pen, x, y, x + w, y);
                    }
                    x += w;
                }
            }
            else if (Visible)
            {
                var pen = new Pen(DividerColor);
                graphics.FillRectangle(Brushes.Black, X, Y - Height, Width, Height);
                graphics.DrawLine(pen, X, Y-Height, Width, Y-Height);
            }
        }

        #region private

        private void SetBars(int n)
        {
            if (n != _nBars)
            {
                var random = new Random(DateTime.Now.Second);
                _nBars = n;
                _barWidths = new float[n];
                _barColors = new Color[n];
                var iColor = random.Next(0, _palette.Length);
                for (int i = 0; i < n; ++i)
                {
                    _barWidths[i] = (float)(5+random.NextDouble());
                    _barColors[i] = _palette[(iColor + i) % _palette.Length];
                }
                var sum = _barWidths.Sum();
                for (int i = 0; i < n; ++i)
                {
                    _barWidths[i] /= sum;
                }
                var q = _barWidths.Sum();
            }
        }

        #endregion
    }
}