
#ifndef _APPSETTINGS_H_
#define _APPSETTINGS_H_

#include "settings.h"

class AppSettings : public Settings
{
public:
    inline static const char *KeyOnePanel = "onepanel";
    inline static const char *KeyLanguage = "language";
    inline static const char *KeyTimeMode = "timemode";
    inline static const char *KeyNTPServer = "ntpserver";
    inline static const char *KeyNTPInterval = "ntpinterval";
    inline static const char *KeyWifiSid = "wifisid";
    inline static const char *KeyWifiPwd = "wifipwd";
    inline static const char *KeyWeatherSource = "weathersource";
    inline static const char *KeyWeerliveKey = "weerlivekey";
    inline static const char *KeyWeerliveLocation = "weerliveloc";
    inline static const char *KeyOpenWeatherKey = "openweatherkey";
    inline static const char *KeyOpenWeatherLocation = "openweatherloc";
    inline static const char *KeyTZ = "tz";
    inline static const char *KeyTZCustom = "tzcustom";
    inline static const char *KeySmoothSecondHand  = "secondhand";
    inline static const char *KeyFlipDisplay  = "flipdisplay";
    inline static const char *KeyFlipKeys  = "flipkeys";

    AppSettings()
    {        
        defaultSettings();
        loadSettings();
    }

    void defaultSettings()
    {        
        add(KeyOnePanel, get(KeyOnePanel, false));   // panel size is maintained
        add(KeyLanguage, "en");
        add(KeyTimeMode, "1");
        add(KeyNTPServer, "pool.ntp.org");
        add(KeyNTPInterval, 60);
        add(KeyWifiSid, "__sid__");
        add(KeyWifiPwd, "__password__");
        add(KeyWeatherSource, "weerlive");
        add(KeyWeerliveKey, "demo");
        add(KeyWeerliveLocation, "Amsterdam");
        add(KeyOpenWeatherKey, "__app_id__");
        add(KeyOpenWeatherLocation, "Amsterdam");
        add(KeyTZ, "UTC");
        add(KeyTZCustom, "__tz__");
        add(KeySmoothSecondHand, true);
        add(KeyFlipDisplay, true);
        add(KeyFlipKeys, false);
    }

    bool OnePanel() const { return get(KeyOnePanel)->asbool(); }
    void OnePanel(bool value) { return get(KeyOnePanel)->set(value); }

    int TimeMode() const { return get(KeyTimeMode)->asint(); }
    void TimeMode(int value) { return get(KeyTimeMode)->set(value); }

    const std::string &NTPServer() const { return get(KeyNTPServer)->asstring(); }
    void NTPServer(const char *value) { return get(KeyNTPServer)->set(value); }

    int NTPInterval() const { return get(KeyNTPInterval)->asint(); }
    void NTPInterval(int value) { return get(KeyNTPInterval)->set(value); }

    const std::string &Language() const { return get(KeyLanguage)->asstring(); }
    void Language(const char *value) { return get(KeyLanguage)->set(value); }

    const std::string &WifiSid() const { return get(KeyWifiSid)->asstring(); }
    void WifiSid(const char *value) { return get(KeyWifiSid)->set(value); }

    const std::string &WifiPassword() const { return get(KeyWifiPwd)->asstring(); }
    void WifiPassword(const char *value) { return get(KeyWifiPwd)->set(value); }

    const std::string &WeatherSource() const { return get(KeyWeatherSource)->asstring(); }
    void WeatherSource(const char *value) { return get(KeyWeatherSource)->set(value); }

    const std::string &WeerliveKey() const { return get(KeyWeerliveKey)->asstring(); }
    void WeerliveKey(const char *value) { return get(KeyWeerliveKey)->set(value); }

    const std::string &WeerliveLocation() const { return get(KeyWeerliveLocation)->asstring(); }
    void WeerliveLocation(const char *value) { return get(KeyWeerliveLocation)->set(value); }

    const std::string &OpenWeatherKey() const { return get(KeyOpenWeatherKey)->asstring(); }
    void OpenWeatherKey(const char *value) { return get(KeyOpenWeatherKey)->set(value); }

    const std::string &OpenWeatherLocation() const { return get(KeyOpenWeatherLocation)->asstring(); }
    void OpenWeatherLocation(const char *value) { return get(KeyOpenWeatherLocation)->set(value); }

    const std::string &TZ() const { return get(KeyTZ)->asstring(); }
    void TZ(const char *tz) const { return get(KeyTZ)->set(tz); }

    const std::string &TZCustom() const { return get(KeyTZCustom)->asstring(); }
    void TZCustom(const char *tz) const { return get(KeyTZCustom)->set(tz); }

    int SmoothSecondHand() const { return get(KeySmoothSecondHand)->asbool(); }
    void SmoothSecondHand(bool smooth) const { return get(KeySmoothSecondHand)->set(smooth); }

    bool FlipDisplay() const { return get(KeyFlipDisplay)->asbool(); }
    void FlipDisplay(bool flip) const { return get(KeyFlipDisplay)->set(flip); }

    bool FlipKeys() const { return get(KeyFlipKeys)->asbool(); }
    void FlipKeys(bool flip) const { return get(KeyFlipKeys)->set(flip); }
};

#endif
