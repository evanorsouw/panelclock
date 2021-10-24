using Microsoft.Extensions.Logging;
using System.Collections.Generic;

namespace WhiteMagic.PanelClock
{
    public class FunctionFactory : IFunctionFactory
    {
        private EnvironmentSource _environment;
        private ILogger _logger;

        public FunctionFactory(EnvironmentSource environment, ILogger logger)
        {
            _environment = environment;
            _logger = logger;
        }

        public ValueSource CreateFunction(string functionname, List<ValueSource> arguments)
        {
            if (functionname == "color")
                return new ColorSource(arguments, _logger);
            if (functionname == "environment")
                return _environment;
            return null;
        }
    }
}
