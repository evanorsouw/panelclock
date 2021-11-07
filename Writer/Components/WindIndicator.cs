using Microsoft.Extensions.Logging;
using System;
using System.Drawing;
using System.Drawing.Drawing2D;


namespace WhiteMagic.PanelClock.Components
{
    public class WindIndicator : Component
    {
        public WindIndicator(string id, ILogger logger) : base(id, logger)
        {
            AddProperty(Create("angle", () => Angle, (obj) => Angle= obj));
        }

        #region Properties

        public float Angle { get; set; }

        #endregion

        #region IDrawable

        public override void Draw(Graphics graphics)
        {
            if (InternalVisible)
            {
                graphics.SmoothingMode = SmoothingMode.AntiAlias;

                var max = Math.Min(Width,Height);
                if (max == 0)
                {
                    max = Math.Max(Width, Height);
                }
                var pw = Math.Max(1f, max/ 21f);
                var d = max - pw - 1f;
                var x = X + pw / 2f;
                var y = Y + pw / 2f;
                var angle = Angle * Math.PI * 2f + Phase(9000,true) * Phase(800,true) * 0.1;
                var cx = x + d / 2f;
                var cy = y + d / 2f;
                var x1 = cx + d * 0.5f * (float)Math.Cos(angle);
                var y1 = cy + d * 0.5f * (float)Math.Sin(angle);
                var x3 = cx - d * 0.25f * (float)Math.Cos(angle);
                var y3 = cy - d * 0.25f * (float)Math.Sin(angle);
                var x2 = cx - d * 0.5f * (float)Math.Cos(angle - 0.6);
                var y2 = cy - d * 0.5f * (float)Math.Sin(angle - 0.6);
                var x4 = cx - d * 0.5f * (float)Math.Cos(angle + 0.6);
                var y4 = cy - d * 0.5f * (float)Math.Sin(angle + 0.6);
                var path = new GraphicsPath();
                path.AddLine(x1, y1, x3, y3);
                path.AddLine(x3, y3, x4, y4);
                path.AddLine(x4, y4, x1, y1);
                graphics.FillPath(Brushes.DarkRed, path);
                path = new GraphicsPath();
                path.AddLine(x1, y1, x2, y2);
                path.AddLine(x2, y2, x3, y3);
                path.AddLine(x3, y3, x1, y1);
                graphics.FillPath(Brushes.Red, path);
            };
        }

        #endregion
    }
}
