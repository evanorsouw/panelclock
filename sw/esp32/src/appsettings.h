
#ifndef _APPSETTINGS_H_
#define _APPSETTINGS_H_

#include "settings.h"

class AppSettings : public Settings
{
public:
    AppSettings()
    {
        addSetting("language", "nl");
        addSetting("wifisid", "<<select>>");
        addSetting("wifipwd", "<<select>>");
        addSetting("weerlivekey", "31a26656d0");
        addSetting("weerlivelocation", "51.732034245965046,5.32637472036842");
        addSetting("dst", false);
    }

    Setting *Language() const { return getSetting("language"); }
    Setting *WifiSid() const { return getSetting("wifisid"); }
    Setting *WifiPassword() const { return getSetting("wifipwd"); }
    Setting *WeerliveKey() const { return getSetting("weerlivekey"); }
    Setting *WeerliveLocation() const { return getSetting("weerlivelocation"); }
    Setting *DST() const { return getSetting("dst"); }
};

#endif
