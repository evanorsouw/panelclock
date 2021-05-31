using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class SegmentClock : IDrawable
    {
        private float _height;
        private float _width;
        private float _x;
        private float _y;
        private float _skew;
        private float _thickness;
        private Color _color;
        private Brush _brushOn;
        private Brush _brushOff;
        private bool _includeSeconds;
        private const float _digitSpacing = 0.1f;
        private const float _dotSpacing = 0.25f;

        public SegmentClock() : this(60, 0, 0)
        {
        }

        public SegmentClock(float height, float x, float y)
        {
            _height = height;
            X = x;
            Y = y;
            Skew = 0.1f;
            Thickness = 0.3f;
            Color = Color.Red;
        }

        public float Height { get { return _height; } set { SetHeight(value); } }
        public float Width { get { return _width; } }

        public float X { get { return _x; } set { _x = value; } }
        public float Y { get { return _y; } set { _y = value; } }
        public float Skew { get { return _skew; } set { SetSkew(value); } }
        public float Thickness{ get { return _thickness; } set { SetThickness(value); } }
        public Color Color { get { return _color;  } set { SetColor(value); } }
        public bool IncludeSeconds { get { return _includeSeconds; } set { SetIncludeSeconds(value); } }

        public void Draw(Graphics g)
        {
            var x = _x;
            var y = _y;
            if (_skew < 0)
            {
                x += -_skew * _height;
            }
            var now = DateTime.Now;
            var doton = now.Millisecond < 500;

            DrawSegments(g, now.Hour / 10, x, y);
            x += _height * (0.5f + _digitSpacing);
            DrawSegments(g, now.Hour % 10, x, y);
            x += _height * 0.5f;
            DrawDots(g, doton, x, y);
            x += _height * (_dotSpacing);
            DrawSegments(g, now.Minute / 10, x, y);
            x += _height * (0.5f + _digitSpacing);
            DrawSegments(g, now.Minute % 10, x, y);
            if (IncludeSeconds)
            {
                x += _height * 0.5f;
                DrawDots(g, doton, x, y);
                x += _height * _dotSpacing;
                DrawSegments(g, now.Second / 10, x, y);
                x += _height * (0.5f + _digitSpacing);
                DrawSegments(g, now.Second % 10, x, y);
            }
        }

        private void DrawSegments(Graphics graphics, int digit, float x, float y)
        {
            graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            for (int i=0; i<7; ++i)
            {
                graphics.FillPolygon(IsOn(digit, i) ? _brushOn: _brushOff, CreateSegment(i, x, y));
            }
        }

        private void DrawDots(Graphics graphics, bool on, float x, float y)
        {
            var diameter = 1.0f + (_height * 0.10f * _thickness);

            var offset1 = 0.3f;
            var offset2 = 0.6f;
            x += (_height * _dotSpacing - diameter) / 2;
            graphics.FillEllipse(on ? _brushOn : _brushOff, x + _height * _skew * (1.0f-offset1), y + _height * offset1, diameter, diameter);
            graphics.FillEllipse(on ? _brushOn : _brushOff, x + _height * _skew * (1.0f-offset2), Y + _height * offset2, diameter, diameter);
        }

        private class Segments
        {
            public Segments(byte s1, byte s2, byte s3, byte s4, byte s5, byte s6, byte s7)
            {
                segment[0] = s1;
                segment[1] = s2;
                segment[2] = s3;
                segment[3] = s4;
                segment[4] = s5;
                segment[5] = s6;
                segment[6] = s7;
            }
            public byte[] segment = new byte[7];
        }
        private Segments[] _digitSegments = new Segments[] {
            new Segments(1,1,1,1,1,1,0),
            new Segments(0,1,1,0,0,0,0),
            new Segments(1,1,0,1,1,0,1),
            new Segments(1,1,1,1,0,0,1),
            new Segments(0,1,1,0,0,1,1),
            new Segments(1,0,1,1,0,1,1),
            new Segments(1,0,1,1,1,1,1),
            new Segments(1,1,1,0,0,0,0),
            new Segments(1,1,1,1,1,1,1),
            new Segments(1,1,1,1,0,1,1),
        };

        private bool IsOn(int digit, int segment)
        {
            return _digitSegments[digit].segment[segment] == 1;
        }

        private PointF[] CreateSegment(int seg, float x, float y)
        {
            var x0 = x + _height * _skew;
            var y0 = y;
            var x1 = x0 + _height / 2;
            var y1 = y0;
            var x2 = x + _height * _skew / 2;
            var y2 = y + _height / 2;
            var x3 = x2 + _height / 2;
            var y3 = y2;
            var x4 = x;
            var y4 = y + _height;
            var x5 = x4 + _height / 2;
            var y5 = y4;

            var thickness = _height * _thickness * 0.20f;
            var offset = thickness / 6f;
            switch (seg)
            {
                case 0:
                    return new PointF[] {
                        new PointF(x0+offset*7, y0+offset*6),
                        new PointF(x0+offset*3, y0+offset*2),
                        new PointF(x0+offset*5, y0+offset*0),
                        new PointF(x1-offset*5, y1+offset*0),
                        new PointF(x1-offset*3, y1+offset*2),
                        new PointF(x1-offset*7, y1+offset*6)
                    };
                case 1:
                    return new PointF[] {
                        new PointF(x1-offset*6, y1+offset*7),
                        new PointF(x1-offset*2, y1+offset*3),
                        new PointF(x1-offset*0, y1+offset*5),
                        new PointF(x3-offset*0, y3-offset*4),
                        new PointF(x3-offset*3, y3-offset*1),
                        new PointF(x3-offset*6, y3-offset*4)
                    };
                case 2:
                    return new PointF[] {
                        new PointF(x3-offset*6, y3+offset*4),
                        new PointF(x3-offset*3, y3+offset*1),
                        new PointF(x3-offset*0, y3+offset*4),
                        new PointF(x5-offset*0, y5-offset*5),
                        new PointF(x5-offset*2, y5-offset*3),
                        new PointF(x5-offset*6, y5-offset*7)
                    };
                case 3:
                    return new PointF[] {
                        new PointF(x5-offset*7, y5-offset*6),
                        new PointF(x5-offset*3, y5-offset*2),
                        new PointF(x5-offset*5, y5-offset*0),
                        new PointF(x4+offset*5, y4-offset*0),
                        new PointF(x4+offset*3, y4-offset*2),
                        new PointF(x4+offset*7, y4-offset*6)
                    };
                case 4:
                    return new PointF[] {
                        new PointF(x2+offset*0, y2+offset*4),
                        new PointF(x2+offset*3, y2+offset*1),
                        new PointF(x2+offset*6, y2+offset*4),
                        new PointF(x4+offset*6, y4-offset*7),
                        new PointF(x4+offset*2, y4-offset*3),
                        new PointF(x4+offset*0, y4-offset*5)
                    };
                case 5:
                    return new PointF[] {
                        new PointF(x0+offset*0, y0+offset*5),
                        new PointF(x0+offset*2, y0+offset*3),
                        new PointF(x0+offset*6, y0+offset*7),
                        new PointF(x2+offset*6, y2-offset*4),
                        new PointF(x2+offset*3, y2-offset*1),
                        new PointF(x2+offset*0, y2-offset*4)
                    };
                case 6:
                    return new PointF[] {
                        new PointF(x2+offset*7, y2+offset*3),
                        new PointF(x2+offset*4, y2+offset*0),
                        new PointF(x2+offset*7, y2-offset*3),
                        new PointF(x3-offset*7, y3-offset*3),
                        new PointF(x3-offset*4, y3-offset*0),
                        new PointF(x3-offset*7, y3+offset*3)
                    };
            }
            return new PointF[0];
        }

        private void SetHeight(float height)
        {
            _height = height;
            CalculateWidth();
        }

        private void SetSkew(float skew)
        {
            _skew = (float)Math.Max(Math.Min(0.1, skew), -0.1);
            CalculateWidth();
        }

        private void SetThickness(float thickness)
        {
            _thickness = (float)Math.Max(Math.Min(1, thickness), 0.01);
        }

        private void SetColor(Color color)
        {
            _color = color;
            _brushOn = new SolidBrush(color);
            var scale = 0.2f;
            _brushOff = new SolidBrush(Color.FromArgb((int)(color.R * scale), (int)(Color.G * scale), (int)(Color.B * scale)));
        }

        private void SetIncludeSeconds(bool include)
        {
            _includeSeconds = include;
            CalculateWidth();
        }

        private void CalculateWidth()
        {
            _width = _height * (IncludeSeconds ? 3 : 2) +
                _height * _digitSpacing * (IncludeSeconds ? 3 : 2) +
                _height * _dotSpacing * (IncludeSeconds ? 2 : 1) +
                Math.Abs(_skew) * _height;
        }
    }
}
    