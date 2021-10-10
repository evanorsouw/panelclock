using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text.RegularExpressions;

namespace WhiteMagic.PanelClock
{
    public class Label : Component
    {
        private string _format;
        private float[] _paddings = { 0, 0, 0, 0 };
        private float _fontHeightInPixels;
        private float _actualTextWidth;
        private string _fontname;
        private Font _font;
        private float _x;
        private float _y;
        private float _width;
        private float _height;
        private RectangleF _textBox;
        private RectangleF _backgroundBox;
        private List<Func<string>> _parts = new List<Func<string>>();

        public Label(string id, ILogger logger) : base(id, logger)
        {
            _fontname = "Arial";
            BackgroundColor = Color.Transparent;
            HorizontalAlignment = Alignment.Left;
            VerticalAlignment = Alignment.Top;
            TextColor = Color.White;

            AddProperty(Create("x", () => _x, (obj) => X = obj));
            AddProperty(Create("y", () => _y, (obj) => Y = obj));
            AddProperty(Create("x2", () => X + Width));
            AddProperty(Create("y2", () => Y + Height));
            AddProperty(Create("x3", () => X + Width * ShowOrHideAnimationElapsed));
            AddProperty(Create("y3", () => Y + Height * ShowOrHideAnimationElapsed));
            AddProperty(Create("format", () => Y, (obj) => Format = obj));
            AddProperty(Create("width", () => Width, (obj) => Width = obj));
            AddProperty(Create("height", () => Height, (obj) => Height = obj));
            AddProperty(Create("fontname", () => FontName, (obj) => FontName = obj));
            AddProperty(Create("textcolor", () => TextColor, (obj) => TextColor = obj));
            AddProperty(Create("backgroundcolor", () => BackgroundColor, (obj) => BackgroundColor = obj));
            AddProperty(Create("horizontalalignment", () => HorizontalAlignment, (obj) => HorizontalAlignment = obj.ToEnum<Alignment>()));
            AddProperty(Create("verticalalignment", () => VerticalAlignment, (obj) => VerticalAlignment = obj.ToEnum<Alignment>()));
        }

        #region IComponent

        public override IComponent Clone(string id)
        {
            var copy = new Label(id, Logger);

            copy._width = _width;
            copy._height = _height;
            copy.X = X;
            copy.Y = Y;
            copy.FontName = _fontname;
            copy.TextColor = TextColor;
            copy.BackgroundColor = BackgroundColor;
            copy._paddings = _paddings.ToArray();
            copy.HorizontalAlignment = HorizontalAlignment;
            copy.VerticalAlignment = VerticalAlignment;
            copy.Visible = Visible;
            copy.ShowOrHideTime = ShowOrHideTime;
            copy.Format = Format;

            return copy;
        }

        #endregion

        #region IDrawable

        public override void Draw(Graphics graphics)
        {
            EvaluateText();

            if (!Visible && !ShowingOrHiding)
                return;

            var elapsed = (float)ShowOrHideAnimationElapsed;
            var textcolor = TextColor;
            var backgroundcolor = BackgroundColor;
            if (ShowingOrHiding)
            {
                textcolor = textcolor.Scale(elapsed);
                backgroundcolor = backgroundcolor.Scale(elapsed);
            }

            if (BackgroundColor.A > 0)
            {
                graphics.FillRectangle(new SolidBrush(backgroundcolor), _backgroundBox);
            }
            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            graphics.SetClip(_backgroundBox);
            graphics.DrawString(Text, _font, new SolidBrush(textcolor), _textBox);
        }

        #endregion

        #region Properties

        public float Width { get { return _backgroundBox.Width; } set { SetWidth(value); } }
        public float Height { get { return _backgroundBox.Height; } set { SetHeight(value); } }
        public float X { get { return _x; } set { SetX(value); } }
        public float Y { get { return _y; } set { SetY(value); } }
        public string FontName { get { return _fontname; } set { SetFontName(value); } }
        public string Text { get; private set; }
        public Color TextColor { get; set; }
        public Color BackgroundColor { get; set; }
        public float HorizontalPadding { set { _paddings[0] = _paddings[2] = value; } } // todo: padding does not update dimensions
        public float LeftPadding { get { return _paddings[0]; } set { _paddings[0] = value; } }
        public float RightPadding { get { return _paddings[2]; } set { _paddings[2] = value; } }
        public Alignment HorizontalAlignment { get; set; }
        public Alignment VerticalAlignment { get; set; }
        public string Format { get { return _format; } set{ SetFormat(value); } }

        #endregion

        #region private code

        private void SetFormat(string format)
        {
            _format = format;

            var tok = new Tokenizer(format);
            var parts = new List<Func<string>>();

            var literal = "";
            while (!tok.EOF)
            {
                if (!tok.Identifier(out var id, false))
                {
                    literal += tok.Next();
                }
                else if (tok.Match('('))
                {
                    if (literal != "")
                    {
                        var copy = literal;
                        parts.Add(() => copy);
                        literal = "";
                    }
                    var args = "";
                    while (!tok.Match(')'))
                    {
                        args += tok.Next();
                    }
                    parts.Add(ParseFunction(id, args));
                }
                else
                {
                    literal += id;
                }
            }
            if (literal != "")
            {
                parts.Add(() => literal);
            }
            _parts = parts;
        }

        private void SetX(float x)
        {
            if (x != _x)
            {
                _x = x;
                SetDimensions();
            }
        }

        private void SetY(float y)
        {
            if (y != _y)
            {
                _y = y;
                SetDimensions();
            }
        }

        private void SetWidth(float width)
        {
            if (width != _width)
            {
                _width = width;
                SetDimensions();
            }
        }

        private void SetHeight(float height)
        {
            if (height != _height)
            {
                _height = height;
                SetDimensions();
            }
        }

        private void SetFontName(string fontname)
        {
            if (fontname == _fontname)
                return;

            var parts = fontname.Split(":");
            var fontsize = 12;
            FontStyle fontstyle = 0;
            foreach (var part in parts.Skip(1).Select(p => p.ToLower()))
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
                    fontstyle |= FontStyle.Italic;
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
            SetText(Text, true);
        }

        private void EvaluateText()
        {
            var text = "";
            foreach (var part in _parts)
            {
                text += part();
            }
            SetText(text);
        }

        private void SetText(string txt, bool force = false)
        {
            if (Text != txt || force)
            {
                Text = txt;
                _actualTextWidth = Graphics.FromImage(new Bitmap(1, 1)).MeasureString(Text, _font).Width;
#if !SIMULATION
                _actualTextWidth += 2;
#endif
                SetDimensions();
            }
        }

        private Func<string> ParseFunction(string function, string args)
        {
            if (function == "wday")
            {
                if (args == "#")
                    return () => (((int)DateTime.UtcNow.DayOfWeek + 1) % 7).ToString();
                var match = Regex.Match(args, @"^\[(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+)\]$");
                if (match.Success)
                {
                    return () => match.Groups[((int)DateTime.Now.DayOfWeek + 6) % 7 + 1].ToString();
                }
            }
            else if (function == "month")
            {
                if (args == "#")
                    return () => DateTime.UtcNow.Month.ToString();
                var match = Regex.Match(args, @"^\[(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+),(\w+)\]$");
                if (match.Success)
                {
                    return () => match.Groups[DateTime.Now.Month].ToString();
                }
            }
            else if (function == "mday")
            {
                if (args == "#")
                    return () => DateTime.Now.Day.ToString();
            }
            else if (function == "hours")
            {
                if (args == "24H")
                    return () => DateTime.Now.Hour.ToString("D2");
                if (args == "12H")
                    return () => (DateTime.Now.Hour % 12).ToString("D2");
            }
            else if (function == "minutes")
            {
                if (args == "")
                    return () => DateTime.Now.Minute.ToString("D2");
            }
            else if (function == "seconds")
            {
                if (args == "")
                    return () => DateTime.Now.Second.ToString("D2");
            }
            return () => $"{function}({args})";
        }

        private void SetDimensions()
        {
            var tw = _actualTextWidth + _paddings[0] + _paddings[2];
            var th = _fontHeightInPixels + _paddings[1] + _paddings[3];
            var bh = _height != 0 ? _height : th;
            var x = HorizontalAlignment switch
            {
                Alignment.Left => X,
                Alignment.Center => (_width == 0) ? (X - tw / 2) : (X + (_width - tw) / 2),
                _ => (_width == 0) ? (X - tw) : (X + _width)
            };
            var y = VerticalAlignment switch
            {
                Alignment.Top => Y,
                Alignment.Center => Y - bh / 2,
                _ => Y - bh
            };
            _backgroundBox = new RectangleF(x, y, _width == 0 ? tw : _width, _height == 0 ? th : _height);

            x = HorizontalAlignment switch
            {
                Alignment.Left => X + _paddings[0],
                Alignment.Center => (_width == 0) ? (X - _actualTextWidth / 2) : (X + (Width - _actualTextWidth) / 2),
                _ => (_width == 0) ? (X - _actualTextWidth - _paddings[2]) : (X + _width - _actualTextWidth - _paddings[2])
            };
            y = VerticalAlignment switch
            {
                Alignment.Top => y + (bh - th) / 2,
                Alignment.Center => Y - _fontHeightInPixels / 2,
                _ => Y - _fontHeightInPixels - _paddings[3]
            };
            _textBox = new RectangleF(x, y - 1, tw, th);
        }

        #endregion
    }
}
