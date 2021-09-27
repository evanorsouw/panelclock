using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System;
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
                item = new VarLabel(id, _logger);
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

                value = ValueSource.Create($"{item.Id}.{name}", value);
                stock.AddExpression(value);
                stock.AddAssignment(new Assignment(() => { property.Value = value.Value; }));
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
            if (value.GetType() == typeof(string) && value.ToString().StartsWith("="))
            {
                var tok = new Tokenizer(value.ToString().Substring(1));
                expression = ParseSubExpression(stock, tok);
            }
            else 
            {
                expression = ValueSource.Create(() => value);
            }
            return expression;
        }

        private ValueSource ParseSubExpression(Stock stock, Tokenizer tok)
        {
            ValueSource expression = null;

            if (tok.Match("("))
            {
                expression = ParseSubExpression(stock, tok);
                if (!tok.Match(")"))
                    throw new Exception("missing closing brace");
            }
            else
            {
                expression = ParseTerminal(stock, tok);
            }

            if (tok.Match("."))
            {
                expression = ParseSourceProperty(stock, tok, expression);
            }

            return expression;
        }

        private ValueSource ParseSourceProperty(Stock stock, Tokenizer tok, ValueSource expression)
        {
            if (!tok.Identifier(out var identifier))
                throw new Exception("identifier expected after '.'");

            var subExpression = expression.GetProperty(identifier);
            if (subExpression == null)
                throw new Exception($"object='{expression.Id}' does not have a property='{identifier}'");

            return subExpression;
        }

        private ValueSource ParseTerminal(Stock stock, Tokenizer tok)
        {
            ValueSource expression = null;

            if (tok.Identifier(out var identifier))
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
                            throw new Exception($"identifier='{identifier}' is not an expression nor an item");
                    }
                }
            } 
            else if (tok.Number(out var number))
            {
                expression = ValueSource.Create(() => number);
            }
            else if (tok.String(out var literal))
            {
                expression = ValueSource.Create(() => literal);
            }
            else if (tok.Match("true"))
            {
                expression = ValueSource.Create(() => true);
            }
            else if (tok.Match("false"))
            {
                expression = ValueSource.Create(() => false);
            }
            return expression;
        }

        private ValueSource ParseFunction(Stock stock, Tokenizer tok, string functionname)
        {
            if (!tok.Match(")"))
                throw new Exception("arguments not yet supported");

            if (functionname == "now")
                return new NowDateSource();            

            throw new Exception($"invalid function='{functionname}'");
        }
    }
}