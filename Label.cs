using System;
using System.Drawing;
using System.Linq;
using System.Text.RegularExpressions;
using writer;

namespace WhiteMagic.PanelClock
{
    public class Label : IDrawable
    {
        private float _x;
        private float _y;
        private float _width;
        private float _height;
        private string _fontname;
        private string _text;
        private Color _textColor;
        private Color _backgroundColor;
        private Font _font;
        private bool _visible;
        private DateTime _animateStartTime;
        private float[] _paddings = { 0, 0, 0, 0 };
        private Alignment _hAlignment;
        private Alignment _vAlignment;
        private float _fontHeightInPixels;
        private float _actualTextWidth;
        private RectangleF _textBox;
        private RectangleF _backgroundBox;

        public Label() : this(0, 0)
        {
        }

        public Label(float x, float y)
        {
            _x = x;
            _y = y;
            _backgroundColor = Color.Transparent;
            _textColor = Color.White;
            Visible = true;
            SetFontName("Arial:12");
        }

        public float Width { get { return _backgroundBox.Width; } set { _width = value; SetDimensions(); } }
        public float Height { get { return _backgroundBox.Height; } set { _height = value; SetDimensions(); } }
        public float X { get { return _x; } set { _x = value; } }
        public float Y { get { return _y; } set { _y = value; } }
        public string FontName { get { return _fontname; } set { SetFontName(value); } }
        public string Text { get { return _text; } set { SetText(value); } }
        public Color TextColor { get { return _textColor; } set { _textColor = value; } }
        public Color BackgroundColor { get { return _backgroundColor; } set { _backgroundColor = value; } }
        public float HorizontalPadding { set { _paddings[0] = _paddings[2] = value; } }
        public float LeftPadding { get { return _paddings[0]; } set { _paddings[0] = value; } }
        public float RightPadding { get { return _paddings[2]; } set { _paddings[2] = value; } }
        public Alignment HorizontalAlignment { get { return _hAlignment; } set { _hAlignment = value; } }
        public Alignment VerticalAlignment { get { return _vAlignment; } set { _vAlignment = value; } }

        public bool Visible
        {
            get { return _visible; }
            set
            {
                if (_visible != value)
                {
                    _visible = value; _animateStartTime = DateTime.Now;
                }
            }
        }

        public float AnimationTime { get; set; } = 0.5f;

        public bool AnimationComplete
        {
            get { return DateTime.Now.Subtract(_animateStartTime).TotalSeconds > AnimationTime; }
        }

        public virtual void Draw(Graphics graphics)
        {
            if (!Visible && AnimationComplete)
                return;

            var elapsed = (float)Math.Min(1, DateTime.Now.Subtract(_animateStartTime).TotalSeconds / AnimationTime);
            var textcolor = _textColor;
            var backgroundcolor = _backgroundColor;
            if (!AnimationComplete)
            {
                if (!Visible)
                    elapsed = 1 - elapsed;
                textcolor = textcolor.Scale(elapsed);
                backgroundcolor = backgroundcolor.Scale(elapsed);
            }

            if (_backgroundColor.A > 0)
            {
                graphics.FillRectangle(new SolidBrush(backgroundcolor), _backgroundBox);
            }
            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            graphics.SetClip(_backgroundBox);
            graphics.DrawString(_text, _font, new SolidBrush(textcolor), _textBox);
        }

        private void SetFontName(string fontname)
        {
            var parts = fontname.Split(":");
            var fontsize = 12;
            FontStyle fontstyle = 0;
            foreach (var part in parts.Skip(1).Select(p=>p.ToLower()))
            {
                if (Regex.IsMatch(part, "^\\d+$"))
                {
                    fontsize = int.Parse(part);
                }
                else if (part == "bold")
                {
                    fontstyle |= FontStyle.Bold;
                }
                else if (part == "italic")
                {
                    fontstyle |= FontStyle.Bold;
                }
            }
#if !SIMULATION
            fontsize -= 1;
#endif
            _font = new Font(parts[0], fontsize, fontstyle, GraphicsUnit.Pixel);
            var graphics = Graphics.FromImage(new Bitmap(1, 1));
            _fontHeightInPixels = graphics.MeasureString("ZQdg", _font).Height;
#if SIMULATION
            _fontHeightInPixels -= 1;
#else
            _fontHeightInPixels += 2;
#endif
            _fontname = fontname;
            SetText(_text, true);
        }

        private void SetText(string txt, bool force=false)
        {
            if (_text != txt || force)
            {
                _text = txt;
                _actualTextWidth = Graphics.FromImage(new Bitmap(1, 1)).MeasureString(_text, _font).Width;
#if !SIMULATION
                _actualTextWidth += 2;
#endif
                SetDimensions();
            }
        }

        private void SetDimensions()
        {
            var tw = _actualTextWidth + _paddings[0] + _paddings[2];
            var th = _fontHeightInPixels + _paddings[1] + _paddings[3];
            var bw = _width != 0 ? _width : tw;
            var bh = _height != 0 ? _height : th;
            var x = _hAlignment switch
            {
                Alignment.Left => _x,
                Alignment.Center => _x - bw / 2,
                _ => _x - bw
            };
            var y = _vAlignment switch
            {
                Alignment.Top => _y,
                Alignment.Center => _y - bh / 2,
                _ => _y - bh
            };
            _backgroundBox = new RectangleF(x, y, bw, bh);

            x = _hAlignment switch
            {
                Alignment.Left => _x + _paddings[0],
                Alignment.Center => _x - _actualTextWidth / 2,
                _ => _x - _actualTextWidth - _paddings[2]
            };
            y = _vAlignment switch
            {
                Alignment.Top => y + (bh - th) / 2,
                Alignment.Center => _y - _fontHeightInPixels / 2,
                _ => _y - _fontHeightInPixels - _paddings[3]
            };
            _textBox = new RectangleF(x, y - 1, tw, th);
        }
    }
}
