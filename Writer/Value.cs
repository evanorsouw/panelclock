using System;
using System.Drawing;

namespace WhiteMagic.PanelClock
{
    public readonly struct Value
    {
        private readonly object _value;

        private Value(object v)
        {
            _value = v;
        }

        public static implicit operator Value(string v) => new Value(v);
        public static implicit operator Value(bool v) => new Value(v);
        public static implicit operator Value(int v) => new Value(v);
        public static implicit operator Value(float v) => new Value(v);
        public static implicit operator Value(double v) => new Value(v);
        public static implicit operator Value(Color v) => new Value(v);
        public static implicit operator Value(Enum v) => new Value(v);

        public static implicit operator string(Value v) { return v.ToString(); }
        public static implicit operator bool(Value v) { return v.ToBool(); }
        public static implicit operator int(Value v) { return v.ToInt(); }
        public static implicit operator float(Value v) { return v.ToFloat(); }
        public static implicit operator double(Value v) { return v.ToDouble(); }
        public static implicit operator Color(Value v) { return v.ToColor(); }

        public bool ToBool()
        {
            if (_value.GetType() == typeof(bool))
                return (bool)_value;
            if (_value.GetType() == typeof(int))
                return (int)_value > 0;
            if (_value.GetType() == typeof(float))
                return (float)_value > 0;
            if (_value.GetType() == typeof(double))
                return (double)_value > 0;
            return false;
        }

        public string ToString()
        {
            return _value.ToString();
        }

        public int ToInt()
        {
            if (_value.GetType() == typeof(int))
                return (int)_value;
            if (_value.GetType() == typeof(float))
                return (int)(float)_value;
            if (_value.GetType() == typeof(double))
                return (int)(double)_value;
            if (_value.GetType() == typeof(string))
            {
                if (int.TryParse(_value.ToString(), out var intvalue))
                    return intvalue;
            }
            return 0;
        }

        public float ToFloat()
        {
            if (_value.GetType() == typeof(float))
                return (float)_value;
            if (_value.GetType() == typeof(double))
                return (float)(double)_value;
            if (_value.GetType() == typeof(bool))
                return ((bool)_value) ? 1f : 0f;
            if (_value.GetType() == typeof(string))
            {
                if (float.TryParse(_value.ToString(), out var floatvalue))
                    return floatvalue;
            }
            return 0f;
        }

        public double ToDouble()
        {
            if (_value.GetType() == typeof(double))
                return (double)_value;
            if (_value.GetType() == typeof(float))
                return (double)(float)_value;
            if (_value.GetType() == typeof(bool))
                return ((bool)_value) ? 1.0 : 0.0;
            if (_value.GetType() == typeof(string))
            {
                if (double.TryParse(_value.ToString(), out var doublevalue))
                    return doublevalue;
            }
            return 0.0;
        }

        private Color ToColor()
        {
            if (_value.GetType() == typeof(Color))
                return (Color)_value;
            if (_value.GetType() == typeof(int))
                return Color.FromArgb(255, (((int)_value) >> 16) & 0xFF, (((int)_value) >> 8) & 0xFF, ((int)_value) & 0xFF);
            if (_value.GetType() == typeof(string))
            {
                var s = _value.ToString();
                if (long.TryParse(s, System.Globalization.NumberStyles.HexNumber, null, out var hex))
                {
                    if (s.Length == 8)
                        return Color.FromArgb((int)(hex >> 24) & 0xFF, (int)(hex >> 16) & 0xFF, (int)(hex >> 8) & 0xFF, (int)(hex & 0xFF));
                    if (s.Length == 6)
                        return Color.FromArgb(255, (int)(hex >> 16) & 0xFF, (int)(hex >> 8) & 0xFF, (int)(hex & 0xFF));
                    if (s.Length == 3)
                        return Color.FromArgb(255, (int)((hex >> 4) & 0xF0), (int)(hex & 0xF0), (int)((hex << 4) & 0xF0));
                }
            }
            return Color.Black;
        }


        public T ToEnum<T>() where T : struct
        {
            if (Enum.TryParse<T>(_value.ToString(), true, out var enumvalue))
                return enumvalue;
            return default(T);
        }
    }
}