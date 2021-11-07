using System;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace WhiteMagic.PanelClock
{
    public class IconDrawings
    {
        private static Random _random = new Random();

        [Description("zonnig")]
        public static Func<Graphics, float, float> Sunny =>
            (Graphics g, float w) =>
            {
                var h = w * 0.6f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    g.TranslateTransform(w * (-0.05f + 0.1f * Phase(57000, true)), 0);
                    DrawSun(g, w, h, true);
                }
                return h;
            };

        [Description("bliksem")]
        public static Func<Graphics, float, float> Lightning => CloudWithLightning();

        [Description("regen")]
        public static Func<Graphics, float, float> Rain => CloudWithRain(false);

        [Description("buien")]
        public static Func<Graphics, float, float> Showers => CloudWithRain(true);

        [Description("hagel")]
        public static Func<Graphics, float, float> Hail =>

            (Graphics g, float w) =>
            {
                return w * 0.75f;
            };

        [Description("mist")]
        public static Func<Graphics, float, float> Fog =>

            (Graphics g, float w) =>
            {
                var h = w * 0.75f;

                var tmp = g.Transform;
                g.ScaleTransform(1f, 0.8f);
                DrawCloud(g, w, Color.Transparent, Color.FromArgb(32, 32, 32));
                g.Transform = tmp;
                DrawFog(g, w, h);
                return h;
            };

        [Description("sneeuw")]
        public static Func<Graphics, float, float> Snow => CloudWithSnow();

        [Description("bewolkt")]
        public static Func<Graphics, float, float> Clouded => DoubleClouds(Color.White, Color.Black);

        [Description("lichtbewolkt")]
        public static Func<Graphics, float, float> Cloudy => CloudWithSun();

        [Description("halfbewolkt")]
        public static Func<Graphics, float, float> PartlyCloudy => CloudWithSun();

        [Description("halfbewolkt_regen")]
        public static Func<Graphics, float, float> CloudyRain => CloudWithSunAndRain();

        [Description("zwaarbewolkt")]
        public static Func<Graphics, float, float> HeavyClouds => DoubleClouds(Color.White, Color.DarkGray);

        [Description("nachtmist")]
        public static Func<Graphics, float, float> NightFog => MoonWithFog();

        [Description("helderenacht")]
        public static Func<Graphics, float, float> ClearNight =>

            (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    var tmp = g.Transform;
                    var d = Math.Min(w, h);
                    g.ScaleTransform(0.6f, 0.6f);
                    g.TranslateTransform(w * (0.1f + 0.2f * Phase(29000, true)), h * 0.2f);

                    var brush = new SolidBrush(Color.FromArgb(64, 64, 64));
                    g.FillEllipse(brush, 0, 0, d, d);
                    var path = new GraphicsPath();
                    path.AddArc(0, 0, d, d, 280, 160);
                    path.AddArc(-d * 1.3f, -d / 2, d * 2, d * 2, 20, -40);
                    g.FillPath(Brushes.White, path);
                }
                return h;
            };

        [Description("wolkennacht")]
        public static Func<Graphics, float, float> CloudedNight => CloudWithMoon();

        private static Func<Graphics, float, float> DoubleClouds(Color lineColor, Color fillColor)
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    var tmp = g.Transform;
                    g.ScaleTransform(1f, 0.7f);
                    g.TranslateTransform(w * (0.1f + 0.2f * Phase(27000, true)), h * 0.1f);
                    DrawCloud(g, w * 0.7f, lineColor, fillColor);
                    g.Transform = tmp;
                    g.TranslateTransform(w * 0.2f * Phase(19000, true), h * 0.2f);
                    DrawCloud(g, w * 0.8f, lineColor, fillColor);

                }
                return h;
            };
        }

        private static Func<Graphics, float, float> CloudWithRain(bool heavy)
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    g.TranslateTransform(w * (0.2f * Phase(27000, true)), 0f, MatrixOrder.Append);
                    var tmp = g.Transform;
                    g.ScaleTransform(1f, 0.8f);
                    var fill = heavy ? Color.FromArgb(32, 32, 32) : Color.Black;
                    DrawCloud(g, w * 0.8f, Color.White, fill);
                    g.Transform = tmp;
                    DrawRain(g, w, h, heavy);
                }
                return h;
            };
        }

        private static Func<Graphics, float, float> CloudWithSun()
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    var tmp = g.Transform;
                    g.TranslateTransform(w * 0.1f * Phase(29000, true), 0, MatrixOrder.Append);
                    g.ScaleTransform(0.7f, 0.7f);
                    DrawSun(g, w, h, true);
                    g.Transform = tmp;
                    g.TranslateTransform(w * 0.1f * Phase(20000, true), h * 0.2f, MatrixOrder.Append);
                    g.ScaleTransform(0.9f, 0.8f);
                    DrawCloud(g, w, Color.White, Color.Black);
                }
                return h;
            };
        }

        private static Func<Graphics, float, float> CloudWithMoon()
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    var tmp = g.Transform;
                    g.TranslateTransform(w * 0.1f * Phase(20000, true), h * 0.1f, MatrixOrder.Append);
                    g.ScaleTransform(0.9f, 0.9f);
                    DrawCloud(g, w, Color.White, Color.Black);
                    g.Transform = tmp;
                    g.TranslateTransform(w * (0.1f + 0.3f * Phase(57000, true)), 0, MatrixOrder.Append);
                    g.ScaleTransform(0.7f, 0.7f);
                    DrawMoon(g, w, h);
                }
                return h;
            };
        }

        private static Func<Graphics, float, float> MoonWithFog()
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    var tmp = g.Transform;
                    g.ScaleTransform(0.8f, 0.8f);
                    g.TranslateTransform(w * (0.2f + 0.3f * Phase(37000, true)), w * 0.05f);
                    DrawMoon(g, w, h);
                    g.Transform = tmp;
                    DrawFog(g, w, h);
                }
                return h;
            };
        }

        private static Func<Graphics, float, float> CloudWithSunAndRain()
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.OliveDrab, 0, 0, w, h);
                    var tmp = g.Transform;
                    g.ScaleTransform(0.6f, 0.6f);
                    g.TranslateTransform(w * 0.2f * Phase(29000, true), 0, MatrixOrder.Append);
                    DrawSun(g, w, h, true);
                    g.Transform = tmp;
                    g.TranslateTransform(w * (0.1f + 0.1f * Phase(27000,true)), w*0.1f);
                    g.ScaleTransform(1f, 0.8f);
                    DrawCloud(g, w * 0.8f, Color.White, Color.Black);
                    DrawRain(g, w, h, false);
                }
                return h;
            };
        }

        private static Func<Graphics,float,float> CloudWithSnow()
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    DrawSnow(g, w, h);
                }
                return h;
            };
        }

        private static Func<Graphics, float, float> CloudWithLightning()
        {
            return (Graphics g, float w) =>
            {
                var h = w * 0.75f;
                if (w > 0)
                {
                    //g.FillRectangle(Brushes.DarkOliveGreen, 0, 0, w, h);
                    var showLightning = _random.Next(100) < 3;
                    g.TranslateTransform(w * 0.15f, 0f);
                    g.ScaleTransform(1f, 0.8f);
                    DrawCloud(g, w * 0.8f, Color.White, Color.FromArgb(32, 32, 32));
                    if (showLightning)
                    {
                        var x = w * (0.1f + 0.2f * (float)_random.NextDouble());
                        var r = -10f + (float)_random.NextDouble() * 20f;
                        g.RotateTransform(r);
                        g.TranslateTransform(x, w * 0.18f);
                        DrawLightning(g, w, h);
                    }
                }
                return h;
            };
        }

        private static void DrawCloud(Graphics g, float w, Color lineColor, Color fillColor)
        {
            var pw = Math.Max(1f, w / 15f);
            var pen = new Pen(lineColor, pw);
            var brush = new SolidBrush(fillColor);
            var h = w * 0.75f;
            var d1 = 0.50f * w;
            var d2 = 0.45f * w;
            var d3 = 0.35f * w;
            var x1 = w - d3 - pw;
            var y1 = h - pw;
            var path = new GraphicsPath();
            path.AddArc(0.4f * w, pw, d2, d2, 190f, 170f);
            path.AddArc(x1, y1 - d3, d3, d3, 290f, 160f);
            path.AddLine(x1, y1, d1 / 2, y1);
            path.AddArc(pw, y1 - d1, d1, d1, 90f, 230f);
            g.FillPath(brush, path);
            g.DrawPath(pen, path);
        }

        private static void DrawSnow(Graphics g, float w, float h)
        {
            var n1 = 19;
            var n2 = 10;
            var phase = Phase(10000,false) * n1;
            var d = w * 0.05f;
            for (int i = 0; i < n1; ++i)
            {
                var ix = (i*i)%n1;
                var low = i;
                var high = (i + n2)%n1;
                if (low < high && (phase < low || phase > high))
                    continue;
                if (high < low && (phase < low && phase > high))
                    continue;
                var p = phase - low;
                if (p < 0)
                    p += n1;
                p /= n2;
                var x = w * (0.1f + 0.8f * ix / n1 + 0.1f * (float)Math.Cos(p * Math.PI * (2+(i&1))));
                var y = (h-d) * p;

                if (p > 0f && p < 1f)
                    g.FillEllipse(Brushes.White, x, y, d, d * (0.3f + 0.7f*Phase(1000+i*400, true)));
            }
        }

        private static void DrawRain(Graphics g, float w, float h, bool heavy)
        {
            var pw = w / 20f;
            var pen = new Pen(Color.White, pw);
            var phase = Phase(heavy ? 800 : 2000, false);
            for (var i = 0; i < (heavy ? 4 : 3); ++i)
            {
                var x = (i + 1) * (heavy ? 0.17f : 0.22f) * w;
                var y = 0.4f * h;
                var dx = -w * (heavy ? 0.15f : 0.05f);
                var dy = h - y - pw;
                phase = (phase + i * 0.2f) % 1f;
                float x1, y1, x2, y2, x3 = 0, y3 = 0, x4 = 0, y4 = 0;
                float length = 0.3f;
                if (phase < length)
                {
                    x1 = x;
                    y1 = y;
                    x2 = x + dx * phase;
                    y2 = y + dy * phase;
                    x3 = x + dx * (phase + 0.5f);
                    y3 = y + dy * (phase + 0.5f);
                    x4 = x + dx * (phase + 0.5f + length);
                    y4 = y + dy * (phase + 0.5f + length);
                }
                else if (phase < (1f - length))
                {
                    x1 = x + dx * phase;
                    y1 = y + dy * phase;
                    x2 = x + dx * (phase + length);
                    y2 = y + dy * (phase + length);
                }
                else
                {
                    x1 = x + dx * phase;
                    y1 = y + dy * phase;
                    x2 = x + dx;
                    y2 = y + dy;
                    x3 = x + dx * (phase - 0.5f);
                    y3 = y + dy * (phase - 0.5f);
                    x4 = x + dx * (phase - 0.5f - length);
                    y4 = y + dy * (phase - 0.5f - length);
                }
                g.DrawLine(pen, x1, y1, x2, y2);
                g.DrawLine(pen, x3, y3, x4, y4);
            }
        }

        private static void DrawLightning(Graphics g, float w, float h)
        {
            var points = new PointF[] {
                new PointF(w*0.37f,h*0.12f),
                new PointF(w*0.25f,h*0.49f),
                new PointF(w*0.36f,h*0.50f),
                new PointF(w*0.12f,h*1f),
                new PointF(w*0.24f,h*0.59f),
                new PointF(w*0.15f,h*0.60f)
            };
            g.FillPolygon(Brushes.White, points);
        }

        private static void DrawMoon(Graphics g, float w, float h)
        {
            var path = new GraphicsPath();
            var pw = w / 15f;
            var d = Math.Min(w, h) - pw;
            path.AddArc(0, pw / 2, d, d, 55, 210);
            path.AddArc(d * 0.29f, pw / 2 - d * 0.12f, d, d, 210, -125);

            g.FillPath(Brushes.Black, path);
            var pen = new Pen(Color.White, pw);
            pen.EndCap = LineCap.Round;
            pen.StartCap = LineCap.Round;
            g.DrawPath(pen, path);
        }

        private static void DrawSun(Graphics g, float w, float h, bool animated)
        {
            var phase = Phase(20000, false);
            var pw = Math.Max(1f, w / 22f);

            var pen = new Pen(Color.LightGoldenrodYellow, pw);
            pen.EndCap = LineCap.Round;
            pen.StartCap = LineCap.Round;

            var max = Math.Min(w, h) - pw;
            var dia = 0.5f * max;
            var x1 = w / 2;
            var y1 = h / 2;
            for (var i = 0; i < 8; ++i)
            {
                var angle = Math.PI / 4 * i;
                if (animated)
                {
                    angle += phase * Math.PI;
                }
                var dx = max * 0.5f * (float)Math.Cos(angle);
                var dy = max * 0.5f * (float)Math.Sin(angle);
                g.DrawLine(pen, x1 + dx * 0.75f, y1 + dy * 0.75f, x1 + dx, y1 + dy);
            }
            g.DrawEllipse(pen, x1 - dia / 2, y1 - dia / 2, dia, dia);
        }

        private static void DrawIceCrystal(Graphics g, float w, float h)
        {
            var pw = Math.Max(1f, w / 15f);

            var phase = Phase(10000, false);
            var max = (int)(phase * 200);
            var n = 0;
            var fade = 1f - Math.Max(0f, Math.Min(1f, (phase-0.9f) / 0.1f));

            var pen = new Pen(Color.FromArgb(186,244,244).ScaleAlpha(fade), pw);
            pen.EndCap = LineCap.Round;
            pen.StartCap = LineCap.Round;
            var d = (Math.Min(h, w) - pw) / 2f;

            var xc = w/2;
            var yc = h/2;
            var ordered = new[] { 0, 3, 4, 2, 1, 5 };
            for (var i = 0; i < 6; ++i)
            {
                var spike = ordered[i];
                var angle0 = Math.PI / 3 * (spike-1);
                var angle1 = Math.PI / 3 * spike;
                var angle2 = Math.PI / 9;

                var s1 = (float)Math.Sin(angle1 - angle2);
                var c1 = (float)Math.Cos(angle1 - angle2);
                var s2 = (float)Math.Sin(angle1);
                var c2 = (float)Math.Cos(angle1);
                var s3 = (float)Math.Sin(angle1 + angle2);
                var c3 = (float)Math.Cos(angle1 + angle2);
                var s4 = (float)Math.Sin(angle0);
                var c4 = (float)Math.Cos(angle0);
                
                var x1 = xc + d * c2 * 0.3f;
                var y1 = yc + d * s2 * 0.3f;
                var x2 = xc + d * c2 * 0.6f;
                var y2 = yc + d * s2 * 0.6f;
                var x3 = xc + d * c2;
                var y3 = yc + d * s2;
                var x4 = xc + d * c1;
                var y4 = yc + d * s1;
                var x5 = xc + d * c3;
                var y5 = yc + d * s3;
                var x6 = xc + d * c4*0.3f;
                var y6 = yc + d * s4*0.3f;
                if (++n<max)
                    g.DrawLine(pen, x1, y1, x3, y3);
                if (++n < max)
                    g.DrawLine(pen, x2, y2, x4, y4);
                if (++n < max)
                    g.DrawLine(pen, x2, y2, x5, y5);
                if (max > 0)
                    g.DrawLine(pen, x1, y1, x6, y6);
            }
        }

        private static void DrawFog(Graphics g, float w, float h)
        {
            var pen = new Pen(Color.White, Math.Max(1f, w / 15f));
            var phase1 = Phase(37000, true);
            var phase2 = Phase(19000, true);
            var y1 = h * 0.25f;
            var y2 = h * 0.39f;
            var y3 = h * 0.53f;
            var y4 = h * 0.67f;
            g.DrawLine(pen, w * (0.2f + 0.2f * phase1), y1, w * (0.7f + 0.05f * phase1), y1);

            g.DrawLine(pen, w * (0.2f + 0.05f * phase2), y2, w * (0.55f + 0.1f * phase1), y2);
            g.DrawLine(pen, w * (0.65f + 0.05f * phase1), y2, w * (0.9f - 0.1f * phase2), y2);

            g.DrawLine(pen, w * (0.25f - 0.05f * phase1), y3, w * (0.4f + 0.15f * phase2), y3);
            g.DrawLine(pen, w * (0.45f + 0.25f * phase2), y3, w * (0.95f - 0.2f * phase1), y3);

            g.DrawLine(pen, w * (0.4f - 0.3f * phase1), y4, w * (0.75f + 0.05f * phase2), y4);
        }

        private static float Phase(int ms, bool wave)
        {
            var phase = DateTime.Now.Ticks / 10000 % ms / (float)ms;
            if (wave)
            {
                phase = 0.5f + (float)Math.Cos(phase * Math.PI * 2) / 2f;
            }
            return phase;
        }
    }
}