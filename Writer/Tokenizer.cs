using System;
using System.Text;

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

        public bool Match(char c, bool ignoreCase = false)
        {
            if (ignoreCase)
            {
                if (char.ToLower(Head) != char.ToLower(c))
                    return false;
            }
            else if (Head != c)
                return false;

            Next();
            return true;
        }

        public bool Match(string s, bool ignoreCase=false)
        {
            var pos = SkipWhites();
            foreach(char c in s)
            {
                if (!Match(c, ignoreCase))
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
            if (!char.IsNumber(Head) && Head != '-' && Head !='.')
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

        public string Remaining()
        {
            var value = new StringBuilder();
            while (!EOF)
            {
                value.Append(Next());
            }
            return value.ToString();
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

        public int LineNo
        {
            get
            {
                int lineno = 1;
                for (int i = 0; i < _head; ++i)
                {
                    if (_s[i] == '\n')
                        lineno++;
                }
                return lineno;
            }
        }

        public int ColumnNo
        {
            get
            {
                int colno= 1;
                for (int i = 0; i < _head; ++i)
                {
                    if (_s[i] == '\n')
                        colno = 0;
                    colno++;
                }
                return colno;
            }
        }

        public override string ToString()
        {
            return _s.Substring(_head) + (EOF ? "(EOF)" : "");
        }

        internal void ThrowException(string msg)
        {
            throw new Exception($"{LineNo}:{ColumnNo}: {msg}");
        }
    }
}
