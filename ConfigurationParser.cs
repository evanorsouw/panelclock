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

        public Movie Parse()
        {
            var movie = new Movie();
            for(int iscene=0; ;++iscene )
            {
                var sceneCfg = _config.GetSection($"scenes:{iscene}");
                var scene = ParseScene(sceneCfg);
                if (scene == null)
                    break;
                movie.AddScene(scene);
            }
            return movie;
        }

        private Scene ParseScene(IConfigurationSection sceneCfg)
        {
            var idCfg = sceneCfg["id"];
            if (idCfg == null)
                return null;

            var id = int.Parse(idCfg);
            var scene = new Scene(id);
            for (int iitem = 0; ; ++iitem)
            {
                var itemCfg = sceneCfg.GetSection($"items:{iitem}");
                if (!ParseSceneItems(scene, itemCfg))
                    break;
            }
            return scene;
        }

        private bool ParseSceneItems(Scene scene, IConfigurationSection itemCfg)
        {
            var type = itemCfg["type"];
            if (type == null)
                return false;

            IDrawable item = null;
            if (type == "label")
            {
                item = new VarLabel();
            }
            else if (type == "analogclock-modern")
            {
                item = new AnalogClockModern(_logger);
            }
            else
            {
                _logger.LogWarning($"unrecognized scene item='{type}'");
            }
            if (item != null)
            {
                ParseProperties(itemCfg, item);
                scene.AddItem(item);
            }
            return true;
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
    }
}