
#include <numbers>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "environment_weerlive.h"
#include "httpclient.h"

static struct {
    const char *imageName;
    weathertype type;
} weathertypes[] = {
    { "bliksem", weathertype::lightning },
    { "buien", weathertype::showers },
    { "bewolkt", weathertype::clouded },
    { "hagel", weathertype::hail },
    { "halfbewolkt", weathertype::partlycloudy },
    { "halfbewolkt_regen", weathertype::cloudyrain },
    { "helderenacht", weathertype::clearnight },
    { "lichtbewolkt", weathertype::cloudy },
    { "mist", weathertype::fog },
    { "wolkennacht", weathertype::cloudednight },
    { "nachtmist", weathertype::nightfog },
    { "regen", weathertype::rain },
    { "sneeuw", weathertype::snow },
    { "zonnig", weathertype::sunny },
    { "zwaarbewolkt", weathertype::heavyclouds }
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

int EnvironmentWeerlive::update()
{
    int nextAttempt = 10 * 60 * 1000;   // update every 10 minutes, that is the rate at which weerlive effectively updates.
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
            nextAttempt = 1 * 60 * 1000;
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
                nextAttempt = 2 * 60 * 1000;
            }
            else
            {
                _values = _parsedValues;        
                printf("weather: ");
                if (valid())
                {
                    _lastupdate = _system->now();
                    if (location().isValid())
                        printf("location=%s ", location().value().c_str());
                    if (temperature().isValid())
                        printf("temp=%.1f ", temperature().value());
                    if (windchill().isValid())
                        printf("gtemp=%.1f ", windchill().value());
                    if (windspeed().isValid())
                        printf("windspeed=%.1f ", windspeed().value());
                    if (windangle().isValid())
                        printf("windangle=%.2f ", windangle().value());
                    if (weather().isValid())
                        printf("weather=%d ", (int)weather().value());
                    if (airpressure().isValid())
                        printf("airpressure=%.1f ", airpressure().value());
                    if (sunrise().isValid())
                        printf("up:%02d:%02d ", sunrise().value().tm_hour, sunrise().value().tm_min);
                    if (sunset().isValid())
                        printf("set:%02d:%02d ", sunset().value().tm_hour, sunset().value().tm_min);
                    printf("\n");
                }
                else
                {
                    printf("error=%s\n", invalidReason().c_str());
                }
            }
        }
        _values = _parsedValues;        
        _updating = false;
    }
    return nextAttempt;
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

weathertype EnvironmentWeerlive::parseWeather(const char *image)
{
    for (int i=0; i<(sizeof(weathertypes) / sizeof(weathertypes[0])); ++i)
    {
        if (!strcmp(image, weathertypes[i].imageName))
            return weathertypes[i].type;
    }
    return weathertype::unknown;
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
