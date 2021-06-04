using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WhiteMagic.PanelClock
{
    public class ImageViewer : IDrawable
    {
        private float _x;
        private float _y;
        private float _width;
        private float _height;
        private IImageSource _imageSource;
        private IImageInfo _activeImage;
        private float _animationTime;
        private RectangleF _srcStart;
        private RectangleF _srcEnd;
        private float _animationHoldAtEnd;
        private DateTime _animationStart;

        public ImageViewer() : this(0, 0, 64, 64)
        {
        }

        public ImageViewer(float x, float y, float width, float height)
        {
            _x = x;
            _y = y;
            _width = width;
            _height = height;
            _animationTime = 20.0f;
            _animationHoldAtEnd = 2.0f;
        }

        public float X { get { return _x; } set { _x = value; } }
        public float Y { get { return _y; } set { _y = value; } }
        public float Width { get { return _width; } set { _width = value; } }
        public float Height { get { return _height; } set { _height = value; } }
        public IImageSource ImageSource { get { return _imageSource; } set { _imageSource = value; } }
        public float AnimationTime { get { return _animationTime; } set { _animationTime = value; } }
        public float AnimationHoldAtEnd { get { return _animationTime; } set { _animationTime = value; } }


        public void Draw(Graphics graphics)
        {
            UpdateImage();
            if (_activeImage == null)
                return;

            var fraction = (float)(DateTime.Now.Subtract(_animationStart).TotalSeconds / _animationTime);
            fraction = Math.Min(fraction, 1f);
            var img = _activeImage.Image;
            var src = new RectangleF(
                _srcStart.X + (_srcEnd.X - _srcStart.X) * fraction,
                _srcStart.Y + (_srcEnd.Y - _srcStart.Y) * fraction,
                _srcStart.Width + (_srcEnd.Width - _srcStart.Width) * fraction,
                _srcStart.Height + (_srcEnd.Height - _srcStart.Height) * fraction);
            var dst = new RectangleF(_x, _y, _width, _height);
            graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            graphics.DrawImage(img, dst, src, GraphicsUnit.Pixel);

        }

        private bool AnimationComplete()
        {
            return DateTime.Now.Subtract(_animationStart).TotalSeconds >= _animationTime;
        }

        private void UpdateImage()
        {
            if (_activeImage != null && !AnimationComplete())
                return;

            _activeImage = _imageSource.GetRandomImage();
            if (_activeImage != null)
            {
                _animationStart = DateTime.Now;
                var img = _activeImage.Image;
                _srcStart = new RectangleF(0, 0, img.Width, img.Height);
                _srcEnd = new RectangleF(img.Width*.1f, img.Height*0.1f, img.Width*0.8f, img.Height*0.8f);
            }
        }
    }
}
