using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text.RegularExpressions;
using writer;

namespace WhiteMagic.PanelClock
{
    public class VarLabel : Label
    {
        private string _format;
        private List<Func<string>> _parts = new List<Func<string>>();

        public VarLabel() : this(0, 0)
        {
        }

        public VarLabel(float x, float y) : base(x, y)
        {
        }

        public string Format { get { return _format; } set{ SetFormat(value); } }

        public override void Draw(Graphics graphics)
        {
            Text = CalculateText();
            base.Draw(graphics);
        }

        private void SetFormat(string format)
        {
            _format = format;

            var tok = new Tokenizer(format);
            var parts = new List<Func<string>>();

            var literal = "";
            while (!tok.EOF)
            {
                var id = tok.identifier();
                if (id == null)
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

        private string CalculateText()
        {
            var text = "";
            foreach(var part in _parts)
            {
                text += part();
            }
            return text;
        }
    }
}
