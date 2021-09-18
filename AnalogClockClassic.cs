using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class AnalogClockClassic : IDrawable
    {
        private DateTime _lastTime;

        public AnalogClockClassic() : this(64, 0, 0)
        {
        }

        public AnalogClockClassic(float diameter, float anchorX, float anchory)
        { 
            Diameter = diameter;
            X = anchorX;
            Y = anchory;
        }

        public float Diameter{ get; set; }

        public float X { get; set; }
        public float Y { get; set; }

        public float Rotation { get; set; }

        public bool ShowDate { get; set; } = true;

        public bool SmoothSeconds { get; set; } = false;

        public void Draw(Graphics g)
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;

            var now = DateTime.Now;
            var seconds = (float)(now.Second / 60.0);
            var minutes = (float)((now.Minute + seconds) / 60.0);
            var hours = (float)((now.Hour % 12 + minutes) / 12.0);

            for (int i = 0; i < 12; ++i)
            {
                var a = i / 12f;
                var p = new PointF[] {
                    RotatedCenter(0.940f, a),
                    RotatedCenter(0.97f, a+0.004f),
                    RotatedCenter(1.00f, a),
                    RotatedCenter(0.97f, a-0.004f)
                };
                g.FillPolygon(Brushes.Red, p);
            }

            var points = new PointF[] {
                RotatedCenter(0,0),
                RotatedCenter(0.35f,hours-0.03f),
                RotatedCenter(0.70f,hours),
                RotatedCenter(0.35f,hours+0.03f)
            };
            g.FillPolygon(Brushes.Goldenrod, points);

            points = new PointF[] {
                RotatedCenter(0,0),
                RotatedCenter(0.40f,minutes-0.02f),
                RotatedCenter(0.85f,minutes),
                RotatedCenter(0.40f,minutes+0.02f)
            };
            g.FillPolygon(Brushes.Goldenrod, points);

            float angle;
            if (SmoothSeconds)
            {
                angle = (now.Second + now.Millisecond / 1000f)/60f;
            }
            else
            {
                angle = now.Second / 60f;
                if (_lastTime.Second != now.Second)
                {
                    _lastTime = now;
                    angle += 0.003f;
                }
            }

            points = new PointF[] {
                RotatedCenter(-0.1f, angle),
                RotatedCenter(0.15f, angle-0.02f),
                RotatedCenter(1.0f, angle),
                RotatedCenter(0.15f, angle+0.02f)
            };
            g.FillPolygon(Brushes.White, points);

            if (ShowDate)
            {
                var brush = Brushes.DarkGray;
                var date = now.Day.ToString();
                var font = new Font("Tahoma", Diameter/6.0f, FontStyle.Regular, GraphicsUnit.Pixel);
                var size = g.MeasureString(date, font, 999);
                var quadrant = DateQuadrant(hours, minutes);
                var position = DateQuadrantToPosition(quadrant, size);
                g.DrawString(date, font, brush, position);
            }
        }

        private int DateQuadrant(float hours, float minutes)
        {
            var top = true;
            var bottom = true;
            var left = true;
            var right = true;
            CheckQuadrant(hours, ref top, ref right, ref bottom, ref left);
            CheckQuadrant(minutes, ref top, ref right, ref bottom, ref left);
            if (bottom)
                return 2;
            if (right)
                return 1;
            if (left)
                return 3;
            if (top)
                return 0;
            return -1;
        }

        private void CheckQuadrant(float angle, ref bool top, ref bool right, ref bool bottom, ref bool left)
        {
            if (angle > 0.875f || angle < 0.125f)
                top = false;
            if (angle > 0.125f && angle < 0.375f)
                right = false;
            if (angle > 0.375f && angle < 0.625f)
                bottom = false;
            if (angle > 0.625f && angle < 0.875f)
                left = false;
        }

        private PointF DateQuadrantToPosition(int quadrant, SizeF size)
        {
            switch (quadrant)
            {
                case 0:
                    return new PointF(X + (Diameter - size.Width) / 2, Y + Diameter * 0.3f - size.Height);
                case 1:
                    return new PointF(X + Diameter * 0.8f - size.Width / 2f, Y + Diameter / 2 - size.Height / 2f);
                case 2:
                    return new PointF(X + (Diameter - size.Width) / 2, Y + Diameter * 0.7f);
                case 3:
                    return new PointF(X + Diameter * 0.1f, Y + Diameter / 2 - size.Height / 2);
            }
            return new PointF(0, 0);
        }
            
        private float sin(double rel)
        {
            return (float)Math.Sin(rad(rel+Rotation));
        }
        private float cos(double rel)
        {
            return (float)Math.Cos(rad(rel+Rotation));
        }

        private double rad(double rel)
        {
            return (float)(Math.PI * 2 * rel - Math.PI / 2);
        }

        private PointF RotatedCenter(float distance, float angle)
        {
            var scale = Diameter * distance / 2;
            return new PointF(X + Diameter/2 - 0.5f + scale * cos(angle), Y + Diameter/2 - 0.5f + scale * sin(angle));
        }
    }
}
