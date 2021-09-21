using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System;

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
            for (int iitem = 0; ; ++iitem)
            {
                var itemCfg = _config.GetSection($"items:{iitem}");
                var item = ParseItem(itemCfg);
                if (item == null)
                    break;
                stock.AddItem(item);
            }
            for (int iscene=0; ;++iscene )
            {
                var sceneCfg = _config.GetSection($"scenes:{iscene}");
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

        private IDrawable ParseItem(IConfigurationSection itemCfg)
        {
            var id = itemCfg["id"];
            if (id == null)
                return null;

            var type = itemCfg["type"];
            if (type == null)
                return null;

            IDrawable item = null;
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
            if (item != null)
            {
                ParseProperties(itemCfg, item);
            }
            return item;
        }

        private void ParseProperties(IConfigurationSection itemCfg, IDrawable item)
        {
            foreach(var rprop in item.GetType().GetProperties())
            {
                var value = itemCfg[rprop.Name.ToLower()];
                if (value != null)
                {
                    _logger.LogInformation($"property='{rprop.Name.ToLower()}' value='{value}'");
                    if (rprop.PropertyType == typeof(string))
                    {
                        rprop.SetValue(item, value);
                    }
                    else if (rprop.PropertyType == typeof(bool))
                    {
                        if (value.GetType() == typeof(bool))
                        {
                            rprop.SetValue(item, value);
                        }
                        else if (bool.TryParse(value, out var parsed))
                        {
                            rprop.SetValue(item, parsed);
                        }
                    }
                    else if (rprop.PropertyType == typeof(int))
                    {
                        if (value.GetType() == typeof(int))
                        {
                            rprop.SetValue(item, value);
                        }
                        else if (int.TryParse(value, out var parsed))
                        {
                            rprop.SetValue(item, parsed);
                        }
                    }
                    else if (rprop.PropertyType == typeof(float))
                    {
                        if (value.GetType() == typeof(float))
                        {
                            rprop.SetValue(item, value);
                        }
                        else if (float.TryParse(value, out var parsed))
                        {
                            rprop.SetValue(item, parsed);
                        }
                    }
                    else if (rprop.PropertyType == typeof(double))
                    {
                        if (value.GetType() == typeof(double))
                        {
                            rprop.SetValue(item, value);
                        }
                        else if (double.TryParse(value, out var parsed))
                        {
                            rprop.SetValue(item, parsed);
                        }
                    }
                    else if (rprop.PropertyType.IsEnum)
                    {
                        var parsed = Enum.Parse(rprop.PropertyType, value, true);
                        rprop.SetValue(item, parsed);
                    }
                }
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
    }
}