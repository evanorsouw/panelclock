#if SIMULATION
using System.Collections.Generic;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

namespace WhiteMagic.PanelClock.Display
{
    public class Display : IDisplay
    {
        private int _width;
        private int _height;
        private int _scale;
        private Form _form;
        private Thread _formThread;
        private Bitmap _displayBitmap;
        private Dictionary<Color, Bitmap> _imageCache = new Dictionary<Color, Bitmap>();

        public Display(int dx, int dy, int scale=8)
        {
            _scale = scale;
            _width = dx;
            _height = dy;

            Application.EnableVisualStyles();
            _form = new Form();
            _form.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            _form.Size = new Size(dx*_scale+1, dy*_scale+1);

            _displayBitmap = new Bitmap(_form.Width, _form.Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            _formThread = new Thread(() => { Application.Run(_form); });
            _formThread.IsBackground = true;
            _formThread.Start();
        }

        public int Width => _width;

        public int Height => _height;

        public void Show(Bitmap bitmap)
        {
            var graphics = Graphics.FromImage(_displayBitmap);

            for (int y = 0; y < _height; ++y)
            {
                for (int x = 0; x < _width; ++x)
                {
                    var pixel = bitmap.GetPixel(x, y);
                    var image = GetImage(pixel);
                    graphics.DrawImage(image, 1 + x * _scale, 1 + y * _scale);
                }
            }
            _form.CreateGraphics().DrawImage(_displayBitmap, 0, 0);
        }

        private Bitmap GetImage(Color color)
        {
            Bitmap image;
            if (!_imageCache.TryGetValue(color, out image))
            {
                image = new Bitmap(_scale - 1, _scale - 1, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
                var graphics = Graphics.FromImage(image);
                var brush = new SolidBrush(Color.FromArgb(color.R & 0xFE, color.G & 0xFE, color.B & 0xFE));
                graphics.FillEllipse(brush, 0, 0, _scale - 1, _scale - 1);
                _imageCache[color] = image;
            }
            return image;
        }
    }
}
#endif