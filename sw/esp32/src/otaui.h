
#ifndef _OTAUI_H_
#define _OTAUI_H_

#include <string>
#include <vector>

#include "renderbase.h"

enum class linetype { info, detail, error, choices };
enum class scrollstate { begin, scrolling, end };

struct lineinfo
{
    lineinfo(const lineinfo &rhs) = default;
    lineinfo(std::vector<std::string> aline, linetype atype) 
    {
        line = aline;
        type = atype;
        scrolloffset = 0;
        scrolldelay = 0;
        state = scrollstate::begin;
        choice = 0;
    }
    std::vector<std::string> line;
    linetype type;
    int scrolloffset;
    scrollstate state;
    uint64_t scrolldelay;
    int choice;

    Color color() const
    {
        switch (type)
        {
        case linetype::info: 
        default:
            return Color::white;
        case linetype::detail: 
            return Color::green;
        case linetype::error: 
            return Color::lightcoral;
        }
    }
};

class OTAUI : public RenderBase
{
private:
    std::vector<lineinfo> _lines;
    bool _OTARunning;
    std::mutex _loglock;
    float _yTop;
    bool _cancel;

public:
    OTAUI(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput)
        : RenderBase(appdata, env, sys, userinput)
    {}

    void init() override;
    void render(Graphics& graphics) override;
    int interact() override;

private:
    void updateOverTheAir();
    void restart();
    void removeLogs(int n);
    void log(linetype type, const char *fmt, ...);
    void choices(std::initializer_list<std::string> list);
};

#endif
