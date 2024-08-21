
#include "settings.h"

Settings::Settings()
{    
    loadSettings();
}

bool Settings::loadSettings()
{    
    addSetting("language", "nl");
    addSetting("wifisid", "evopevo_guest");
    addSetting("wifipwd", "hereyouar");
    addSetting("weerlivekey", "31a26656d0");
    addSetting("weerlivelocation", "51.732034245965046,5.32637472036842");
    return true;
}

bool Settings::saveSettings()
{
    return true;
}


