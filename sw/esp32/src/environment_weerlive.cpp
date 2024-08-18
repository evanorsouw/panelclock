
#include "environment_weerlive.h"
#include "httpclient.h"

void EnvironmentWeerlive::update()
{
    HTTPClient client;
    auto url = std::string("https://weerlive.nl/api/weerlive_api_v2.php?key=") + _accesskey + "&locatie=" + _location;

    _state = ParseState::WaitArray;
    JsonParser parser([this](const JsonEntry &json) { return handleJson(json); });
    auto ok = client.get(url.c_str(), [&](void *data, int len) { parser.parse((char *)data, len); });

    printf("temp=%f, gtemp=%f, wndspeed=%f, windangle=%f, weather=%d, airpress=%f, rise=%04d-%02d-%02d %02d:%02d:%02d, set=%04d-%02d-%02d %02d:%02d:%02d\n",
        temperature(),
        windchill(),
        windspeed(),
        windangle(),
        (int)weathertype(),
        airpressure(),
        sunrise().tm_mday, sunrise().tm_mon, sunrise().tm_year, sunrise().tm_hour, sunrise().tm_min, sunrise().tm_sec, 
        sunset().tm_mday, sunset().tm_mon, sunset().tm_year, sunset().tm_hour, sunset().tm_min, sunset().tm_sec);
}

bool EnvironmentWeerlive::handleJson(const JsonEntry &json)
{
    switch (_state)
    {
    case ParseState::WaitArray:
        if (json.item == JsonItem::Array && !strcmp(json.name, "liveweer"))
        {
            printf("got start array\n");
            _state = ParseState::WaitObject1;
        }
        break;
    case ParseState::WaitObject1:
        if (json.item == JsonItem::Object)
        {
            printf("got object 1\n");
            _state = ParseState::Reading;
        }
        break;
    case ParseState::Reading:
        printf("object %s\n", json.name);
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
                _temperature = (float)json.number;
            else if (!strcmp(json.name, "gtemp"))
                _windchill = (float)json.number;
            else if (!strcmp(json.name, "windrgr"))
                _windangle = (float)json.number;
            else if (!strcmp(json.name, "windms"))
                _windspeed = (float)json.number;
            else if (!strcmp(json.name, "luchtd"))
                _airpressure = (float)json.number;
            break;
        case JsonItem::String:
            if (!strcmp(json.name, "sup"))
                _sunrise = parseTime(json.string);
            else if (!strcmp(json.name, "sunder"))
                _sunset = parseTime(json.string);
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
    printf("%s => n=%d, offset=%d\n", s, n, offset);
    if (n == 2 && offset == strlen(s))
        return when;

    // failed to parse 
    return tm { 0 };
}

