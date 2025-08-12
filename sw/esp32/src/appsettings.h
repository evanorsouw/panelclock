
#ifndef _APPSETTINGS_H_
#define _APPSETTINGS_H_

#include "settings.h"

class AppSettings : public Settings
{
public:
    // Note: keys may be max 15 chars long or they won't be written
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
    inline static const char *KeySmoothSecondHand = "secondhand";
    inline static const char *KeyPanelSides = "panelsides";
    inline static const char *KeyPanelMode = "panelmode";
    inline static const char *KeyPanel1Flipped = "panel1flipped";
    inline static const char *KeyPanel2Flipped = "panel2flipped";
    inline static const char *KeyFlipKeys = "flipkeys";
    inline static const char *KeySoftwareUpdateInterval = "swupdinterval";

    AppSettings(const char *nvsname) 
        : Settings(nvsname)
    {        
        defaultSettings();
        loadSettings();
    }

    void defaultSettings()
    {        
        // these are maintained
        add(KeyPanelMode, get(KeyPanelMode, 2));
        add(KeyPanelSides, get(KeyPanelSides, 0));
        add(KeyPanel1Flipped, get(KeyPanel1Flipped, 0));
        add(KeyPanel2Flipped, get(KeyPanel2Flipped, 0));
        add(KeyFlipKeys, get(KeyFlipKeys, false));

        // the rest is default
        add(KeyLanguage, "en");
        add(KeyTimeMode, "1");
        add(KeyNTPServer, "pool.ntp.org");
        add(KeyNTPInterval, 60);    // minutes
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
        add(KeySoftwareUpdateInterval, 7*24*60);  // minutes
    }

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

    bool PanelSides() const { return get(KeyPanelSides)->asbool(); }
    void PanelSides(bool sides) const { return get(KeyPanelSides)->set(sides); }

    // Mode, 0=1-panel, 1=2-panel-landscape, 2=2-panel-portrait
    int PanelMode() const { return get(KeyPanelMode)->asint(); }
    void PanelMode(int mode) const { return get(KeyPanelMode)->set(std::max(0, std::min(2, mode))); }

    bool Panel1Flipped() const { return get(KeyPanel1Flipped)->asbool(); }
    void Panel1Flipped(bool flipped) const { return get(KeyPanel1Flipped)->set(flipped); }

    bool Panel2Flipped() const { return get(KeyPanel2Flipped)->asbool(); }
    void Panel2Flipped(bool flipped) const { return get(KeyPanel2Flipped)->set(flipped); }

    bool FlipKeys() const { return get(KeyFlipKeys)->asbool(); }
    void FlipKeys(bool flip) const { return get(KeyFlipKeys)->set(flip); }

    int SoftwareUpdateInterval() const { return get(KeySoftwareUpdateInterval)->asbool(); }
    void SoftwareUpdateInterval(int interval) const { return get(KeySoftwareUpdateInterval)->set(interval); }
};

#endif
