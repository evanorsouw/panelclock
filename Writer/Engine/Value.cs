using System;
using System.Drawing;

namespace WhiteMagic.PanelClock.Engine
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
        public static implicit operator Value(DateTime v) => new Value(v);

        public static implicit operator string(Value v) { return v.ToString(); }
        public static implicit operator bool(Value v) { return v.ToBool(); }
        public static implicit operator int(Value v) { return v.ToInt(); }
        public static implicit operator float(Value v) { return v.ToFloat(); }
        public static implicit operator double(Value v) { return v.ToDouble(); }
        public static implicit operator Color(Value v) { return v.ToColor(); }
        public static implicit operator DateTime(Value v) { return v.ToDateTime(); }

        public object Raw => _value;

        public static bool operator true(Value v) { return true; }

        public static bool operator false(Value v) { return false; }

        public static bool operator ==(Value lhs, Value rhs)
        {
            if (lhs._value.GetType() == rhs._value.GetType())
                return lhs.Equals(rhs);
            if (lhs._value.GetType() == typeof(bool) || rhs._value.GetType() == typeof(bool))
                return lhs.ToBool() == rhs.ToBool();
            if (lhs._value.GetType() == typeof(DateTime) || rhs._value.GetType() == typeof(DateTime))
                return lhs.ToDateTime() == rhs.ToDateTime();
            if (lhs._value.GetType() == typeof(string) || rhs._value.GetType() == typeof(string))
                return lhs.ToString() == rhs.ToString();

            return lhs.ToDouble() == rhs.ToDouble();
        }

        public static bool operator !=(Value lhs, Value rhs)
        {
            return !(lhs == rhs);
        }

        public static bool operator <(Value lhs, Value rhs)
        {
            if (lhs._value.GetType() == typeof(bool) || rhs._value.GetType() == typeof(bool))
                throw new Exception("cannot relative compare booleans");
            if (lhs._value.GetType() == typeof(DateTime) || rhs._value.GetType() == typeof(DateTime))
                return lhs.ToDateTime().CompareTo(rhs.ToDateTime()) < 0;
            if (lhs._value.GetType() == typeof(string) || rhs._value.GetType() == typeof(string))
                return lhs.ToString().CompareTo(rhs.ToString()) < 0;
            return lhs.ToDouble() < rhs.ToDouble();
        }

        public static bool operator <=(Value lhs, Value rhs)
        {
            return lhs < rhs || lhs == rhs;
        }

        public static bool operator >(Value lhs, Value rhs)
        {
            if (lhs._value.GetType() == typeof(bool) || rhs._value.GetType() == typeof(bool))
                throw new Exception("cannot relative compare booleans");
            if (lhs._value.GetType() == typeof(string) || rhs._value.GetType() == typeof(string))
                return lhs.ToDateTime().CompareTo(rhs.ToDateTime()) > 0;
            if (lhs._value.GetType() == typeof(string) || rhs._value.GetType() == typeof(string))
                return lhs.ToString().CompareTo(rhs.ToString()) > 0;
            return lhs.ToDouble() > rhs.ToDouble();
        }

        public static bool operator >=(Value lhs, Value rhs)
        {
            return lhs > rhs || lhs == rhs;
        }

        public static Value operator | (Value lhs, Value rhs)
        {
            return lhs.ToBool() || rhs.ToBool();
        }

        public static Value operator & (Value lhs, Value rhs)
        {
            return lhs.ToBool() && rhs.ToBool();
        }

        public static Value operator +(Value lhs, Value rhs)
        {
            if (lhs._value.GetType() == typeof(string) || rhs._value.GetType() == typeof(string))
                return lhs._value.ToString() + rhs._value.ToString();
            return lhs.ToDouble() + rhs.ToDouble();
        }

        public static Value operator -(Value lhs, Value rhs)
        {
            return lhs.ToDouble() - rhs.ToDouble();
        }

        public static Value operator *(Value lhs, Value rhs)
        {
            return lhs.ToDouble() * rhs.ToDouble();
        }

        public static Value operator /(Value lhs, Value rhs)
        {
            return lhs.ToDouble() / rhs.ToDouble();
        }

        public static Value operator %(Value lhs, Value rhs)
        {
            return lhs.ToDouble() % rhs.ToDouble();
        }

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
            if (_value.GetType() == typeof(string))
                return _value.ToString().Length > 0;
            return false;
        }

        public override string ToString()
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
            if (_value.GetType() == typeof(int))
                return (int)_value;
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
                return (float)_value;
            if (_value.GetType() == typeof(int))
                return (int)_value;
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
            if (_value.GetType() == typeof(double))
            {
                var color = (long)(double)_value;
                return Color.FromArgb(255, (int)((color >> 16) & 0xFF), (int)((color >> 8) & 0xFF), (int)(color & 0xFF));
            }
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

        public DateTime ToDateTime()
        {
            if (_value.GetType() == typeof(DateTime))
                return (DateTime)_value;
            if (_value.GetType() == typeof(string))
            {
                if (DateTime.TryParse(_value.ToString(), out var timestamp))
                    return timestamp;
            }
            return DateTime.MinValue;
        }
    }
}