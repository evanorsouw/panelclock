using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class ValueSource : IValueSource
    {
        private Func<object> _getter = ()  => null;
        private Action<object> _setter = obj => {};
        private List<ValueSource> _properties = new List<ValueSource>();

        protected ValueSource(string name)
        {
            NativeType = typeof(object);
            Id = name;
        }

        protected ValueSource(string name, Type type)
        {
            NativeType = type;
            Id = name;
        }

        protected ValueSource(string name, Type targetType, Func<object> getter, Action<object> setter, IEnumerable<ValueSource> properties)
        {
            Id = name;
            NativeType = targetType;
            _getter = () =>
            {
                var xx = name;
                var value = getter();
                if (value.GetType() == targetType)
                    return value;

                if (targetType.IsEnum)
                {
                    if (Enum.TryParse(targetType, value.ToString(), true, out var enumvalue))
                    {
                        value = enumvalue;
                    }
                }
                else if (targetType == typeof(bool))
                {
                    if (value.GetType() == typeof(string))
                    {
                        if (bool.TryParse(value.ToString(), out var boolvalue))
                            value = boolvalue;
                    }
                    else if (value.GetType() == typeof(int))
                    {
                        value = (int)value > 0;
                    }
                    else if (value.GetType() == typeof(double))
                    {
                        value = (double)value > 0;
                    }
                    else if (value.GetType() == typeof(float))
                    {
                        value = (float)value > 0;
                    }
                }
                else if (targetType == typeof(Color))
                {
                    var s = value.ToString();
                    if (int.TryParse(s, System.Globalization.NumberStyles.HexNumber, null, out var rgb))
                    {
                        value = Color.FromArgb(0xFF, (byte)((rgb >> 16) & 0xFF), (byte)((rgb >> 8) & 0xFF), (byte)(rgb & 0xFF));
                    }
                    else
                    {
                        value = Color.FromName(value.ToString());
                    }
                } 
                else if (targetType == typeof(float))
                {
                    if (value.GetType() == typeof(int))
                    {
                        value = (float)(int)value;
                    }
                    else if (value.GetType() == typeof(string))
                    {
                        if (float.TryParse(value.ToString(), out var ival))
                            value = ival;
                    }
                    else
                    {
                        value = (float)(double)value;
                    }
                }
                return value;
            };
            _setter = setter;
            _properties = properties.ToList();
        }

        protected ValueSource(string name, Func<object> getter) : this(name)
        {
            _getter = getter;
        }

        public static ValueSource Create(string name, ValueSource expression)
        {
            var expr = new ValueSource(name);
            expr.NativeType = expression.NativeType;
            expr._getter = expression._getter;
            expr._setter = expression._setter;
            expr._properties = expression._properties;
            return expr;
        }

        public static ValueSource Create(string name, Type type, ValueSource expression)
        {
            return new ValueSource(name, type, expression._getter, expression._setter, expression._properties);
        }

        public static ValueSource Create(string name)
        {
            return new ValueSource(name);
        }

        public static ValueSource Create(Func<object> getter)
        {
            return new ValueSource("", typeof(object), getter, obj => { }, new List<ValueSource>());
        }

        public static ValueSource Create(string name, Type type, Func<object> getter)
        {
            return new ValueSource(name, type, getter, obj => { }, new List<ValueSource>());
        }

        public static ValueSource Create(string name, Type type, Func<object> getter, Action<object> setter)
        {
            return new ValueSource(name, type, getter, setter, new List<ValueSource>());
        }

        public static ValueSource Create(string name, Func<object> getter, Action<object> setter, IEnumerable<ValueSource> properties)
        {
            return new ValueSource(name, typeof(object), getter, setter, properties);
        }

        public static ValueSource Create(string name, Func<object> getter, IEnumerable<ValueSource> properties)
        {
            return new ValueSource(name, typeof(object), getter, obj => { }, properties);
        }

        public ValueSource AddProperty(ValueSource expr)
        {
            _properties.Add(expr);
            return this;
        }

        public ValueSource GetProperty(string name)
        {
            return _properties.FirstOrDefault(p => p.Id == name);
        }

        public string Id { get; private set; }

        public object Value
        {
            get { return _getter(); }
            set { _setter?.Invoke(value); }
        }

        public Type NativeType { get; private set; }

        public bool Writable => _setter != null;

        public ValueSource this[string name] => _properties.FirstOrDefault(p => p.Id == name);

        public IEnumerable<string> Properties => _properties.Select(p => p.Id);
    }
}