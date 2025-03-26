
#ifndef _OTAUI_H_
#define _OTAUI_H_

#include <string>
#include <vector>

#include "consolebase.h"
#include "version.h"

class OTAUI : public ConsoleBase
{
private:
    enum state { readmanifest, idle, update, restartcountdown, restarting };

private:
    state _state;
    bool _cancel;
    Version _manifestVersion;
    int _checkUpdateInterval;
    bool _automatic;
    uint64_t _checkUpdateTimer;

public:
    OTAUI(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput);

    void init() override;
    int interact() override;

    bool isUpdateAvailable();
    void setAutomatic(bool automatic) { _automatic = automatic; }

private:
    bool readManifest(bool silent);
    void updateOverTheAir();
    void initiateRestart();
    void restart();
};

#endif
