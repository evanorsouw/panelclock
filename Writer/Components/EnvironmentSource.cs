using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net.Http;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace WhiteMagic.PanelClock
{
    public class EnvironmentSource : ValueSource, IDisposable
    {
        public static string Name => "environment";
        private ILogger _logger;
        private string _apiKey;
        private double _lattitude;
        private double _longitude;
        private double _temperature;
        private double _windchill;
        private DateTime _sunriseValue;
        private DateTime _sunsetValue;
        private WeatherType _weatherType;
        private TimestampSource _sunset;
        private TimestampSource _sunrise;
        private NowSource _now;
        private IDisposable _timerObserver;

        public EnvironmentSource(ILogger logger) : base(Name)
        {
            _logger = logger;
            _sunrise = new TimestampSource("", _logger, () => _sunriseValue);
            _sunset = new TimestampSource("", _logger, () => _sunsetValue);
            _now = new NowSource(_logger);

            AddProperty(Create("weeronlineapikey", () => _apiKey, obj => _apiKey = obj));
            AddProperty(Create("lattitude", () => _lattitude, obj => _lattitude = obj));
            AddProperty(Create("longitude", () => _longitude, obj => _longitude = obj));
            AddProperty(Create("now", _now));
            AddProperty(Create("sunset", () => _sunset.Value));
            AddProperty(Create("sunrise", () => _sunrise.Value));
            AddProperty(Create("temperature", () => _temperature));
            AddProperty(Create("windchill", () => _windchill));
            AddProperty(Create("weather", () => _weatherType));

            _apiKey = "31a26656d0";
            _longitude = 51.732034245965046;
            _lattitude = 5.32637472036842;

            var next = Observable.Timer(TimeSpan.FromSeconds(3)).Merge(Observable.Interval(TimeSpan.FromSeconds(600)));
            _timerObserver = next.Subscribe(async _ => await UpdateEnvironment());
        }

        private async Task UpdateEnvironment()
        {
            var uri = $"https://weerlive.nl/api/json-data-10min.php?key={_apiKey}&locatie={_longitude},{_lattitude}";
            try
            {
                using var client = new HttpClient();
                var response = await client.GetAsync(uri);
                if (!response.IsSuccessStatusCode)
                {
                    _logger.LogInformation($"failed to retrieve weatherinfo from uri='{uri}', status='{response.StatusCode}', reason='{response.ReasonPhrase}'");
                    return;
                }
                var parts = ParseJson(await response.Content.ReadAsStringAsync());
                _logger.LogInformation($"retrieved {parts.Count} properties from uri='{uri}'");
            }
            catch (Exception ex)
            {
                _logger.LogError($"exception while retriving weatherinfo from uri='{uri}': {ex.Message}'");
            }
        }

        private List<string> ParseJson(string json)
        {
            var updatedProperties = new List<string>();
            var doc = JsonDocument.Parse(json);
            var root = doc.RootElement;

            foreach (JsonProperty o in root.EnumerateObject())
            {
                if (o.Name != "liveweer")
                    continue;

                var properties = o.Value.EnumerateArray().FirstOrDefault();
                if (properties.TryGetProperty("temp", out var temp))
                {
                    if (double.TryParse(temp.ToString(), out _temperature))
                    {
                        updatedProperties.Add("temperature");
                    }
                }
                if (properties.TryGetProperty("gtemp", out var gtemp))
                {
                    if (double.TryParse(gtemp.ToString(), out _windchill))
                    {
                        updatedProperties.Add("windchill");
                    }
                }
                if (properties.TryGetProperty("sup", out var sup))
                {
                    if (GetTimestamp(sup, out _sunriseValue))
                    {
                        updatedProperties.Add("sunrise");
                    }
                }
                if (properties.TryGetProperty("sunder", out var sunder))
                {
                    if (GetTimestamp(sunder, out _sunsetValue))
                    {
                        updatedProperties.Add("sunset");
                    }
                }
                if (properties.TryGetProperty("d0weer", out var d0weer))
                {
                    if (ParseEnumeration(d0weer.ToString(), out _weatherType))
                    {
                        updatedProperties.Add("weathertype");
                    }
                }
            }
            return updatedProperties;
        }

        private bool ParseEnumeration<T>(string text, out T enumValue)
        {
            enumValue = default(T);
            foreach (var en in typeof(T).GetEnumValues())
            {
                var type = en.GetType();
                var memInfo = type.GetMember(en.ToString());
                var attributes = memInfo[0].GetCustomAttributes(typeof(DescriptionAttribute), false);
                if (attributes.Any())
                {
                    var attr = attributes[0] as DescriptionAttribute;
                    if (attr.Description.ToLower() == text.ToLower())
                    {
                        enumValue = (T)en;
                        return true;
                    }
                }
            }
            return false;
        }

        private bool GetTimestamp(JsonElement json, out DateTime timestamp)
        {
            var hour = 0;
            var minute = 0;

            var s = json.GetString();
            var match = Regex.Match(s, @"^(\d\d):(\d\d)$");
            if (match.Success)
            {
                hour = int.Parse(match.Groups[1].ToString());
                minute = int.Parse(match.Groups[2].ToString());
            }

            var now = DateTime.Now;
            timestamp = new DateTime(now.Year, now.Month, now.Day, hour, minute, 0);

            return match.Success;
        }

        public void Dispose()
        {
            _timerObserver.Dispose();
        }
    }
}
