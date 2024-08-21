
#ifndef _APPSETTINGS_H_
#define _APPSETTINGS_H_

#include "settings.h"

class AppSettings : public Settings
{
public:
    Setting *WifiSid() const { return getSetting("wifisid"); }
    Setting *WifiPassword() const { return getSetting("wifipwd"); }
    Setting *Language() const { return getSetting("language"); }
    Setting *WeerliveKey() const { return getSetting("weerlivekey"); }
    Setting *WeerliveLocation() const { return getSetting("weerliveloc"); }
};

#endif
