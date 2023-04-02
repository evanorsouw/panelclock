using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text.RegularExpressions;
using WhiteMagic.PanelClock.Components;
using WhiteMagic.PanelClock.DomainTypes;
using WhiteMagic.PanelClock.Engine;
using WhiteMagic.PanelClock.Extensions;

namespace WhiteMagic.PanelClock
{
    public class Label : Component
    {
        private string _format;
        private string _fontname;
        private float _textWidth;
        private float _textHeight;
        private List<Func<string>> _parts = new List<Func<string>>();
        private Font _font;

        public Label(string id, ILogger logger) : base(id, logger)
        {
            FontName = "Arial";
            BackgroundColor = Color.Transparent;
            HorizontalAlignment = Alignment.TopOrLeft;
            VerticalAlignment = Alignment.TopOrLeft;
            TextColor = Color.White;

            AddProperty(Create("format", () => Y, (obj) => Format = obj));
            AddProperty(Create("width", () => Width, (obj) => Width = obj));
            AddProperty(Create("height", () => Height, (obj) => Height = obj));
            AddProperty(Create("fontname", () => FontName, (obj) => FontName = obj));
            AddProperty(Create("textcolor", () => TextColor, (obj) => TextColor = obj));
            AddProperty(Create("backgroundcolor", () => BackgroundColor, (obj) => BackgroundColor = obj));
        }

        #region IDrawable

        public override void Draw(Graphics graphics)
        {
            if (!InternalVisible && !ShowingOrHiding)
                return;

            EvaluateText();

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
                graphics.FillRectangle(new SolidBrush(backgroundcolor), BackgroundBox);
            }
            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            graphics.SetClip(BackgroundBox);
            graphics.DrawString(Text, _font, new SolidBrush(textcolor), ContentTopLeft);
        }

        #endregion

        #region Properties

        public string FontName { get { return _fontname; } set { SetFontName(value); } }
        public string Text { get; private set; }
        public Color TextColor { get; set; }
        public Color BackgroundColor { get; set; }
        public string Format { get { return _format; } set{ SetFormat(value); } }
        public float TextWidth => _textWidth;
        public float TextHeight => _textHeight;
        public Font Font => _font;

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
            _textHeight = graphics.MeasureString("ZQdg", _font).Height;
#if SIMULATION
            _textHeight -= 1;
#else
            _textHeight += 1;
#endif
            _fontname = fontname;
            SetText(Text, true);
        }

        protected void EvaluateText()
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
                _textWidth = Graphics.FromImage(new Bitmap(1, 1)).MeasureString(Text, _font).Width;
#if !SIMULATION
                _textWidth += 2;
#endif
                CoordinatesChanged();
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

        protected override void GetFullSize(out float width, out float height)
        {
            width = _textWidth;
            height = _textHeight;
        }

        #endregion
    }
}
