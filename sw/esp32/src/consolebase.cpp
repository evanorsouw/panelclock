
#include "consolebase.h"

void ConsoleBase::init()
{
    _yTop = 0;
    _lines.clear();
}

void ConsoleBase::render(Graphics& graphics)
{
    std::lock_guard<std::mutex> lock(_loglock);
    auto font = _appctx.fontSettings();
    for (auto i=0; i<_lines.size(); i++)
    {
        auto yt = _yTop + i * font->height();
        if (yt >= graphics.dy())
            break;
        auto yb = yt + font->height() - 1;
        if (yb < 0)
            continue;

        auto &info = _lines[i];
        if (info.type == linetype::choices)
        {
            auto x = 0;
            for (auto i=0; i<info.line.size(); ++i)
            {
                auto size = font->textsize(info.line[i].c_str());
                if (info.choice == i)
                {
                    graphics.rect(x+1, yt, size.dx, size.dy + font->descend(), Color::darkgreen);
                    graphics.rect(x, yt + 1, size.dx + 2, size.dy - 2 + font->descend(), Color::darkgreen);
                    graphics.text(font, x + 1, yb + font->descend(), info.line[i].c_str(), info.color());
                }
                else
                {
                    graphics.text(font, x + 1, yb + font->descend(), info.line[i].c_str(), info.color());
                }
                x += size.dx + 2;
            }
        }
        else
        {
            auto line = info.line[0].c_str();
            auto x = graphics.text(font, -info.scrolloffset, yb + font->descend(), line, info.color());
            if (info.type == linetype::progress && i == (_lines.size() - 1))
            {
                char dots[4] = "   ";
                dots[(drawtime() / 250) % 3] = '.';
                graphics.text(font, x, yb + font->descend(), dots, info.color());
            }
            auto linedx = font->textsize(line).dx;
            switch (info.state)
            {
            case scrollstate::begin:
                info.scrolloffset = 0;
                if (timeout(info.scrolldelay, 1000))
                {
                    starttimer(info.scrolldelay);
                    info.state = scrollstate::scrolling;
                }
                break;
            case scrollstate::scrolling:
                if (info.scrolloffset + graphics.dx() >= linedx)
                {
                    starttimer(info.scrolldelay);
                    info.state = scrollstate::end;
                }
                else
                {
                    info.scrolloffset++;
                }
                break;
            case scrollstate::end:
                if (timeout(info.scrolldelay, 1500))
                {
                    info.state = scrollstate::begin;
                    starttimer(info.scrolldelay);
                }
                break;
            }
        }
    }
    auto yb = (int)_yTop + _lines.size() * font->height();
    auto yt = yb - font->height();
    auto ytarget = _yTop;
    if (yt < 0)
        ytarget += -yt;
    else if (yb > graphics.dy())
        ytarget -= yb - graphics.dy();

    graduallyUpdateVariable(_yTop, ytarget, 0.5f);
}

void ConsoleBase::removeLogs(int n)
{
    std::lock_guard<std::mutex> lock(_loglock);
    while (n-- > 0)
    {
        if (!_lines.empty())
        {
            _lines.pop_back();
        }
    }
}

void ConsoleBase::log(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    log(linetype::info, fmt, argp);
}

void ConsoleBase::detail(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    log(linetype::detail, fmt, argp);
}

void ConsoleBase::error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    log(linetype::error, fmt, argp);
}

void ConsoleBase::progress(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    log(linetype::progress, fmt, argp);
}

void ConsoleBase::choices(std::initializer_list<std::string> list)
{
    _lines.push_back(lineinfo(list, linetype::choices));
}

std::string ConsoleBase::handleChoice(KeyPress& press) 
{
    if (_lines.back().type == linetype::choices)
    {
        auto &info = _lines.back();
        switch (press.key)
        {
        case UserInput::KEY_SET:
            {
                auto choice = info.line[info.choice];
                auto n = choice.find(' ');
                if (n != std::string::npos)
                {
                    choice = choice.substr(0, n);
                }
                return choice;
            }
        case UserInput::KEY_DOWN:
            info.choice = (info.choice + 1) % info.line.size(); 
            break;
        case UserInput::KEY_UP:
            info.choice = (info.choice - 1 + info.line.size()) % info.line.size(); 
            break;
        }        
    }
    return "";
}

void ConsoleBase::log(linetype type, const char *fmt, va_list argp)
{
    std::lock_guard<std::mutex> lock(_loglock);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, argp);
    _lines.push_back(lineinfo(std::vector<std::string>{ buf }, type));
}

