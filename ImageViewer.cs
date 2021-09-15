using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Reactive;
using System.Reactive.Subjects;
using System.Reactive.Concurrency;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using Microsoft.Extensions.Logging;
using System.Threading.Tasks;
//using DlibDotNet;
//using DlibDotNet.Extensions;
//using Dlib = DlibDotNet.Dlib;

namespace WhiteMagic.PanelClock
{
    public class ImageViewer : IDrawable, IDisposable
    {
        private ILogger _logger;
        private float _x;
        private float _y;
        private float _width;
        private float _height;
        private IImageSource _imageSource;                                                                                                                         
        private ConcurrentQueue<ImageAnimation> _imageQueue;
        private ImageAnimation _activeImage;
        private float _animationTime;
        private Subject<Unit> _prepareImageSubject;
        private CompositeDisposable _disposables;
//        private FrontalFaceDetector _faceDetector;
        private Random _random = new Random((int)DateTime.Now.Ticks);

        private class ImageAnimation
        {
            private IImageInfo _info;
            private IImageZoomer _zoomer;
            private float _animationTime;
            private DateTime _animationStart;

            public ImageAnimation(IImageInfo info, IImageZoomer zoomer, float animationTime)
            {
                _info = info;
                _zoomer = zoomer;
                _animationTime = animationTime;
                StartAnimation();
            }

            public Image Image => _info.Image;
            public string ImagePath => _info.Path;
            public string ImageSource => _info.Source;
            public RectangleF AnimationRect
            {
                get
                {
                    var fraction = Math.Min(1f, Elapsed / _animationTime);
                    return _zoomer.GetRectangle(fraction);
                }
            }

            public void StartAnimation()
            {
                _animationStart = DateTime.Now;
            }

            public bool AnimationComplete
            {
                get { return Elapsed > _animationTime; }
            }

            private float Elapsed => (float)(DateTime.Now.Subtract(_animationStart).TotalSeconds);
        }

        public ImageViewer(ILogger logger) : this(0, 0, 64, 64, logger)
        {
        }

        public ImageViewer(float x, float y, float width, float height, ILogger logger)
        {
            _logger = logger;
            _x = x;
            _y = y;
            _width = width;
            _height = height;
            _animationTime = 30.0f;

            _disposables = new CompositeDisposable();
            _imageQueue = new ConcurrentQueue<ImageAnimation>();

            //_faceDetector = Dlib.GetFrontalFaceDetector();
            //_disposables.Add(_faceDetector);

            var scheduler = new EventLoopScheduler();
            _disposables.Add(scheduler);

            _prepareImageSubject = new Subject<Unit>();
            _prepareImageSubject
                .ObserveOn(scheduler)
                .Subscribe(async _ => await TryToLoadImage());
            _disposables.Add(_prepareImageSubject);
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
            var image = GetActiveImage();
            if (image == null)
                return;

            var dst = new RectangleF(_x, _y, _width, _height);

            graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            graphics.DrawImage(image.Image, dst, image.AnimationRect, GraphicsUnit.Pixel);
        }

        private ImageAnimation GetActiveImage()
        {
            if (_activeImage != null && !_activeImage.AnimationComplete)
                return _activeImage;

            _activeImage = null;
            if (_imageQueue.TryDequeue(out var image))
            {
                image.StartAnimation();
                _logger.LogInformation($"now viewing image='{image.ImagePath}' from source='{image.ImageSource}'");
                _activeImage = image;
            }
            //_logger.LogInformation($"{_imageQueue.Count}");
            LoadImageIfBufferTooLow();
            return _activeImage;
        }

        private RectangleF[] FindFaceLocations(Bitmap bitmap)
        {
            //try
            //{
            //    using var image = bitmap.ToArray2D<RgbPixel>();
            //    var faces = _faceDetector.Operator(image);
            //    return faces.Select(r => new RectangleF(r.Left, r.Top, r.Width, r.Height)).ToArray();
            //}
            //catch (Exception)
            //{
            //}
            return new RectangleF[0];
        }

        private void LoadImageIfBufferTooLow()
        {
            if (_imageQueue.Count < 5)
            {
                _prepareImageSubject.OnNext(Unit.Default);
            }
        }

        private async Task TryToLoadImage()
        {
            await LoadImage();
            LoadImageIfBufferTooLow();
        }

        private async Task LoadImage()
        {
            var imgInfo = await _imageSource.GetRandomImage();
            if (imgInfo == null)
                return;

            if (imgInfo.Image.PixelFormat == System.Drawing.Imaging.PixelFormat.Format8bppIndexed)
                return;

            var oversample = 4;
            var image = ScaleDownImage(imgInfo.Image, oversample);
            //var faces = FindFaceLocations(image);
            //if (faces.Length == 0)
            //{
            //    _logger.LogDebug($"skipped image='{imgInfo.Path}' because it has no faces");
            //    return;
            //}

            //var face = faces[_random.Next(0, faces.Length)];

            //var graphics = Graphics.FromImage(image);
            //graphics.DrawRectangle(Pens.Red, face.X, face.Y, face.Width, face.Height);
            var face = new RectangleF(image.Width * 0.2f, image.Height * 0.2f, image.Width * 0.6f, image.Height * 0.6f);

            var zoomer = new AOIZoomer(face, Width * oversample, Height * oversample, image.Width, image.Height);
            var animation = new ImageAnimation(
                new ImageInfo { Image = image, Path = imgInfo.Path, Source = imgInfo.Source },
                zoomer, _animationTime);

            _imageQueue.Enqueue(animation);
            _logger.LogInformation($"{_imageQueue.Count}");
        }

        private Bitmap ScaleDownImage(Image image, float oversample)
        {
            var tgtAspect = _width / _height;
            var srcAspect = (float)image.Width / image.Height;

            float scale;
            if (srcAspect < tgtAspect)
            {
                scale = _width * oversample / image.Width;
            }
            else
            {
                scale = _height * oversample / image.Height;
            }
            var scaledImage = new Bitmap((int)(image.Width * scale+0.5f), (int)(image.Height * scale+0.5f), image.PixelFormat);
            var graphics = Graphics.FromImage(scaledImage);
            graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            graphics.DrawImage(image, new RectangleF(0, 0, scaledImage.Width, scaledImage.Height), new RectangleF(0, 0, image.Width, image.Height), GraphicsUnit.Pixel);

            return scaledImage;
        }

        public void Dispose()
        {
            _disposables.Dispose();
        }
    }
}
