using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace writer
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

        public string identifier()
        {
            if (!char.IsLetter(Head) && Head != '_')
                return null;

            string id = "";
            while (char.IsLetterOrDigit(Head) || Head == '_')
            {
                id += Next();
            }
            return id;
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

    }
}
