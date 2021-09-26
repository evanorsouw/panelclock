using System;
using System.Collections.Generic;

namespace WhiteMagic.PanelClock
{
    public interface IValueSource
    {
        public string Id { get; }
        public Type NativeType { get; }
        public object Value { get; set; }
        public bool Writable { get; }
        public ValueSource this[string name] { get; }
        public IEnumerable<string> Properties { get; }

    }
}
