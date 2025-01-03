
#include <numbers>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "color.h"
#include "environment_weerlive.h"
#include "httpclient.h"

static struct {
    const char *imageName;
    std::vector<WeatherLayer> layers;
} weathertypes[] = {
    { "bliksem",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithLightning, .oy=-3 }
      }
    },
    { "wolkennacht",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Moon },
        WeatherLayer { .icon = WeatherIcon::Cloud }
      }
    },
    { "buien",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithHeavyRain, .oy=-4 }
      }
    },
    { "bewolkt",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::Cloud, .oy=2, .phase1=22000, .phase1offset=3000, .phase2=19000 }
      }
    },
    { "hagel",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithHail, .color1=Color(0x203250), .color2=Color(0xDEF0F9), .oy=-4 },
      }
    },
    { "halfbewolkt",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithRays, .ox=-3, .oy=-3 },
        WeatherLayer { .icon = WeatherIcon::Cloud },
      }
    },
    { "halfbewolkt_regen",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithRays, .ox=-3, .oy=-3 },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .oy=-2 },
      }
    },
    { "helderenacht",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Stars },
        WeatherLayer { .icon = WeatherIcon::Moon, .fontsize='l' }
      }
    },
    { "lichtbewolkt",

      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithRays, .ox=-3, .oy=-3 },
        WeatherLayer { .icon = WeatherIcon::Cloud }
      }
    },
    { "mist",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color2=Color(128,128,128), .oy=-4 },
        WeatherLayer { .icon = WeatherIcon::Fog }
      }
    },
    { "nachtmist",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Moon, .color1=Color(128,128,128), .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::Fog }
      }
    },
    { "regen",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .oy=-3 },
      }
    },
    { "sneeuw",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SnowFlakes },
      }
    },
    { "zonnig",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithRays },
      }
    },
    { "zwaarbewolkt",
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color1 = Color(1,1,1) },
        WeatherLayer { .icon = WeatherIcon::Cloud },
      }
    }
};

static struct {
    const char *r;
    float angle;
} windrs[] = {
    { "O", 180 },
    { "ONO", 157.5 },
    { "NO", 135 },
    { "NNO", 112.55 },
    { "N", 90 },
    { "NNW", 67.5 },
    { "NW", 45 },
    { "WNW", 22.5 },
    { "W", 0 },
    { "WZW", 337.5 },
    { "ZW", 315 },
    { "ZZW", 292.5 },
    { "Z", 270 },
    { "ZZO", 247.5 },
    { "ZO", 225 },
    { "OZO", 202.5 }
};

void EnvironmentWeerlive::update()
{
    if (_updating)
    {
        printf("update weerlive ignore, already in progress\n");
    }
    else
    {
        _updating = true;
        printf("update weerlive\n");

        _parsedValues.clear();
        if (!_system->wifiConnected())
        {
            printf("weather: no internet\n");
            _parsedValues.invalidReason = "no internet";
        }
        else
        {
            HTTPClient client;
            auto url = std::string("https://weerlive.nl/api/weerlive_api_v2.php?key=") + _accesskey->asstring() + "&locatie=" + _location->asstring();
            _state = ParseState::WaitArray;
            JsonParser parser([this](const JsonEntry &json) { return handleJson(json); });

            auto result = client.get(url.c_str(), [&](uint8_t *data, int len) { parser.parse((char *)data, len); });
            if (result != 200)
            {
                char buf[40];
                snprintf(buf, sizeof(buf), "server response %d", result);
                printf("weather: %s\n", buf);
                _parsedValues.invalidReason = buf;
            }
            else
            {
                _values = _parsedValues;        
                _values.print();
                if (valid())
                {
                    _lastupdate = _system->now();
                }
            }
        }
        _values = _parsedValues;        
        _updating = false;
    }
}

bool EnvironmentWeerlive::handleJson(const JsonEntry &json)
{
    if (json.item == JsonItem::Error)
    {
        if (_state != ParseState::Failed)
        {
            printf("parsing of weerlive request failed, error='%s'\n", json.string);
            _state = ParseState::Failed;
        }
        return true;
    }

    switch (_state)
    {
    case ParseState::WaitArray:
        if (json.item == JsonItem::Array && !strcmp(json.name, "liveweer"))
        {
           _state = ParseState::WaitObject1;
        }
        break;
    case ParseState::WaitObject1:
        if (json.item == JsonItem::Object)
        {
            _state = ParseState::Reading;
        }
        break;
    case ParseState::Reading:
        //printf("got element '%s'(%d)='%d','%f','%s'\n", json.name, (int)json.item, json.boolean, json.number, json.string);
        switch (json.item)
        {
        case JsonItem::Close:
            _state = ParseState::Completed;
            break;
        case JsonItem::End:
        case JsonItem::Array:
        case JsonItem::Object:
            printf("got unexpected token=%s(%d) parsing failed\n", json.name, (int)json.item);
            _state = ParseState::Failed;
            break;  // structure not as expected
        case JsonItem::Number:
            if (!strcmp(json.name, "temp"))
                _parsedValues.temperature.set((float)json.number);
            else if (!strcmp(json.name, "gtemp"))
                _parsedValues.windchill.set((float)json.number);
            else if (!strcmp(json.name, "windms"))
                _parsedValues.windspeed.set((float)json.number);
            else if (!strcmp(json.name, "luchtd"))
                _parsedValues.airpressure.set((float)json.number);
            break;
        case JsonItem::String:
            if (!strcmp(json.name, "fout"))
                _parsedValues.invalidReason = json.string;
            else if (!strcmp(json.name, "plaats"))
                _parsedValues.location.set(json.string);
            else if (!strcmp(json.name, "image"))
                _parsedValues.weather.set(parseWeather(json.string));
            else if (!strcmp(json.name, "sup"))
                _parsedValues.sunrise.set(parseTime(json.string));
            else if (!strcmp(json.name, "sunder"))
                _parsedValues.sunset.set(parseTime(json.string));
            else if (!strcmp(json.name, "windr"))
                _parsedValues.windangle.set(parseWindr(json.string));
            break;        
        default:
            // ignored
            break;
        }
        break;
    case ParseState::Completed:
        break;
    case ParseState::Failed:
        return true;
    }
    return false;
}

tm EnvironmentWeerlive::parseTime(const char *s)
{
    tm when = { 0 };
    int offset;
    auto n = sscanf(s, "%d:%d%n", &when.tm_hour, &when.tm_min, &offset);
    if (n == 2 && offset == strlen(s))
        return when;

    // failed to parse 
    return tm { 0 };
}

std::vector<WeatherLayer> EnvironmentWeerlive::parseWeather(const char *image)
{
    for (int i=0; i<(sizeof(weathertypes) / sizeof(weathertypes[0])); ++i)
    {
        if (!strcmp(image, weathertypes[i].imageName))
            return weathertypes[i].layers;
    }
    return std::vector<WeatherLayer>();
}

float EnvironmentWeerlive::parseWindr(const char *r)
{
    for (int i=0; i<(sizeof(weathertypes) / sizeof(weathertypes[0])); ++i)
    {
        if (!strcmp(r, windrs[i].r))
            return windrs[i].angle / 180 * std::numbers::pi;
    }
    return 0.0f;
}
