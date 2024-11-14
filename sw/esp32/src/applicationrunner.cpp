
#include <algorithm>
#include "applicationrunner.h"
#include "bitmap.h"

ApplicationRunner::ApplicationRunner(ApplicationContext& appdata, LedPanel& panel, BootAnimations& bootui, Application& appui, ConfigurationUI& configui, System& system)
    : _appctx(appdata)
    , _panel(panel)
    , _bootui(bootui)
    , _appui(appui)
    , _configui(configui)
    , _system(system)
{
    _refreshCountStart = std::numeric_limits<long>::max();
    _refreshCount = 0;
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
    _appctx.settime(_system.now());

    Bitmap *screen = 0;
    xQueueReceive(_hRenderQueue, &screen, 1000);
    monitorRefreshRate();
    screen->fill(Color::black);
    stepGUI(*screen);
    xQueueSend(_hDisplayQueue, &screen, 1000);
}

void ApplicationRunner::displayTask()
{
    Bitmap *screen;
    xQueueReceive(_hDisplayQueue, &screen, 1000);
    screen->copyTo(_panel, 0, 0);
    _panel.showScreen(_iShowScreen);
    if (++_iShowScreen == 3)
        _iShowScreen = 0;
    _panel.selectScreen(_iShowScreen);
    xQueueSend(_hRenderQueue, &screen, 1000);

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

void ApplicationRunner::stepGUI(Bitmap& screen)
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
        _bootui.render(screen);
        if (interact && _bootui.interact())
        {
            startMode(UIMode::DateTime, TransitionPhase::Entering);
        }
        break;
    case UIMode::DateTime:
        _appui.render(screen);
        if (interact && _appui.interact())
        {
            startMode(UIMode::Config, TransitionPhase::Leaving);
        }
        break;
    case UIMode::Config:
        _configui.render(screen);
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
    _refreshCount++;
    auto now = _appctx.msSinceMidnight();
    auto elapsed = now - _refreshCountStart;
    if (elapsed < 0 || elapsed > 30000)
    {
        _refreshCount = 0;
        _refreshCountStart = now;
    }
    else if (elapsed > 10000)
    {
        printf("%d fps\n", _refreshCount /10);
        _refreshCount = 0;
        _refreshCountStart += 10000;
    }
}

