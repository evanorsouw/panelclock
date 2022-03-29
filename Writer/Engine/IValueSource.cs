using System.Collections.Generic;
using WhiteMagic.PanelClock.Components;

namespace WhiteMagic.PanelClock.Engine
{
    public interface IValueSource
    {
        public string Id { get; }
        public Value Value { get; set; }
        public bool Writable { get; }
        public ValueSource this[string name] { get; }
        public IEnumerable<string> Properties { get; }

    }
}
