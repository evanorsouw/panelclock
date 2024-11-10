
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
    inline static const char *KeyWeerliveLocation = "weerlivelocation";
    inline static const char *KeyDST  = "dst";

    AppSettings()
    {        
        add(KeyLanguage, "nl");
        add(KeyWifiSid, "__sid__");
        add(KeyWifiPwd, "__password__");
        add(KeyWeerliveKey, "31a26656d0");
        add(KeyWeerliveLocation, "51.732034245965046,5.32637472036842");
        add(KeyDST, false);

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
};

#endif
