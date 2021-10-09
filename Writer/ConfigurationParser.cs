using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;

namespace WhiteMagic.PanelClock
{
    public class ConfigurationParser
    {
        private IConfiguration _config;
        private ILogger _logger;

        public ConfigurationParser(IConfiguration config, ILogger logger)
        {
            _config = config;
            _logger = logger;
        }

        public Stock Parse()
        {
            var stock = new Stock();
            foreach (var exprCfg in _config.GetSection("expressions").GetChildren())
            {
                var name = exprCfg.Key;
                var expression = ValueSource.Create(name);
                stock.AddExpression(expression);
            }

            foreach (var itemCfg in _config.GetSection("items").GetChildren())
            {
                var item = ScanItem(itemCfg);
                if (item == null)
                    continue;
                stock.AddItem(item);
            }

            foreach (var exprCfg in _config.GetSection("expressions").GetChildren())
            {
                var name = exprCfg.Key;
                var value = exprCfg.Value;
                var expression = ParseExpression(stock, value);
                stock.AddExpression(ValueSource.Create(name, expression));
            }

            foreach (var itemCfg in _config.GetSection("items").GetChildren())
            {
                ParseItem(stock, itemCfg);
            }

            foreach (var sceneCfg in _config.GetSection("scenes").GetChildren())
            { 
                var scene = ParseScene(stock, sceneCfg);
                if (scene == null)
                    break;
                stock.AddScene(scene);
            }
            return stock;
        }

        private Scene ParseScene(Stock stock, IConfigurationSection sceneCfg)
        {
            var id = sceneCfg["id"];
            if (id == null)
                return null;

            var scene = new Scene(id, stock);
            scene.CronSpec = sceneCfg["cron"];

            for (int iitem = 0; ; ++iitem)
            {
                var itemCfg = sceneCfg.GetSection($"items:{iitem}");
                var item = ParseSceneItem(itemCfg);
                if (item == null)
                    break;
                scene.AddItem(item);
            }
            return scene;
        }

        private void ParseItem(Stock stock, IConfigurationSection itemCfg)
        {
            var id = itemCfg["id"];
            if (id == null)
                return;

            var item = stock.GetItem(id);
            ParseItemProperties(stock, itemCfg, item);
        }

        private Component ScanItem(IConfigurationSection itemCfg)
        {
            var id = itemCfg["id"];
            if (id == null)
                return null;

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
            else
            {
                _logger.LogWarning($"unrecognized scene item='{type}'");
            }
            return item;
        }

        private void ParseItemProperties(Stock stock, IConfigurationSection itemCfg, ValueSource item)
        {
            foreach(var property in item.Properties.Select(prop=>item[prop]))
            {
                var name = property.Id.ToLower();
                var expression = itemCfg[name];
                if (expression == null)
                    continue;

                var value = ParseExpression(stock, expression);
                if (value == null)
                    continue;

                var namedvalue = ValueSource.Create($"{item.Id}.{name}", value);
                stock.AddExpression(namedvalue);
                stock.AddAssignment(new Assignment(() => { property.Value = namedvalue.Value; }));
            }
        }

        private SceneItem ParseSceneItem(IConfigurationSection itemCfg)
        {
            var id = itemCfg["id"];
            if (id == null)
                return null;

            var item = new SceneItem(id);

            return item;
        }

        private ValueSource ParseExpression(Stock stock, string value)
        { 
            ValueSource expression = null;
            var tok = new Tokenizer(value);
            if (tok.Match("="))
            {
                expression = ParseConditional(stock, tok);
            }
            else 
            {
                expression = ParseValue(stock, tok);
            }
            if (!tok.EOF)
                tok.ThrowException("unexpected characters");

            return expression;
        }

        private  ValueSource ParseConditional(Stock stock, Tokenizer tok)
        {
            var expression = ParseOr(stock, tok);
            if (tok.Match("?"))
            {
                var conditionExpression = expression;
                var trueExpression = ParseConditional(stock, tok);
                if (!tok.Match(":"))
                    tok.ThrowException("':' expected after '?' ");
                var falseExpression = ParseConditional(stock, tok);
                expression = ValueSource.Create(() => (bool)conditionExpression.Value ? trueExpression.Value : falseExpression.Value);
            }
            return expression;
        }

        private ValueSource ParseOr(Stock stock, Tokenizer tok)
        {
            var expression = ParseAnd(stock, tok);
            while (tok.Match("||"))
            {
                var lhs = expression;
                var rhs = ParseAnd(stock, tok);
                expression = ValueSource.Create(() => lhs.Value || rhs.Value);
            }
            return expression;
        }

        private ValueSource ParseAnd(Stock stock, Tokenizer tok)
        {
            var expression = ParseRelational(stock, tok);
            while (tok.Match("&&"))
            {
                var lhs = expression;
                var rhs = ParseRelational(stock, tok);
                expression = ValueSource.Create(() => lhs.Value && rhs.Value);
            }
            return expression;
        }


        private ValueSource ParseRelational(Stock stock, Tokenizer tok)
        {
            var expression = ParseEquality(stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("<"))
                {
                    var rhs = ParseEquality(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value < rhs.Value);
                }
                else if (tok.Match("<="))
                {
                    var rhs = ParseEquality(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value <= rhs.Value);
                }
                else if (tok.Match(">"))
                {
                    var rhs = ParseEquality(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value > rhs.Value);
                }
                else if (tok.Match(">="))
                {
                    var rhs = ParseEquality(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value >= rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseEquality(Stock stock, Tokenizer tok)
        {
            var expression = ParseAddition(stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("=="))
                {
                    var rhs = ParseAddition(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value == rhs.Value);
                }
                else if (tok.Match("!="))
                {
                    var rhs = ParseAddition(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value != rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseAddition(Stock stock, Tokenizer tok)
        {
            var expression = ParseMultiplication(stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("+"))
                {
                    var rhs = ParseMultiplication(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value + rhs.Value);
                }
                else if (tok.Match("-"))
                {
                    var rhs = ParseMultiplication(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value - rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseMultiplication(Stock stock, Tokenizer tok)
        {
            var expression = ParseUnary(stock, tok);
            for (; ; )
            {
                var lhs = expression;
                if (tok.Match("*"))
                {
                    var rhs = ParseUnary(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value * rhs.Value);
                }
                else if (tok.Match("/"))
                {
                    var rhs = ParseUnary(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value / rhs.Value);
                }
                else if (tok.Match("%"))
                {
                    var rhs = ParseUnary(stock, tok);
                    expression = ValueSource.Create(() => lhs.Value % rhs.Value);
                }
                else
                    break;
            }
            return expression;
        }

        private ValueSource ParseUnary(Stock stock, Tokenizer tok)
        {
            ValueSource expression = null;

            if (tok.Match("!"))
            {
                var notExpression = ParseSubExpression(stock, tok);
                expression = ValueSource.Create(() => !notExpression.Value);
            }
            else
            {
                expression = ParseSubExpression(stock, tok);
            }

            return expression;
        }

        private ValueSource ParseSubExpression(Stock stock, Tokenizer tok)
        {
            ValueSource expression;

            if (tok.Match("("))
            {
                expression = ParseConditional(stock, tok);
                if (!tok.Match(")"))
                    tok.ThrowException("missing closing brace");
            }
            else
            {
                expression = ParseTerminal(stock, tok);
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

        private ValueSource ParseTerminal(Stock stock, Tokenizer tok)
        {
            var expression = ParseValueOrIdentifier(stock, tok);
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

        private ValueSource ParseValueOrIdentifier(Stock stock, Tokenizer tok)
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
                        expression = ParseFunction(stock, tok, identifier);
                    }
                    else
                    {
                        expression = stock.GetValueSource(identifier);
                        if (expression == null)
                        {
                            expression = stock.GetItem(identifier);
                            if (expression == null)
                                tok.ThrowException($"identifier='{identifier}' is not an expression nor an item");
                        }
                    }
                }
            }
            return expression;
        }

        private ValueSource ParseFunction(Stock stock, Tokenizer tok, string functionname)
        {
            var arguments = new List<ValueSource>();
            if (!tok.Match(")"))
            {
                arguments = ParseArguments(stock, tok);
                if (!tok.Match(")"))
                    tok.ThrowException("missing closing brace after arguments");
            }
            if (functionname == NowDateSource.Name)
                return new NowDateSource(arguments);
            if (functionname == "color")
                return new ColorSource(arguments);

            tok.ThrowException($"invalid function='{functionname}'");
            return null;
        }

        private List<ValueSource> ParseArguments(Stock stock, Tokenizer tok)
        {
            var arguments = new List<ValueSource>();
            do
            {
                var argument = ParseOr(stock, tok);
                arguments.Add(argument);
            }
            while (tok.Match(','));

            return arguments;
        }
    }
}