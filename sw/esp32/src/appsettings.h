
#ifndef _APPSETTINGS_H_
#define _APPSETTINGS_H_

#include "settings.h"

class AppSettings : public Settings
{
public:
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
    inline static const char *KeyPanelType  = "setuptype";

    inline static const int PanelTypeCountOffset = 0;    
    inline static const int PanelTypeCountMsk = (0x03 << PanelTypeCountOffset);

    inline static const int PanelTypeOrientationOffset = 2;
    inline static const int PanelTypeOrientationMsk = (0x03 << PanelTypeOrientationOffset);

    inline static const int PanelTypeSideOffset = 4;
    inline static const int PanelTypeSideMask = (0x01 << PanelTypeSideOffset);

    inline static const int PanelTypeKeysOrientationOffset = 5;
    inline static const int PanelTypeKeysOrientationMsk = (0x01 << PanelTypeOrientationOffset);

    AppSettings()
    {        
        defaultSettings();
        loadSettings();
    }

    void defaultSettings()
    {        
        add(KeyPanelType, get(KeyPanelType, 0));   // panbel type is maintained
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
    }

    int PanelType() const { return get(KeyPanelType)->asint(); }
    void PanelType(int type) const { return get(KeyPanelType)->set(type); }

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

    bool OnePanel() const { return PanelCount() == 1; }
    /// @return 0=landscape usb down, 2=landscape usb up, 1+3=vertical (not supported yet)

    int PanelOrientation() const { return (PanelType() & PanelTypeOrientationMsk) >> PanelTypeOrientationOffset; }
    void PanelOrientation(int orientation) 
    {
        auto type = PanelType() & (~PanelTypeOrientationMsk);
        type |= (orientation << PanelTypeOrientationOffset) & PanelTypeOrientationMsk;
        PanelType(type);
    }
    int PanelSides() const { return (PanelType() & PanelTypeSideMask) >> PanelTypeSideOffset; }
    void PanelSides(int sides)
    {
        auto type = PanelType() & (~PanelTypeSideMask);
        type |= (sides << PanelTypeSideOffset) & PanelTypeSideMask;
        PanelType(type);
    }
    int PanelCount() const { return 1 + ((PanelType() & PanelTypeCountMsk) >> PanelTypeCountOffset); }
    void PanelCount(int count)
    {
        auto type = PanelType() & (~PanelTypeCountMsk);
        type |= ((count-1) << PanelTypeCountOffset) & PanelTypeCountMsk;
        PanelType(type);
    }
    bool FlipKeys() const { return (PanelType() & PanelTypeKeysOrientationMsk) != 0; }
    void FlipKeys(bool flip) 
    {
        auto type = PanelType() & (~PanelTypeKeysOrientationMsk);
        type |= flip ? PanelTypeKeysOrientationMsk : 0;
        PanelType(type);
    }
};

#endif
