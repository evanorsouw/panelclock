
#include <cmath>
#include <numbers>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cJSON.h>

#include "color.h"
#include "environment_openweather.h"
#include "httpclient.h"

static struct {
    std::vector<int> ids;
    std::vector<WeatherLayer> layers;
} weathertypes[] = {
    // for interpretations of id see
    // https://openweathermap.org/weather-conditions#Weather-Condition-Codes
    { std::vector<int> { 200,201,230,231,232 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightning, .oy=2, .phase1=22000, .phase1offset=3000, .phase2=19000 }
      }
    },
    { std::vector<int> { 202 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithHeavyRain, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightning, .oy=2, .phase1=22000, .phase1offset=3000, .phase2=19000 }
      }
    },
    { std::vector<int> { 210,211,212,221 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightning, .oy=2, .phase1=22000, .phase1offset=3000, .phase2=19000 }
      }
    },
    { std::vector<int> { 300,301,310,311,313,321 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .oy=-2 }
      }
    },
    { std::vector<int> { 302,312,314 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithHeavyRain, .oy=-2 }
      }
    },
    { std::vector<int> { 500,501 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithoutRays, },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .oy=-3 }
      }
    },
    { std::vector<int> { -500,-501 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Moon, },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .oy=-3 }
      }
    },
    { std::vector<int> { 502,503,504 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithoutRays, .ox=-3, .oy=-5 },
        WeatherLayer { .icon = WeatherIcon::CloudWithHeavyRain, .oy=-3 }
      }
    },
    { std::vector<int> { -502,-503,-504 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Moon },
        WeatherLayer { .icon = WeatherIcon::CloudWithHeavyRain, .oy=-3 }
      }
    },
    { std::vector<int> { 511,-511 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::CloudWithHail, .color1=Color(0x203250), .color2=Color(0xDEF0F9), .oy=-4 },
      }
    },
    { std::vector<int> { 520,521,-520,-521 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::CloudWithLightRain, .oy=-2 },
      }
    },
    { std::vector<int> { 522,531,-522,-531 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::CloudWithHeavyRain, .oy=-2 },
      }
    },
    { std::vector<int> { 
        600,601,602,611,612,613,615,616,620,621,622,
        -600,-601,-602,-611,-612,-613,-615,-616,-620,-621,-622
      },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SnowFlakes }
      }
    },
    { std::vector<int> { 
        701,711,721,731,741,751,761,762,771,781 
        -701,-711,-721,-731,-741,-751,-761,-762,-771,-781 
      },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Fog }
      }
    },
    { std::vector<int> { 800 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithRays }
      }
    },
    { std::vector<int> { -800 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Moon }
      }
    },
    { std::vector<int> { 801 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::SunWithRays, .ox=-3, .oy=-3 },
        WeatherLayer { .icon = WeatherIcon::Cloud }
      }
    },
    { std::vector<int> { -801 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Moon, .ox=3, .oy=-3 },
        WeatherLayer { .icon = WeatherIcon::Cloud }
      }
    },
    { std::vector<int> { 802, -802 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud }
      }
    },
    { std::vector<int> { 803, -803, 804, -804 },
      std::vector<WeatherLayer>{ 
        WeatherLayer { .icon = WeatherIcon::Cloud, .color1 = Color(0,0,0), .oy=-4, .fontsize='l' },
        WeatherLayer { .icon = WeatherIcon::Cloud, .oy=2, .phase1=22000, .phase1offset=3000, .phase2=19000 }
      }
    }
};

EnvironmentOpenWeather::EnvironmentOpenWeather(System *system, Settings &settings, Event *updateEvent)
    : EnvironmentBase(name(), updateEvent)
{
    _system = system;
    _accesskey = settings.get(AppSettings::KeyOpenWeatherKey);
    _location = settings.get(AppSettings::KeyOpenWeatherLocation);
    _parsedValues.clear();
    _updating = false;
    _lastupdate = system->now();
    _hTimer = xTimerCreate("openweather", _updateIntervalTicks, pdTRUE, this, 
        [](TimerHandle_t timer){ 
            ((EnvironmentOpenWeather*) pvTimerGetTimerID(timer))->triggerUpdate();
        });
}

void EnvironmentOpenWeather::update()
{
    if (_updating)
    {
        printf("update openweather ignore, already in progress\n");
    }
    else
    {
        _updating = true;
        printf("update openweather\n");

        _parsedValues.clear();
        if (!_system->wifiConnected())
        {
            printf("weather: no internet\n");
            _parsedValues.invalidReason = "no internet";
        }
        else
        {
            HTTPClient client;
            char url[256];
            auto loc = _location->asstring().c_str();
            float lat, lon;
            int n;
            if (sscanf(loc, "%f,%f%n", &lat, &lon, &n) == 2 && n==strlen(loc))
            {
                sprintf(url, "https://api.openweathermap.org/data/2.5/weather?lat=%.3f&lon=%.3f&units=metric&APPID=%s",
                    lat, lon, _accesskey->asstring().c_str());
            }
            else
            {
                sprintf(url, "https://api.openweathermap.org/data/2.5/weather?q=%s&units=metric&APPID=%s",
                    _location->asstring().c_str(), _accesskey->asstring().c_str());
            }

            char buf[4096];
            memset(buf, 0, sizeof(buf));
            auto result = client.get(url, (uint8_t*)buf, sizeof(buf));
            if (result != 200)
            {
                snprintf(buf, sizeof(buf), "server response %d", result);
                printf("weather: %s\n", buf);
                _parsedValues.invalidReason = buf;
            }
            else if (!parseJson(buf))
            {
                printf("weather: %s\n", _parsedValues.invalidReason.c_str());
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
        xTimerReset(_hTimer, _updateIntervalTicks);
    }
}

bool EnvironmentOpenWeather::parseJson(const char *buf)
{
    _parsedValues.clear();

    cJSON *json = cJSON_Parse(buf);
    if (!json)
    {
        _parsedValues.invalidReason = "invalid json received";
        return false;
    }

    auto message = cJSON_GetObjectItem(json, "message");
    if (message)
    {
        _parsedValues.invalidReason = message->string;
    }
    else
    {
        auto name = cJSON_GetObjectItem(json, "name");
        if (cJSON_IsString(name))
        {
            _parsedValues.location.set(name->valuestring);
        }
        auto main = cJSON_GetObjectItem(json, "main");
        if (cJSON_IsObject(main))
        {
            auto value = cJSON_GetObjectItem(main, "temp");
            if (cJSON_IsNumber(value)) _parsedValues.temperature.set((float)value->valuedouble);
            value = cJSON_GetObjectItem(main, "feels_like");
            if (cJSON_IsNumber(value)) _parsedValues.windchill.set((float)value->valuedouble);
            value = cJSON_GetObjectItem(main, "pressure");
            if (cJSON_IsNumber(value)) _parsedValues.airpressure.set((float)value->valuedouble);
        }
        auto wind = cJSON_GetObjectItem(json, "wind");
        if (cJSON_IsObject(wind))
        {
            auto value = cJSON_GetObjectItem(wind, "speed");
            if (cJSON_IsNumber(value)) _parsedValues.windspeed.set((float)value->valuedouble);
            value = cJSON_GetObjectItem(wind, "deg");
            if (cJSON_IsNumber(value)) _parsedValues.windangle.set(parseWindAngle((float)value->valuedouble));
        }
        auto weather = cJSON_GetObjectItem(json, "weather");
        if (cJSON_IsArray(weather))
        {
            auto weather0 = cJSON_GetArrayItem(weather, 0);
            if (cJSON_IsObject(weather0))
            {
                int weatherid = 0;
                auto id = cJSON_GetObjectItem(weather0, "id");
                if (cJSON_IsNumber(id)) weatherid = (int)id->valuedouble;
                auto icon = cJSON_GetObjectItem(weather0, "icon");
                if (cJSON_IsString(icon)) 
                {
                    if (icon->valuestring[strlen(icon->valuestring)-1] == 'n')
                    {
                        // negative id's indicate night-version (where applicable)
                        weatherid = -weatherid;
                        
                    }
                }
                _parsedValues.weather.set(parseWeather(weatherid));
            }
        }
        auto sys = cJSON_GetObjectItem(json, "sys");
        if (cJSON_IsObject(sys))
        {
            auto value = cJSON_GetObjectItem(sys, "sunrise");
            if (cJSON_IsNumber(value)) _parsedValues.sunrise.set(parseTime((time_t)value->valuedouble));
            value = cJSON_GetObjectItem(sys, "sunset");
            if (cJSON_IsNumber(value)) _parsedValues.sunset.set(parseTime((time_t)value->valuedouble));
        }
    }
    cJSON_Delete(json);
    return _parsedValues.invalidReason == "";
}

tm EnvironmentOpenWeather::parseTime(time_t when)
{
    struct tm tm;
    return *localtime_r(&when, &tm);
}

float EnvironmentOpenWeather::parseWindAngle(float angle)
{
    angle = std::fmod(-(angle + 90.0f) + 720.0f, 360.0f);
    return angle / 180 * std::numbers::pi;
}

std::vector<WeatherLayer> EnvironmentOpenWeather::parseWeather(int id)
{
    printf("finding openweather graphics for id='%d' (%s) ", id, id < 0 ? "night" : "day");
    for (int i=0; i<(sizeof(weathertypes) / sizeof(weathertypes[0])); ++i)
    {
        auto &ids = weathertypes[i].ids;
        auto match =  std::find(ids.begin(), ids.end(), id);
        if (match != ids.end())
        {
            printf(" - match\n");
            return weathertypes[i].layers;
        }
    }
    printf("no match\n");
    return std::vector<WeatherLayer>();
}