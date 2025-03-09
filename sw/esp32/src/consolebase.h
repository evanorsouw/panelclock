
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <string>
#include <vector>

#include "renderbase.h"

enum class linetype { info, detail, error, choices, progress };
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
        autoChoiceDelay = 0;
        if (atype == linetype::choices)
        {
            for (int i=0; i<line.size(); ++i)
            {
                auto pos = line[i].find_last_of(':');
                if (pos != std::string::npos)
                {
                    choice = i;
                    autoChoiceDelay = std::atoi(line[i].substr(pos+1).c_str());
                    line[i] = line[i].substr(0, pos);
                }
            }
        }
    }
    std::vector<std::string> line;
    linetype type;
    int scrolloffset;
    scrollstate state;
    uint64_t scrolldelay;
    int choice;
    int autoChoiceDelay;

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

class ConsoleBase : public RenderBase
{
private:
    std::vector<lineinfo> _lines;
    std::mutex _loglock;
    float _yTop;
    uint64_t _autoChoiceTimer;

public:
    ConsoleBase(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput)
        : RenderBase(appdata, env, sys, userinput)
    {}

    void init() override;
    void render(Graphics& graphics) override;

protected:
    void log(const char *fmt, ...);
    void detail(const char *fmt, ...);
    void error(const char *fmt, ...);
    void progress(const char *fmt, ...);
    void choices(std::initializer_list<std::string> list);
    void removeLogs(int n);
    std::string handleChoice(KeyPress& press); 

private:
    void log(linetype type, const char *fmt, va_list argp);
};

#endif
