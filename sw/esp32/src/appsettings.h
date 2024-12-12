
#ifndef _APPSETTINGS_H_
#define _APPSETTINGS_H_

#include "settings.h"

class AppSettings : public Settings
{
public:
    inline static const char *KeyLanguage = "language";
    inline static const char *KeyWifiSid = "wifisid";
    inline static const char *KeyWifiPwd = "wifipwd";
    inline static const char *KeyWeerliveKey = "weerlivekey";
    inline static const char *KeyWeerliveLocation = "weerliveloc";
    inline static const char *KeyDST  = "dst";
    inline static const char *KeyBootscreen  = "boot";
    inline static const char *KeySmoothSecondHand  = "secondhand";
    inline static const char *KeyFlipDisplay  = "flipdisplay";

    AppSettings()
    {        
        add(KeyLanguage, "nl");
        add(KeyWifiSid, "__sid__");
        add(KeyWifiPwd, "__password__");
        add(KeyWeerliveKey, "demo");
        add(KeyWeerliveLocation, "Amsterdam");
        add(KeyDST, false);
        add(KeyBootscreen, true);
        add(KeySmoothSecondHand, true);
        add(KeyFlipDisplay, true);

        loadSettings();
    }

    const char *Language() const { return get(KeyLanguage)->asstring(); }
    void Language(const char *value) { return get(KeyLanguage)->set(value); }

    const char *WifiSid() const { return get(KeyWifiSid)->asstring(); }
    void WifiSid(const char *value) { return get(KeyWifiSid)->set(value); }

    const char *WifiPassword() const { return get(KeyWifiPwd)->asstring(); }
    void WifiPassword(const char *value) { return get(KeyWifiPwd)->set(value); }

    const char *WeerliveKey() const { return get(KeyWeerliveKey)->asstring(); }
    void WeerliveKey(const char *value) { return get(KeyWeerliveKey)->set(value); }

    const char *WeerliveLocation() const { return get(KeyWeerliveLocation)->asstring(); }
    void WeerliveLocation(const char *value) { return get(KeyWeerliveLocation)->set(value); }

    bool DST() const { return get(KeyDST)->asbool(); }
    void DST(bool dst) const { return get(KeyDST)->set(dst); }

    bool Bootscreen() const { return get(KeyBootscreen)->asbool(); }
    void Bootscreen(bool show) const { return get(KeyBootscreen)->set(show); }

    int SmoothSecondHand() const { return get(KeySmoothSecondHand)->asbool(); }
    void SmoothSecondHand(bool smooth) const { return get(KeySmoothSecondHand)->set(smooth); }

    bool FlipDisplay() const { return get(KeyFlipDisplay)->asbool(); }
    void FlipDisplay(bool flip) const { return get(KeyFlipDisplay)->set(flip); }
};

#endif
