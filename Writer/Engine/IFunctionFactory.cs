using System.Collections.Generic;
using WhiteMagic.PanelClock.Components;

namespace WhiteMagic.PanelClock.Engine
{
    public interface IFunctionFactory
    {
        ValueSource CreateFunction(string name, List<ValueSource> arguments);
    }
}
