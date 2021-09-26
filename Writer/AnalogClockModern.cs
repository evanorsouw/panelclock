using Microsoft.Extensions.Logging;
using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class AnalogClockModern : Component
    {
        private ILogger _logger;
        private DateTime _lastTime;
        private bool _visible;
        private DateTime _animateStartTime;
        private string _timezonename;
        private TimeZoneInfo _timezone = null;

        public AnalogClockModern(string id, ILogger logger=null) : base(id)
        {
            Id = id;
            _logger = logger;
            Visible = true;

            AddProperty(Create("x", typeof(float), () => X, (obj) => X = (float)obj));
            AddProperty(Create("y", typeof(float), () => Y, (obj) => Y = (float)obj));
            AddProperty(Create("diameter", typeof(float), () => Diameter, (obj) => Diameter = (float)obj));
            AddProperty(Create("showseconds", typeof(bool), () => ShowSeconds, (obj) => ShowSeconds = (bool)obj));
            AddProperty(Create("smoothseconds", typeof(bool), () => SmoothSeconds, (obj) => SmoothSeconds = (bool)obj));
        }

        #region IComponent

        public string Id { get; private set; }

        public override IComponent Clone(string id)
        {
            var copy = new AnalogClockModern(id, _logger);

            copy.Diameter = Diameter;
            copy.X = X;
            copy.Y = Y;
            copy.TimeZone = TimeZone;
            copy.ShowSeconds = ShowSeconds;
            copy.SmoothSeconds = SmoothSeconds;
            copy.Visible = Visible;
            copy.AnimationTime = AnimationTime;
            return this;
        }

        #endregion

        #region IDrawable interface

        public override void Draw(Graphics graphics)
        {
            var now = DateTime.Now;
            var animationComplete = now.Subtract(_animateStartTime).TotalSeconds > AnimationTime;
            if (!Visible && animationComplete)
                return;

            graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;

            var dia = Diameter * 0.07f;
            graphics.FillEllipse(Brushes.White, new RectangleF(X + (Diameter - dia) / 2, Y + (Diameter - dia) / 2, dia, dia));

            if (_timezone != null)
            {
                now = TimeZoneInfo.ConvertTime(now, _timezone);
            }
            var seconds = (float)(now.Second / 60.0);
            var minutes = (float)((now.Minute + seconds) / 60.0);
            var hours = (float)((now.Hour % 12 + minutes) / 12.0);

            for (int i = 0; i < 12; ++i)
            {
                var a = i / 12f;
                var p1 = RotatedCenter(0.9f, a);
                var p2 = RotatedCenter(1f, a);
                graphics.DrawLine(Pens.White, p1, p2);
            }

            var pen = new Pen(Color.White, 3);
            graphics.DrawLine(pen, RotatedCenter(0.2f, hours), RotatedCenter(0.6f, hours));
            pen = new Pen(Color.White, 1.6f);
            graphics.DrawLine(pen, RotatedCenter(0.2f, minutes), RotatedCenter(0.8f, minutes));

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
                pen = new Pen(Color.Red, 0.6f);
                graphics.DrawLine(pen, RotatedCenter(0.1f, angle), RotatedCenter(0.9f, angle));
            }
        }

        #endregion

        #region Properties

        public float Diameter { get; set; } = 64;
        public float X { get; set; }
        public float Y { get; set; }
        public string TimeZone { get { return _timezonename; } set { SetTimeZone(value); } }
        public bool ShowSeconds { get; set; } = true;
        public bool SmoothSeconds { get; set; } = false;
        public bool Visible
        {
            get { return _visible; }
            set
            {
                if (_visible != value)
                {
                    _visible = value; _animateStartTime = DateTime.Now;
                }
            }
        }
        public float AnimationTime { get; set; } = 1f;

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

        private PointF RotatedCenter(float distance, float angle)
        {
            var elapsed = (float)DateTime.Now.Subtract(_animateStartTime).TotalSeconds;
            if (_visible)
            {
                var timePhase1 = AnimationTime / 3f;
                var timePhase2 = AnimationTime - timePhase1;

                var elapsedPhase1 = Math.Min(timePhase1, elapsed) / timePhase1;
                var elapsedPhase2 = Math.Min(timePhase2, Math.Max(0, elapsed - timePhase1)) / timePhase2;

                distance = elapsedPhase1 * distance;
                angle = angle * elapsedPhase2;
            }
            else
            {
                var timePhase2 = AnimationTime / 3f;
                var timePhase1 = AnimationTime - timePhase2;

                var elapsedPhase1 = 1f - Math.Min(timePhase1, elapsed) / timePhase1;
                var elapsedPhase2 = 1f - Math.Min(timePhase2, Math.Max(0, elapsed - timePhase1)) / timePhase2;

                angle = angle * elapsedPhase1;
                distance = elapsedPhase2 * distance;
            }

            var scale = Diameter * distance / 2;
            return new PointF(X + Diameter / 2 - 0.5f + scale * cos(angle), Y + Diameter / 2 - 0.5f + scale * sin(angle));
        }

        private void SetTimeZone(string value)
        {
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
                _logger.LogWarning($"timezone='{value}' is not recognized");
                foreach (var info in TimeZoneInfo.GetSystemTimeZones())
                {
                    _logger.LogInformation($"possible timezone='{info.Id}'");
                }
            }
        }

        #endregion
    }
}
