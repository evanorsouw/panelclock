using Microsoft.Extensions.Logging;
using System.Collections.Generic;

namespace WhiteMagic.PanelClock
{
    public interface IFunctionFactory
    {
        ValueSource CreateFunction(string name, List<ValueSource> arguments);
    }
}
