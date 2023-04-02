using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using WhiteMagic.PanelClock.Components;
using WhiteMagic.PanelClock.Engine;

namespace WhiteMagic.PanelClock
{
    public class ConfigurationParser
    {
        private IConfiguration _config;
        private ILogger _logger;
        private IFunctionFactory _factory;

        public ConfigurationParser(IConfiguration config, IFunctionFactory factory, ILogger logger)
        {
            _config = config;
            _factory = factory;
            _logger = logger;
        }

        public Stock Parse()
        {
            var stock = new Stock();
            foreach (var exprCfg in _config.GetSection("expressions").GetChildren())
            {
                var name = exprCfg.Key;
                var expression = ValueSource.Create(name);
                stock.AddValueSource(expression);
            }

            foreach (var itemCfg in _config.GetSection("items").GetChildren())
            {
                var item = ScanItem("", itemCfg);
                if (item == null)
                    continue;
                stock.AddItem(item);
            }

            foreach (var sceneCfg in _config.GetSection("scenes").GetChildren())
            {
                var scene = ScanScene(stock, sceneCfg);
                if (scene == null)
                    break;
                stock.AddScene(scene);
            }

            foreach (var exprCfg in _config.GetSection("expressions").GetChildren())
            {
                var name = exprCfg.Key;
                var value = exprCfg.Value;
                var expression = ParseExpression("", stock, value);
                stock.AddValueSource(ValueSource.Create(name, expression));
            }

            foreach (var itemCfg in _config.GetSection("items").GetChildren())
            {
                ParseItem("", stock, itemCfg);
            }

            foreach (var sceneCfg in _config.GetSection("scenes").GetChildren())
            {
                ParseScene(stock, sceneCfg);
            }
            return stock;
        }

        private Scene ScanScene(Stock stock, IConfigurationSection sceneCfg)
        {
            var id = sceneCfg["id"];
            if (id == null)
                return null;

            var scene = new Scene(id, stock);
            scene.CronSpec = sceneCfg["cron"] ?? "* * * * * *";

            foreach (var exprCfg in sceneCfg.GetSection("expressions").GetChildren())
            {
                var name = id + "." + exprCfg.Key;
                var expression = ValueSource.Create(name);
                stock.AddValueSource(expression);
            }

            foreach (var itemCfg in sceneCfg.GetSection("items").GetChildren())
            {
                string itemId = null;
                var item = ScanItem(id, itemCfg);
                if (item != null)
                {
                    itemId = item.Id;
                    stock.AddItem(item);
                }
                else
                {
                    itemId = itemCfg["id"];
                }
                if (itemId != null)
                {
                    scene.AddItem(itemId);
                }
            }
            return scene;
        }

        private Scene ParseScene(Stock stock, IConfigurationSection sceneCfg)
        {
            var id = sceneCfg["id"];
            if (id == null)
                return null;

            var scene = stock.GetScene(id);

            foreach (var itemCfg in sceneCfg.GetSection("items").GetChildren())
            {
                ParseItem(id, stock, itemCfg);
            }
            return scene;
        }

        private void ParseItem(string prefix, Stock stock, IConfigurationSection itemCfg)
        {
            var id = itemCfg["id"];
            if (id == null)
                return;

            var item = stock.GetItem(id);
            if (item == null)
            {
                item = stock.GetItem($"{prefix}.{id}");
            }
            if (item != null)
            {     
                ParseItemProperties(prefix, stock, itemCfg, item);
            }
            return;
        }

        private Component ScanItem(string prefix, IConfigurationSection itemCfg)
        {
            var id = itemCfg["id"];
            if (id == null)
                return null;

            if (prefix != "")
            {
                id = prefix + "." + id;
            }
            var type = itemCfg["type"];
            if (type == null)
                return null;

            Component item = null;
            if (type == "label")
            {
                item = new Label(id, _logger);
            }
            else if (type == "analogclock-modern")
            {
                item = new AnalogClockModern(id, _logger);
            }
            else if (type == "ticker")
            {
                item = new Ticker(id, _logger);
            }
            else if (type == "icon")
            {
                item = new Icon(id, _logger);
            }
            else if (type == "windindicator")
            {
                item = new WindIndicator(id, _logger);
            }
            else
            {
                _logger.LogWarning($"unrecognized scene item='{type}'");
            }
            return item;
        }

        private void ParseItemProperties(string prefix, Stock stock, IConfigurationSection itemCfg, ValueSource item)
        {
            foreach (var property in item.Properties.Select(prop => item[prop]))
            {
                var name = property.Id.ToLower();
                var expression = itemCfg[name];
                if (expression == null)
                    continue;

                var value = ParseExpression(prefix, stock, expression);
                if (value == null)
                    continue;

                var namedvalue = ValueSource.Create($"{item.Id}.{name}", value);
                stock.AddValueSource(namedvalue);
                stock.AddAssignment(new Assignment(() => { property.Value = namedvalue.Value; }));
            }
        }

        private ValueSource ParseExpression(string prefix, Stock stock, string value)
        {
            ValueSource expression = null;
            var tok = new Tokenizer(value);
            if (tok.Match("="))
            {
                expression = ParseConditional(prefix, stock, tok);
            }
            else
            {
                expression = ParseValue(stock, tok);
            }
            if (!tok.EOF)
                tok.ThrowException("unexpected characters");

            return expression;
        }

        private ValueSource ParseConditional(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseOr(prefix, stock, tok);
            if (tok.Match("?"))
            {
                var conditionExpression = expression;
                var trueExpression = ParseConditional(prefix, stock, tok);
                if (!tok.Match(":"))
                    tok.ThrowException("':' expected after '?' ");
                var falseExpression = ParseConditional(prefix, stock, tok);
                expression = ValueSource.Create(() => (bool)conditionExpression.Value ? trueExpression.Value : falseExpression.Value);
            }
            return expression;
        }

        private ValueSource ParseOr(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseAnd(prefix, stock, tok);
            while (tok.Match("||"))
            {
                var lhs = expression;
                var rhs = ParseAnd(prefix, stock, tok);
                expression = ValueSource.Create(() => lhs.Value || rhs.Value);
            }
            return expression;
        }

        private ValueSource ParseAnd(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseRelational(prefix, stock, tok);
            while (tok.Match("&&"))
            {
                var lhs = expression;
                var rhs = ParseRelational(prefix, stock, tok);
                expression = ValueSource.Create(() => lhs.Value && rhs.Value);
            }
            return expression;
        }


        private ValueSource ParseRelational(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseEquality(prefix, stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("<="))
                {
                    var rhs = ParseEquality(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value <= rhs.Value);
                }
                else if (tok.Match(">="))
                {
                    var rhs = ParseEquality(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value >= rhs.Value);
                }
                else if (tok.Match("<"))
                {
                    var rhs = ParseEquality(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value < rhs.Value);
                }
                else if (tok.Match(">"))
                {
                    var rhs = ParseEquality(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value > rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseEquality(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseAddition(prefix, stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("=="))
                {
                    var rhs = ParseAddition(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value == rhs.Value);
                }
                else if (tok.Match("!="))
                {
                    var rhs = ParseAddition(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value != rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseAddition(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseMultiplication(prefix, stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("+"))
                {
                    var rhs = ParseMultiplication(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value + rhs.Value);
                }
                else if (tok.Match("-"))
                {
                    var rhs = ParseMultiplication(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value - rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseMultiplication(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseUnary(prefix, stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("*"))
                {
                    var rhs = ParseUnary(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value * rhs.Value);
                }
                else if (tok.Match("/"))
                {
                    var rhs = ParseUnary(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value / rhs.Value);
                }
                else if (tok.Match("%"))
                {
                    var rhs = ParseUnary(prefix, stock, tok);
                    expression = ValueSource.Create(() => lhs.Value % rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseUnary(string prefix, Stock stock, Tokenizer tok)
        {
            ValueSource expression = null;

            if (tok.Match("!"))
            {
                var notExpression = ParseSubExpression(prefix, stock, tok);
                expression = ValueSource.Create(() => !notExpression.Value);
            }
            else
            {
                expression = ParseSubExpression(prefix, stock, tok);
            }

            return expression;
        }

        private ValueSource ParseSubExpression(string prefix, Stock stock, Tokenizer tok)
        {
            ValueSource expression;

            if (tok.Match("("))
            {
                expression = ParseConditional(prefix, stock, tok);
                if (!tok.Match(")"))
                    tok.ThrowException("missing closing brace");
            }
            else
            {
                expression = ParseTerminal(prefix, stock, tok);
            }

            return expression;
        }

        private ValueSource ParseSourceProperty(Stock stock, Tokenizer tok, ValueSource expression)
        {
            if (!tok.Identifier(out var identifier))
                throw new Exception("identifier expected after '.'");

            var subExpression = expression.GetProperty(identifier);
            if (subExpression == null)
                tok.ThrowException($"object='{expression.Id}' does not have a property='{identifier}'");

            return subExpression;
        }

        private ValueSource ParseTerminal(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseValueOrIdentifier(prefix, stock, tok);
            while (tok.Match("."))
            {
                expression = ParseSourceProperty(stock, tok, expression);
            }
            return expression;
        }

        private ValueSource ParseValue(Stock stock, Tokenizer tok)
        {
            var expression = ParseNumberOrBoolean(stock, tok);
            if (expression == null)
            {
                var value = tok.Remaining();
                expression = ValueSource.Create(() => value);
            }
            return expression;
        }

        private ValueSource ParseNumberOrBoolean(Stock stock, Tokenizer tok)
        {
            ValueSource expression = null;

            if (tok.Match("true", true))
            {
                expression = ValueSource.Create(() => true);
            }
            else if (tok.Match("false", true))
            {
                expression = ValueSource.Create(() => false);
            }
            else if (tok.Number(out var number))
            {
                expression = ValueSource.Create(() => number);
            }
            return expression;
        }

        private ValueSource ParseValueOrIdentifier(string prefix, Stock stock, Tokenizer tok)
        {
            var expression = ParseNumberOrBoolean(stock, tok);
            if (expression == null)
            {
                if (tok.String(out var literal))
                {
                    expression = ValueSource.Create(() => literal);
                }
                else if (tok.Identifier(out var identifier))
                {
                    if (tok.Match('('))
                    {
                        expression = ParseFunction(prefix, stock, tok, identifier);
                    }
                    else
                    {
                        expression = _factory.CreateFunction(identifier, new List<ValueSource>());
                        if (expression == null)
                        {
                            expression = ParseIdentifierExpression(identifier, stock, tok);
                        }
                        if (expression == null && prefix != "")
                        {
                            expression = ParseIdentifierExpression(prefix + "." + identifier, stock, tok);
                        }
                        if (expression == null)
                            tok.ThrowException($"identifier='{identifier}' is not an expression nor an item");
                    }
                }
            }
            return expression;
        }

        private ValueSource ParseIdentifierExpression(string identifier, Stock stock, Tokenizer tok)
        {
            var expression = stock.GetValueSource(identifier);
            if (expression == null)
            {
                expression = stock.GetItem(identifier);
            }
            return expression;
        }

        private ValueSource ParseFunction(string prefix, Stock stock, Tokenizer tok, string functionname)
        {
            var arguments = new List<ValueSource>();
            if (!tok.Match(")"))
            {
                arguments = ParseArguments(prefix, stock, tok);
                if (!tok.Match(")"))
                    tok.ThrowException("missing closing brace after arguments");
            }
            var expression = _factory.CreateFunction(functionname, arguments);
            if (expression != null)
                return expression;

            tok.ThrowException($"invalid function='{functionname}'");
            return null;
        }

        private List<ValueSource> ParseArguments(string prefix, Stock stock, Tokenizer tok)
        {
            var arguments = new List<ValueSource>();
            do
            {
                var argument = ParseOr(prefix, stock, tok);
                arguments.Add(argument);
            }
            while (tok.Match(','));

            return arguments;
        }
    }
}