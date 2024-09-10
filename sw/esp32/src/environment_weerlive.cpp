
#include <numbers>

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
    { "nachtbewolkt", weathertype::cloudednight },
    { "nachtmist", weathertype::nightfog },
    { "regen", weathertype::rain },
    { "sneeuw", weathertype::snow },
    { "zonnig", weathertype::sunny },
    { "zwaarbewolkt", weathertype::heavyclouds }
};

void EnvironmentWeerlive::updateTask()
{
    _system->waitForInternet();
    HTTPClient client;
    auto url = std::string("https://weerlive.nl/api/weerlive_api_v2.php?key=") + _accesskey->asstring() + "&locatie=" + _location->asstring();

    _state = ParseState::WaitArray;
    JsonParser parser([this](const JsonEntry &json) { return handleJson(json); });

    clearValues();
    auto status = client.get(url.c_str(), [&](void *data, int len) { parser.parse((char *)data, len); });
    _valid = status == 200;
    
    auto delayMs = 30 * 1000;
    if (_valid)
    {
        delayMs = 10 * 60 * 1000;
        if (temperature().isValid())
            printf("temp=%f ", temperature().value());
        if (windchill().isValid())
            printf("gtemp=%f ", windchill().value());
        if (windspeed().isValid())
            printf("windspeed=%f ", windspeed().value());
        if (windangle().isValid())
            printf("windangle=%f ", windangle().value());
        if (weather().isValid())
            printf("weather=%d ", (int)weather().value());
        if (airpressure().isValid())
            printf("airpressure=%f ", airpressure().value());
        if (sunrise().isValid())
            printf("%04d-%02d-%02d %02d:%02d:%02d ", 
                sunrise().value().tm_year, sunrise().value().tm_mon, sunrise().value().tm_mday, sunrise().value().tm_hour, sunrise().value().tm_min, sunrise().value().tm_sec);
        if (sunset().isValid())
            printf("%04d-%02d-%02d %02d:%02d:%02d ", 
                sunset().value().tm_year, sunset().value().tm_mon, sunset().value().tm_mday, sunset().value().tm_hour, sunset().value().tm_min, sunset().value().tm_sec);
        printf("\n");
    }
    vTaskDelay(delayMs / portTICK_PERIOD_MS);
}

bool EnvironmentWeerlive::handleJson(const JsonEntry &json)
{
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
        switch (json.item)
        {
        case JsonItem::End:
            _state = ParseState::Completed;
            _valid = true;
            break;
        case JsonItem::Array:
        case JsonItem::Object:
        case JsonItem::Close:
            _state = ParseState::Completed;
            break;  // structure not as expected
        case JsonItem::Number:
            if (!strcmp(json.name, "temp"))
                _temperature.set((float)json.number);
            else if (!strcmp(json.name, "gtemp"))
                _windchill.set((float)json.number);
            else if (!strcmp(json.name, "windrgr"))
                _windangle.set((float)json.number / 180 * std::numbers::pi);
            else if (!strcmp(json.name, "windms"))
                _windspeed.set((float)json.number);
            else if (!strcmp(json.name, "luchtd"))
                _airpressure.set((float)json.number);
            break;
        case JsonItem::String:
            if (!strcmp(json.name, "image"))
                _weather.set(parseWeather(json.string));
            else if (!strcmp(json.name, "sup"))
                _sunrise.set(parseTime(json.string));
            else if (!strcmp(json.name, "sunder"))
                _sunset.set(parseTime(json.string));
            break;        
        default:
            // ignored
            break;
        }
        break;
    case ParseState::Completed:
        break;
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

void EnvironmentWeerlive::clearValues()
{
    _sunset.clear();
    _sunrise.clear();
    _temperature.clear();
    _windchill.clear();
    _weather.clear();
    _windangle.clear();
    _windspeed.clear();
    _airpressure.clear();
}