using System;

namespace WhiteMagic.PanelClock
{
    public class Tokenizer
    {
        private string _s;
        private int _head;

        public Tokenizer(string s)
        {
            _s = s;
        }

        public bool Match(char c)
        {
            if (Head != c)
                return false;

            Next();
            return true;
        }

        public bool Match(string s)
        {
            var pos = SkipWhites();
            foreach(char c in s)
            {
                if (!Match(c))
                {
                    Restore(pos);
                    return false;
                }
            }
            return true;
        }

        public int SkipWhites()
        {
            while (char.IsWhiteSpace(Head))
            {
                Next();
            }
            return _head;
        }

        public int Save()
        {
            return _head;
        }

        public void Restore(int pos)
        {
            _head = pos;
        }

        public bool Identifier(out string id, bool skipWhites=true)
        {
            if (skipWhites)
            {
                SkipWhites();
            }
            id = null;
            if (!char.IsLetter(Head) && Head != '_')
                return false;

            id = "";
            while (char.IsLetterOrDigit(Head) || Head == '_')
            {
                id += Next();
            }
            return true;
        }

        public bool Number(out double value, bool skipWhites=true)
        {
            if (skipWhites)
            {
                SkipWhites();
            }
            value = 0;
            if (!char.IsNumber(Head) && Head != '-')
                return false;

            var negative = Match('-');
            while (char.IsDigit(Head))
            {
                value = value * 10 + (Next() - '0');
            }
            if (Match('.'))
            {
                if (!char.IsDigit(Head))
                    throw new Exception("invalid number");
                var add = 0.1;
                while (char.IsDigit(Head))
                {
                    value += add * (Next() - '0');
                    add /= 10;
                }
            }
            value = negative ? -value : value;
            return true;
        }

        public bool String(out string value, bool skipWhites=true)
        {
            if (skipWhites)
            {
                SkipWhites();
            }
            
            value = "";

            if (!Match('\''))
                return false;

            var escaped = false;
            while (!EOF)
            {
                if (!escaped && Match('\''))
                    return true;

                if (Head == '\\')
                {
                    Next();
                    escaped = true;
                }
                else
                {
                    value += Next();
                    escaped = false;
                }
            }

            throw new Exception("unterminated string");
        }

        public char Head
        {
            get
            {
                if (_head == _s.Length)
                    return (char)0;
                return _s[_head];
            }
        }

        public bool EOF
        {
            get { return _head == _s.Length; }
        }

        public char Next()
        {
            char c = Head;
            if (_head < _s.Length)
            { 
                _head++; 
            }
            return c;
        }

        public override string ToString()
        {
            return _s.Substring(_head) + (EOF ? "(EOF)" : "");
        }
    }
}
