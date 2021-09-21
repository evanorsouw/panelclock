//using System;
//using System.Collections.Generic;
//using System.Drawing;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
//using WhiteMagic.PanelClock;

//namespace writer
//{
//    public class Ticker : IDrawable
//    {
//        private float _width;
//        private float _height;
//        private float _x = 0;
//        private float _y = 0;
//        private string _text;
//        private string _fontname = "Arial";
//        private float _speed = 8f;
//        private int _loops = int.MaxValue;
//        private Color _textColor = Color.White;
//        private Color _backgroundColor = Color.Transparent;
//        private Font _font;
//        private Bitmap _bitmap;
//        private int _loopCountdown;
//        private DateTime _loopStart;
//        private Bitmap _nextBitmap;

//        public Ticker(int width, int height)
//        {
//            _x = 0;
//            _y = 0;
//            _width = width;
//            _height = height;
//            SetDerivedValues();
//        }

//        public float X { get { return _x; } set { _x = value; } }
//        public float Y { get { return _y; } set { _y = value; } }
//        public float Width { get { return _width; } set { _width = value; } }
//        public float Height { get { return _height; } set { _height = value; SetDerivedValues(); } }
//        public string Text { get { return _text; } set { SetText(value); } }
//        public string FontName { get { return _fontname; } set { _fontname = value; SetDerivedValues(); } }
//        public float Speed { get { return _speed; } set { _speed = value; } }
//        public int Loops { get { return _loops; } set { _loops = value; } }
//        public Color TextColor { get { return _textColor; } set { _textColor = value; } }
//        public Color BackgroundColor { get { return _backgroundColor; } set { _backgroundColor = value; } }

//        public void Draw(Graphics g)
//        {
//            var now = DateTime.Now;
//            if (_bitmap != null)
//            {
//                var elapsed = now.Subtract(_loopStart).TotalSeconds;
//                var offset = (float)(elapsed * _speed);
//                if (offset < _width)
//                {
//                    g.DrawImage(_bitmap, new RectangleF(_x + _width - offset, _y, _width, _height), new RectangleF(0, 0, _width, _bitmap.Height), GraphicsUnit.Pixel);
//                }
//                else
//                {
//                    g.DrawImage(_bitmap, new RectangleF(_x, _y, _width, _height), new RectangleF(offset - _width, 0, _width, _bitmap.Height), GraphicsUnit.Pixel);
//                }
//                if (offset < _bitmap.Width + _width)
//                    return;

//                _loopStart = now;
//                if (--_loopCountdown > 0)
//                    return;
//            }
//            if (_nextBitmap != null)
//            {
//                _bitmap = _nextBitmap;
//                _nextBitmap = null;
//                _loopCountdown = _loops;
//                _loopStart = now;
//            }
//        }

//        private void SetDerivedValues()
//        {
//            _font = new Font(_fontname, _height*0.8f, FontStyle.Italic, GraphicsUnit.Pixel);
//        }

//        private void SetText(string text)
//        {
//            using (var g= Graphics.FromImage(new Bitmap(8, 8)))
//            {
//                var size = g.MeasureString(text, _font);
//                _nextBitmap = new Bitmap((int)(size.Width + 0.5), (int)(_height + 0.5), System.Drawing.Imaging.PixelFormat.Format32bppArgb);
//            }
//            using var graphics = Graphics.FromImage(_nextBitmap);
//            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
//            graphics.Clear(_backgroundColor);
//            graphics.DrawString(text, _font, new SolidBrush(_textColor), 0, 0);
//            _text = text;
//        }
//    }
//}