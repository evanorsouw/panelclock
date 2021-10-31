using Microsoft.Extensions.Logging;
using System;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class Ticker : Label
    {
        private int _nBars = -1;
        private DateTime _startTime;
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
            BarAnimateOverlap = 0.7f;
            DividerColor = Color.White.Scale(0.5f);
            BackgroundColor = Color.Black;
            ScrollSpeed = 18;
            DirectVisibility = false;
            VerticalAlignment = Alignment.Center;
            DirectVisibility = true;
            ExternalVisible = false;

            AddProperty(Create("scrollspeed", () => ScrollSpeed, (obj) => ScrollSpeed = obj));
            AddProperty(Create("loops", () => Loops, (obj) => Loops = obj));
            AddProperty(Create("bars", () => Bars, (obj) => Bars = obj));
            AddProperty(Create("baranimateoverlap", () => BarAnimateOverlap, (obj) => BarAnimateOverlap = obj));
            AddProperty(Create("dividercolor", () => DividerColor, (obj) => DividerColor = obj));
        }

        #region Properties

        public int ScrollSpeed { get; set; }
        public int Loops { get; set; }
        public int Bars { get { return _nBars; } set { SetBars(value); } }
        public float BarAnimateOverlap { get; set; }
        public Color DividerColor { get; set; }

        #endregion

        public override void Draw(Graphics graphics)
        {
            if (!InternalVisible && !ShowingOrHiding)
                return;

            var y = Y1;
            if (ShowingOrHiding)
            {
                _startTime = DateTime.Now;
                var x = X;
                var h = Height;
                var elapsed = ShowOrHideAnimationElapsed;
                for (int i=0; i<_nBars; ++i)
                {
                    var w = _barWidths[i] * Width;
                    var relStart = (1 - BarAnimateOverlap) / (Bars / 2) * Math.Abs(i - Bars / 2);
                    var relative = 0f;
                    Relative(elapsed, relStart, relStart + BarAnimateOverlap, ref relative);
                    var rel = 0.0f;
                    if (Relative(relative, 0.0f, 0.33f, ref rel))
                    {
                        var pen = new Pen(DividerColor.ScaleAlpha(rel), 1);
                        graphics.DrawLine(pen, x, y + h / 2, x + w, y + h / 2);
                    }
                    else if (Relative(relative, 0.33f, 0.66f, ref rel))
                    {
                        var y1 = (float)(y + h / 2 - rel * h / 2);
                        var y2 = (float)(y + h / 2 + rel * h / 2);
                        graphics.FillRectangle(new SolidBrush(_barColors[i]), x, y1, w, y2 - y1);
                        var pen = new Pen(DividerColor, 1);
                        graphics.DrawLine(pen, x, y1, x + w, y1);
                        graphics.DrawLine(pen, x, y2, x + w, y2);
                    }
                    else
                    {
                        Relative(relative, 0.66f, 1.0f, ref rel);
                        var y1 = (float)(y + h / 2 - rel * h / 2);
                        var y2 = (float)(y + h / 2 + rel * h / 2);
                        var brush = new SolidBrush(_barColors[i].Scale(1 - rel));
                        graphics.FillRectangle(brush, x, y, w, y1 - y);
                        graphics.FillRectangle(new SolidBrush(BackgroundColor), x, y1, w, y2 - y1);
                        graphics.FillRectangle(brush, x, y2, w, y + h - y2);
                        var pen = new Pen(DividerColor, 1);
                        graphics.DrawLine(pen, x, y, x + w, y);
                        graphics.DrawLine(pen, x, y + h, x + w, y + h);
                    }
                    x += w;
                }
            }
            else if (InternalVisible)
            {
                var pen = new Pen(DividerColor);
                graphics.FillRectangle(new SolidBrush(BackgroundColor), X, y, Width, Height);
                graphics.DrawLine(pen, X, y , X+ Width, y);
                graphics.DrawLine(pen, X, y + Height, X + Width, y + Height);

                EvaluateText();

                var scrollWidth = TextWidth + Width;
                var elapsed = DateTime.Now.Subtract(_startTime).TotalSeconds;
                var pixels = elapsed * ScrollSpeed;
                var loops = (int)(pixels / scrollWidth);
                InternalVisible = Loops == 0 || loops < Loops;
                var tx = Width - (float)(pixels % scrollWidth);
                var ty = ContentTopLeft.Y;

                graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
                graphics.SetClip(BackgroundBox);
                graphics.DrawString(Text, Font, new SolidBrush(TextColor), tx, ty);

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