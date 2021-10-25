using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Drawing;

namespace WhiteMagic.PanelClock.Components
{
    public class Icon : Component
    {
        private string _source;
        private Image _image;
        private Dictionary<string, Image> _cache = new ();

        public Icon(string id, ILogger logger) : base(id, logger)
        {
            AddProperty(Create("source", () => Source, (obj) => SetSource(obj)));

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
                //var bitmap = new Bitmap(20, 10, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                //Graphics.FromImage(bitmap).Clear(Color.FromArgb(128, 255, 255, 255));
                //_image = bitmap;
                graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
                graphics.DrawImage(_image, X1, Y1, Width, Height);
            }
        }

        #endregion

        protected override void GetFullSize(out float width, out float height)
        {
            width = _image?.Width ?? -1;
            height = _image?.Height ?? -1;
        }

        private void SetSource(string source)
        {
            if (_source == source)
                return;

            if (_cache.TryGetValue(source, out _image))
                return;

            _source = source;
            var path = $"Graphics/{source}";
            try
            {
                var image = Image.FromFile(path);
                Logger.LogInformation($"read icon='{path}' size='{image.Width}x{image.Height}', format='{image.PixelFormat}'");

                var bitmap = new Bitmap(image.Width, image.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                using (var g = Graphics.FromImage(bitmap))
                {
                    for (var y=0; y<image.Height; ++y)
                    {
                        for (var x = 0; x<image.Width; ++x)
                        {
                            var p = ((Bitmap)image).GetPixel(x, y);
                            g.FillRectangle(new SolidBrush(p), new Rectangle(x, y, 1, 1));
                        }
                    }
                }
                _image = bitmap;
                _cache[source] = _image;
                CoordinatesChanged();
                Logger.LogInformation($"icon location='{X1}x{Y1}' size='{Width}x{Height}'");
            }
            catch (Exception ex)
            {
                Logger.LogError($"Failed to load image from source='{path}': {ex.Message}");
                CreatePlaceholderImage();
            }
        }

        private void CreatePlaceholderImage()
        {
            _image = new Bitmap(8, 8, System.Drawing.Imaging.PixelFormat.Format32bppRgb);
            using var graphics = Graphics.FromImage(_image);
            graphics.Clear(Color.FromArgb(128, 255, 192, 203));
        }
    }
}
