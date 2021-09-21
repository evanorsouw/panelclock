using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public class AOIZoomer : IImageZoomer
    {
        private RectangleF _from;
        private RectangleF _to;

        public AOIZoomer(RectangleF aoi, float dstWidth, float dstHeight, float srcWidth, float srcHeight)
        {
            var srcAspect = srcWidth / srcHeight;
            var dstAspect = dstWidth / dstHeight;

            var sx = aoi.X;
            var sy = aoi.Y;
            var sw = aoi.Width;
            var sh = sw / dstAspect;

            var horizontalFit = srcAspect < dstAspect;
            var ex = horizontalFit ? 0f : (srcWidth - dstWidth) / 2;
            var ey = horizontalFit ? Math.Max(0f, sy - srcHeight * 0.1f) / 2 : 0f;
            var ew = horizontalFit ? srcWidth : dstWidth;
            var eh = horizontalFit ? dstHeight : srcHeight;

            _from = new RectangleF(sx, sy, sw, sh);
            _to = new RectangleF(ex, ey, ew, eh);
        }

        public RectangleF GetRectangle(float progress)
        {
            if (progress < 0)
                return _from;
            if (progress > 1)
                return _to;
            return new RectangleF(
                _from.X + (_to.X - _from.X) * progress,
                _from.Y + (_to.Y - _from.Y) * progress,
                _from.Width + (_to.Width - _from.Width) * progress,
                _from.Height + (_to.Height - _from.Height) * progress);
        }
    }
}

//var srcImage = image.Image;

//var tgtAspect = _width / _height;
//var srcAspect = (float)srcImage.Width / srcImage.Height;

//float scale;
//if (srcAspect < tgtAspect)
//{
//    scale = _width * oversample / srcImage.Width;
//}
//else
//{
//    scale = _height * oversample / srcImage.Height;
//}
//var tgtImage = new Bitmap((int)(srcImage.Width * scale), (int)(srcImage.Height * scale), srcImage.PixelFormat);
//var graphics = Graphics.FromImage(tgtImage);
//graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
//graphics.DrawImage(srcImage, new RectangleF(0, 0, tgtImage.Width, tgtImage.Height), new RectangleF(0, 0, srcImage.Width, srcImage.Height), GraphicsUnit.Pixel);


//var sx = face.X;
//var sy = face.Y;
//var sw = face.Width;
//var sh = sw / tgtAspect;

//var portrait = srcAspect < tgtAspect;
//var ex = portrait ? 0f : (tgtImage.Width - _width * oversample) / 2;
//var ey = portrait ? Math.Max(0f, sy - tgtImage.Height * 0.1f) / 2 : 0f;
//var ew = portrait ? tgtImage.Width : _width * oversample;
//var eh = portrait ? _height * oversample : tgtImage.Height;

//var animation = new ImageAnimation(new ImageInfo { Image = tgtImage, Path = image.Path, Source = image.Source }, _animationTime, _animationHoldAtEnd);
//animation.EndRect = new RectangleF(ex, ey, ew, eh);
//animation.StartRect = new RectangleF(sx, sy, sw, sh);

