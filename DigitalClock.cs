using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class DigitalClock : IDrawable
    {
        private float _width;
        private float _height;
        private float _rotation;
        private bool _includeSeconds = true;
        private Font _font;
        private float _boundingWidth;
        private float _boundingHeight;

        public DigitalClock() : this(12, 0, 0)
        {
        }

        public DigitalClock(float height, float x, float y)
        {
            Height = height;
            X = x;
            Y = y;
        }

        public float Width { get { return _width; } }

        public float Height { get { return _height; } set { SetHeight(value); } }

        public float Rotation { get { return _rotation; } set { SetRotation(value); } }

        public float X { get; set; }

        public float Y { get; set; }

        public float BWidth => _boundingWidth;

        public float BHeight => _boundingHeight;

        public bool IncludeSeconds { get { return _includeSeconds; } set { _includeSeconds = value; SetHeight(Height); } }

        public void Draw(Graphics graphics)
        {
            var max = 128;
            var bitmap = new Bitmap(max, max, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            var g = Graphics.FromImage(bitmap);
            g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            var time = DateTime.Now.ToString(IncludeSeconds ? "HH:mm:ss" : "HH:mm");
            g.TranslateTransform(max / 2 - 0.5f, max / 2-0.5f);
            g.RotateTransform(Rotation * 360);
            g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            g.DrawString(time, _font, Brushes.White, -_width / 2, -_height / 2);
            graphics.DrawImage(bitmap, X - (max - BWidth) / 2f, Y - (max - BHeight) / 2f);
        }

        private void SetHeight(float height)
        {
            _height = height;
            CalculateDerivedValues();
        }

        private void SetRotation(float rotation)
        {
            _rotation = rotation;
            CalculateDerivedValues();
        }

        private void CalculateDerivedValues()
        { 
            _font = new Font("Tahoma", Height, FontStyle.Bold, GraphicsUnit.Pixel);
            var graphics = Graphics.FromImage(new Bitmap(1, 1));
            _width = graphics.MeasureString(IncludeSeconds ? "88:88:88" : "88:88", _font, 999).Width;

            var diag = Math.Sqrt(_width * _width + _height * _height) / 2;
            var angle = Math.Atan2(_height, _width);
            var rot = _rotation * Math.PI * 2;
            var dx1 = diag * Math.Cos(rot + angle);
            var dx2 = diag * Math.Cos(rot - angle);
            var dy1 = diag * Math.Sin(rot + angle);
            var dy2 = diag * Math.Sin(rot - angle);
            _boundingWidth = (float)(Math.Max(Math.Abs(dx1), Math.Abs(dx2))*2);
            _boundingHeight = (float)(Math.Max(Math.Abs(dy1), Math.Abs(dy2))*2);
        }
    }
}
