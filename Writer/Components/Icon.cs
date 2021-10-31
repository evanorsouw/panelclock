using Microsoft.Extensions.Logging;
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Reflection;

namespace WhiteMagic.PanelClock.Components
{
    public class Icon : Component
    {
        private string _source;
        private float _phase;
        private Func<Graphics, float, float> _iconDrawer;

        public Icon(string id, ILogger logger) : base(id, logger)
        {
            AddProperty(Create("source", () => Source, (obj) => Source = obj));

            CreatePlaceholderImage();
        }

        #region Properties

        public string Source { get { return _source; } set { SetSource(value); } }

        #endregion

        #region IDrawable

        public override void Draw(Graphics graphics)
        {
            if (InternalVisible)
            {
                graphics.SmoothingMode = SmoothingMode.AntiAlias;
                graphics.TranslateTransform(ContentTopLeft.X, ContentTopLeft.Y);
                _iconDrawer(graphics, Width);
            }
        }

        #endregion

        private void SetSource(string source)
        {
            if (source == _source)
                return;

            _source = source;

            var type = typeof(IconDrawings);
            foreach(var p in type.GetProperties(BindingFlags.Public | BindingFlags.Static))
            {
               if (p.Name.ToLower() != source)
                    continue;

                _iconDrawer = (Func<Graphics, float, float>)p.GetValue(type, null);
                    return;
            }
            _iconDrawer = CreatePlaceholderImage();
        }

        private Func<Graphics, float, float> CreatePlaceholderImage()
        {
            return (Graphics g, float w) =>
                {
                    var pen = new Pen(Color.Pink, 1);
                    var h = w;
                    var phase = DateTime.Now.Millisecond / 1000f;
                    g.DrawLine(pen, phase * w, 0, (1 - phase) * w, h);
                    g.DrawLine(pen, w, phase * h, 0, (1 - phase) * h);
                    return h;
                };
        }
    }
}
