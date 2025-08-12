
#include <algorithm>
#include "applicationrunner.h"
#include "bitmap.h"

ApplicationRunner::ApplicationRunner(
    ApplicationContext& appdata, 
    LedPanel& panel, 
    Application& appui, 
    ConfigurationUI& configui,
    OTAUI& otaui, 
    SetupUI& setupui, 
    System& system,
    Graphics& graphics)
    : _appctx(appdata)
    , _panel(panel)
    , _appui(appui)
    , _configui(configui)
    , _otaui(otaui)
    , _setupui(setupui)
    , _system(system)
    , _graphics(graphics)
{
    _appctx.starttimer(_totaltime);
    _appctx.starttimer(_totalrendertime);
    _appctx.starttimer(_totaldisplaytime);
    _appctx.starttimer(_lastdisplaytime);
    _rendercount = 0;
    _fpsInterval = 10000;

    _iShowScreen = 0;
    _automaticSoftwareUpdate = false;

    startMode(UIMode::DateTime, TransitionPhase::Entering);

    // 2 screen bitmaps, 1 being copied, the other for rendering the next one
    _hRenderQueue = xQueueCreate(2, sizeof(Bitmap *));
    auto screen = new Bitmap(panel.dx(), panel.dy(), 3);
    xQueueSend(_hRenderQueue, &screen, 0);
    screen = new Bitmap(panel.dx(), panel.dy(), 3);
    xQueueSend(_hRenderQueue, &screen, 0);

    _hDisplayQueue = xQueueCreate(2, sizeof(Bitmap *));
}

void ApplicationRunner::render()
{
    uint64_t timer;

    Bitmap *screen = 0;
    xQueueReceive(_hRenderQueue, &screen, 1000);
    
    screen->fill(Color::black);
    _graphics.linkBitmap(screen);
    _appctx.starttimer(timer);
    _appctx.settime(_system.now());

    auto width = _system.settings().PanelMode() == 1 ? 128 : 64;
    auto height = _system.settings().PanelMode() == 2 ? 128 : 64;
    auto view = _graphics.fullview(width).moveOrigin(0, 0, width, height);
    render(view);
    xQueueSend(_hDisplayQueue, &screen, 1000);
    _totalrendertime += _appctx.elapsed(timer);
}

void ApplicationRunner::display()
{
    uint64_t timer;

    auto elapsed = _appctx.elapsed(_lastdisplaytime);
    if (elapsed < 40)
    {
        vTaskDelay((40 - elapsed) / portTICK_PERIOD_MS);
    }
    _appctx.starttimer(_lastdisplaytime);

    Bitmap *screen;
    xQueueReceive(_hDisplayQueue, &screen, 1000);

    _appctx.starttimer(timer);
    _panel.copyFrom(*screen);
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
        case UIMode::DateTime: _appui.init(); break;
        case UIMode::Config: _configui.init(); break;            
        case UIMode::OTA: _otaui.init(); break;            
        case UIMode::Setup: _setupui.init(); break;            
    }
}

void ApplicationRunner::startTransition(TransitionPhase phase)
{
    _appctx.starttimer(_transitionStart);
    _phase = phase;
}

void ApplicationRunner::render(Graphics &graphics)
{
    auto interact = true;
    if (_phase != TransitionPhase::Stable)
    {
        updateIntensity();
        interact = false;
    }

    switch (_mode)
    {
    case UIMode::DateTime:
        _appui.render(graphics);
        if (interact)
        {
            if (_automaticSoftwareUpdate)
            {
                _otaui.setAutomatic(true);
                startMode(UIMode::OTA, TransitionPhase::Leaving);
            } 
            else switch (_appui.interact())
            {
            case 1:
                startMode(UIMode::Config, TransitionPhase::Leaving);
                break;
            case 2:
                startMode(UIMode::OTA, TransitionPhase::Leaving);
                break;
            case 3:
                startMode(UIMode::Setup, TransitionPhase::Leaving);
                break;
            }
        }
        break;
    case UIMode::Config:
        _configui.render(graphics);
        if (interact)
        {
            switch (_configui.interact())
            {
            case 1:
                startMode(UIMode::DateTime, TransitionPhase::Leaving);
                break;
            case 2:
                _otaui.setAutomatic(false);
                startMode(UIMode::OTA, TransitionPhase::Leaving);
                break;
            case 3:
                startMode(UIMode::Setup, TransitionPhase::Leaving);
                break;
            }
        }
        break;
    case UIMode::OTA:
        _otaui.render(graphics);
        if (interact && _otaui.interact())
        {
            startMode(UIMode::DateTime, TransitionPhase::Leaving);
        }
        break;
    case UIMode::Setup:
        _setupui.render(graphics);
        if (interact && _setupui.interact())
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
    if (elapsed > _fpsInterval)
    {
        _appctx.starttimer(_totaltime);
        auto fps = _rendercount / (_fpsInterval / 1000.0f);
        auto prender = _totalrendertime * 100.0f / elapsed;
        auto pdisplay = _totaldisplaytime * 100.0f / elapsed;

        _rendercount = 0;
        _totaldisplaytime = 0;
        _totalrendertime = 0;
        _fpsInterval = std::min(_fpsInterval * 2, 10*60*1000);
        printf("%.1f fps, render=%.1f%%, display=%.1f%%\n", fps, prender, pdisplay);
    }
}
