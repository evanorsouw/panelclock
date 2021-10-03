using System;
using System.Collections.Generic;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class ValueSource : IValueSource
    {
        private Func<Value> _getter = ()  => 0;
        private Action<Value> _setter = obj => {};
        private List<ValueSource> _properties = new List<ValueSource>();

        protected ValueSource(string name)
        {
            Id = name;
        }

        protected ValueSource(string name, Func<Value> getter, Action<Value> setter, IEnumerable<ValueSource> properties)
        {
            Id = name;
            _getter = getter;
            _setter = setter;
            _properties = properties.ToList();
        }

        public static ValueSource Create(string name, ValueSource expression)
        {
            var expr = new ValueSource(name);
            expr._getter = expression._getter;
            expr._setter = expression._setter;
            expr._properties = expression._properties;
            return expr;
        }

        public static ValueSource Create(string name)
        {
            return new ValueSource(name);
        }

        public static ValueSource Create(Func<Value> getter)
        {
            return new ValueSource("", getter, obj => { }, new List<ValueSource>());
        }

        public static ValueSource Create(string name, Func<Value> getter)
        {
            return new ValueSource(name, getter, obj => { }, new List<ValueSource>());
        }

        public static ValueSource Create(string name, Func<Value> getter, Action<Value> setter)
        {
            return new ValueSource(name, getter, setter, new List<ValueSource>());
        }

        protected ValueSource SetGetter(Func<Value> getter)
        {
            _getter = getter;
            return this;
        }

        protected ValueSource AddProperty(ValueSource expr)
        {
            _properties.Add(expr);
            return this;
        }

        public ValueSource GetProperty(string name)
        {
            return _properties.FirstOrDefault(p => p.Id == name);
        }

        #region IValueSource

        public string Id { get; private set; }

        public Value Value
        {
            get { return _getter(); }
            set { _setter?.Invoke(value); }
        }

        public bool Writable => _setter != null;

        public ValueSource this[string name] => _properties.FirstOrDefault(p => p.Id == name);

        public IEnumerable<string> Properties => _properties.Select(p => p.Id);

        #endregion
    }
}