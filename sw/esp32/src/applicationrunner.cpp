
#include <algorithm>
#include "applicationrunner.h"
#include "bitmap.h"

ApplicationRunner::ApplicationRunner(
    ApplicationContext& appdata, 
    LedPanel& panel, 
    BootAnimations& bootui, 
    Application& appui, 
    ConfigurationUI& configui, 
    System& system,
    Graphics& graphics)
    : _appctx(appdata)
    , _panel(panel)
    , _bootui(bootui)
    , _appui(appui)
    , _configui(configui)
    , _system(system)
    , _graphics(graphics)
{
    _appctx.starttimer(_totaltime);
    _appctx.starttimer(_totalrendertime);
    _appctx.starttimer(_totaldisplaytime);
    _rendercount = 0;

    _iShowScreen = 0;

    if (system.settings().Bootscreen())
        startMode(UIMode::Boot, TransitionPhase::Stable);
    else
        startMode(UIMode::DateTime, TransitionPhase::Entering);

    // 2 screen bitmaps, 1 being copied, the other for rendering the next one
    _hRenderQueue = xQueueCreate(2, sizeof(Bitmap *));
    auto screen = new Bitmap(panel.dx(), panel.dy(), 3);
    xQueueSend(_hRenderQueue, &screen, 0);
    screen = new Bitmap(panel.dx(), panel.dy(), 3);
    xQueueSend(_hRenderQueue, &screen, 0);

    _hDisplayQueue = xQueueCreate(2, sizeof(Bitmap *));
}

void ApplicationRunner::renderTask()
{
    uint64_t timer;

    Bitmap *screen = 0;
    xQueueReceive(_hRenderQueue, &screen, 1000);
    
    screen->fill(Color::black);
    _graphics.linkBitmap(screen);
    _appctx.starttimer(timer);
    _appctx.settime(_system.now());
    stepGUI();

    xQueueSend(_hDisplayQueue, &screen, 1000);
    _totalrendertime += _appctx.elapsed(timer);
}

void ApplicationRunner::displayTask()
{
    uint64_t timer;

    Bitmap *screen;
    xQueueReceive(_hDisplayQueue, &screen, 1000);
    _appctx.starttimer(timer);
    screen->copyTo(_panel, 0, 0);
    _panel.showScreen(_iShowScreen);
    if (++_iShowScreen == 3)
        _iShowScreen = 0;
    _panel.selectScreen(_iShowScreen);
    xQueueSend(_hRenderQueue, &screen, 1000);

    _totaldisplaytime += _appctx.elapsed(timer);
    monitorRefreshRate();
}

void ApplicationRunner::startMode(UIMode mode, TransitionPhase phase)
{
    _nextMode = mode;    
    if (phase != TransitionPhase::Leaving) _mode = mode;
    if (phase == TransitionPhase::Entering) _appctx.intensity(0);
    startTransition(phase);
    
    switch (mode)
    {
        case UIMode::Boot: _bootui.init(); break;
        case UIMode::DateTime: _appui.init(); break;
        case UIMode::Config: _configui.init(); break;            
    }
}

void ApplicationRunner::startTransition(TransitionPhase phase)
{
    _appctx.starttimer(_transitionStart);
    _phase = phase;
}

void ApplicationRunner::stepGUI()
{
    auto interact = true;
    if (_phase != TransitionPhase::Stable)
    {
        updateIntensity();
        interact = false;
    }
    switch (_mode)
    {
    case UIMode::Boot:
        _bootui.render();
        if (interact && _bootui.interact())
        {
            startMode(UIMode::DateTime, TransitionPhase::Entering);
        }
        break;
    case UIMode::DateTime:
        _appui.render();
        if (interact && _appui.interact())
        {
            startMode(UIMode::Config, TransitionPhase::Leaving);
        }
        break;
    case UIMode::Config:
        _configui.render();
        if (interact && _configui.interact())
        {
            startMode(UIMode::DateTime, TransitionPhase::Leaving);
        }
        break;
    }
}

void ApplicationRunner::updateIntensity()
{
    auto elapsed = _appctx.elapsed(_transitionStart) / 250.0f;
    auto intensity = 0.0f;
    switch (_phase)
    {
        case TransitionPhase::Leaving:
            intensity = std::max(0.0f, 1.0f - elapsed);
            if (intensity == 0.0f)
                startTransition(TransitionPhase::Entering);
            break;
        case TransitionPhase::Entering:
            intensity = std::min(1.0f, elapsed);
            if (intensity == 1.0f)
                startTransition(TransitionPhase::Stable);
            _mode = _nextMode;
            break;
        case TransitionPhase::Stable:
            intensity = 1;
            _mode = _nextMode;
            break;
    }
    _appctx.intensity(intensity);
}

void ApplicationRunner::monitorRefreshRate()
{
    _rendercount++;
    auto elapsed = _appctx.elapsed(_totaltime);
    if (elapsed > 10000)
    {
        _appctx.starttimer(_totaltime);
        auto fps = _rendercount / 10.0f;
        auto prender = _totalrendertime * 100.0f / elapsed;
        auto pdisplay = _totaldisplaytime * 100.0f / elapsed;

        _rendercount = 0;
        _totaldisplaytime = 0;
        _totalrendertime = 0;
        printf("%.1f fps, render=%.1f%%, display=%.1f%%\n", fps, prender, pdisplay);
    }
}
