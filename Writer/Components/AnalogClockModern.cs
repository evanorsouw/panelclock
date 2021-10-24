using Microsoft.Extensions.Logging;
using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class AnalogClockModern : Component
    {
        private DateTime _lastTime;
        private string _timezonename;
        private TimeZoneInfo _timezone = null;

        public AnalogClockModern(string id, ILogger logger=null) : base(id, logger)
        {
            AddProperty(Create("x", () => X, (obj) => X = obj));
            AddProperty(Create("y", () => Y, (obj) => Y = obj));
            AddProperty(Create("x2", () => X + Diameter));
            AddProperty(Create("y2", () => Y + Diameter));
            AddProperty(Create("x3", () => X + Diameter * ShowOrHideAnimationElapsed));
            AddProperty(Create("y3", () => Y + Diameter * ShowOrHideAnimationElapsed));
            AddProperty(Create("diameter", () => Diameter, (obj) => Diameter = obj));
            AddProperty(Create("width", () => Diameter));
            AddProperty(Create("height", () => Diameter));
            AddProperty(Create("showseconds", () => ShowSeconds, (obj) => ShowSeconds = obj));
            AddProperty(Create("smoothseconds", () => SmoothSeconds, (obj) => SmoothSeconds = obj));
            AddProperty(Create("timezone", () => TimeZone, (obj) => TimeZone = obj));
            AddProperty(Create("maincolor", () => MainColor, (obj) => MainColor = obj));
            AddProperty(Create("secondhandcolor", () => SecondHandColor, (obj) => SecondHandColor = obj));
        }

        #region IComponent

        public override IComponent Clone(string id)
        {
            var copy = new AnalogClockModern(id, Logger);

            copy.Diameter = Diameter;
            copy.X = X;
            copy.Y = Y;
            copy.TimeZone = TimeZone;
            copy.ShowSeconds = ShowSeconds;
            copy.SmoothSeconds = SmoothSeconds;
            copy.ExternalVisible = ExternalVisible;
            copy.InternalVisible = InternalVisible;
            copy.ShowOrHideTime = ShowOrHideTime;
            return this;
        }

        #endregion

        #region IDrawable interface

        public override void Draw(Graphics graphics)
        {
            if (!InternalVisible && !ShowingOrHiding)
                return;
            
            graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;

            var dia = Diameter * 0.07f;
            var brush = new SolidBrush(MainColor);
            graphics.FillEllipse(brush, new RectangleF(X + (Diameter - dia) / 2, Y + (Diameter - dia) / 2, dia, dia));

            var now = DateTime.Now;
            if (_timezone != null)
            {
                now = TimeZoneInfo.ConvertTime(now, _timezone);
            }
            var elapsed = (float)ShowOrHideAnimationElapsed;
            var seconds = (float)(now.Second / 60.0);
            var minutes = (float)((now.Minute + seconds) / 60.0);
            var hours = (float)((now.Hour % 12 + minutes) / 12.0);

            var pen = new Pen(MainColor, Diameter/40);
            for (int i = 0; i < 12; ++i)
            {
                var a = i / 12f;
                var p1 = RotatedCenter(0.9f, a, elapsed);
                var p2 = RotatedCenter(1f, a, elapsed);
                graphics.DrawLine(pen, p1, p2);
            }

            pen = new Pen(MainColor, Diameter/20);
            graphics.DrawLine(pen, RotatedCenter(0.2f, hours, elapsed), RotatedCenter(0.6f, hours, elapsed));
            pen = new Pen(MainColor, Diameter/30);
            graphics.DrawLine(pen, RotatedCenter(0.2f, minutes, elapsed), RotatedCenter(0.8f, minutes, elapsed));

            if (ShowSeconds)
            {
                float angle;
                if (SmoothSeconds)
                {
                    angle = (now.Second + now.Millisecond / 1000f) / 60f;
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
                pen = new Pen(SecondHandColor, Diameter/60);
                graphics.DrawLine(pen, RotatedCenter(0.1f, angle, elapsed), RotatedCenter(0.9f, angle, elapsed));
            }
        }

        #endregion

        #region Properties

        public Color MainColor { get; set; } = Color.White;
        public Color SecondHandColor { get; set; } = Color.Red;
        public float Diameter { get; set; } = 64;
        public float X { get; set; }
        public float Y { get; set; }
        public string TimeZone { get { return _timezonename; } set { SetTimeZone(value); } }
        public bool ShowSeconds { get; set; } = true;
        public bool SmoothSeconds { get; set; } = false;

        #endregion

        #region private code

        private float sin(double rel)
        {
            return (float)Math.Sin(rad(rel));
        }

        private float cos(double rel)
        {
            return (float)Math.Cos(rad(rel));
        }

        private double rad(double rel)
        {
            return (float)(Math.PI * 2 * rel - Math.PI / 2);
        }

        private PointF RotatedCenter(float distance, float angle, float elapsed)
        {
            var timePhase1 = 1 / 3f;
            var timePhase2 = 1f - timePhase1;

            var elapsedPhase1 = Math.Min(timePhase1, elapsed) / timePhase1;
            var elapsedPhase2 = Math.Min(timePhase2, Math.Max(0, elapsed - timePhase1)) / timePhase2;

            distance = elapsedPhase1 * distance;
            angle = angle * elapsedPhase2;

            var scale = Diameter * distance / 2;
            return new PointF(X + Diameter / 2 - 0.5f + scale * cos(angle), Y + Diameter / 2 - 0.5f + scale * sin(angle));
        }

        private void SetTimeZone(string value)
        {
            if (value == _timezonename)
                return;

            _timezonename = value;
            _timezone = null;

            foreach(var info in TimeZoneInfo.GetSystemTimeZones())
            {
                if (info.Id.ToLower().Contains(value.ToLower()))
                {
                    _timezone = info;
                    break;
                }
            }
            if (_timezone == null)
            {
                Logger.LogWarning($"timezone='{value}' is not recognized");
                foreach (var info in TimeZoneInfo.GetSystemTimeZones())
                {
                    Logger.LogInformation($"possible timezone='{info.Id}'");
                }
            }
        }

        #endregion
    }
}
