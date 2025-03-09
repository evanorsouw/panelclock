
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

public:
    OTAUI(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput)
        : ConsoleBase(appdata, env, sys, userinput)
    {}

    void init() override;
    int interact() override;

private:
    void readManifest();
    void updateOverTheAir();
    void initiateRestart();
    void restart();
};

#endif
