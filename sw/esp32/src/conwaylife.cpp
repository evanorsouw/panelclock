
#include <algorithm>
#include <random>
#include <string.h>
#include "esp_random.h"

#include "color.h"
#include "conwaylife.h"

ConwayLife::ConwayLife(ApplicationContext &appdata, int dx, int dy)
    : _ctx(appdata)
{
    _maxdx = dx;
    _maxdy = dy;
    int ncells = dx * dy * 2;
    _cells = new uint8_t[ncells];
    _enabled = false;
    _stopping = false;
    init(dx, dy);
    
    _generateInterval = 200;
    _dimUpDuration = 400;
    _dimDownDuration = 1000;

    printf("conway constructed with max %dx%d (%d cells)\n", dx, dy, ncells);
}

void ConwayLife::init(int dx, int dy)
{
    if (dx > _maxdx || dy > _maxdy)
        return;

    printf("init conway %dx%d\n", dx, dy);

    _dx = dx;
    _dy = dy;
    _cells1 = _cells;
    _cells2 = _cells + _dx * _dy;
    memset(_cells, 0, _dx*_dy*2);
}

bool ConwayLife::enable(bool on)
{
    bool starting = !_enabled && on;
    _stopping = _enabled && !on;
    if (starting)
    {
        printf("starting conway\n");
        memset(_cells, 0,  _dx * _dy * 2);
        _enabled = true;
        _enableStart = _ctx.starttimer();
    }
    return starting;
}

void ConwayLife::render(Graphics &graphics, const timeinfo &now)
{
    if (!_enabled)
        return;

    generation(now);
    auto elapsed = _ctx.elapsed(_enableStart);
    auto show = elapsed/100;

    Color colBorn = _colBorn;
    Color colDying = _colDie * 0.4f;
    Color colAlive = _colAlive * 0.4f;

    float dy = graphics.dy() / _dy;
    float dx = graphics.dx() / _dx;

    uint8_t *pcells = _side ? _cells2 : _cells1;
    for (int y=0; y < _dy; ++y)
    {
        for (int x=0; x < _dx; ++x)
        {
            if (x + y > show)
                continue;

            auto cell = pcells[x];

            if (cell == 0)
                continue;

            float f = (cell & 0x7F) / 127.0f;
            Color color;
            if (cell < 128)
            {
                color = Color::gradient(colDying, colAlive, f) * f;
            }
            else
            {
                color = Color::gradient(colBorn, colAlive, f) * f;
            }
            graphics.rect(x * dx + 0.5f, y * dy + 0.5f, dx - 1.0f, dy - 1.0f, color);
        }
        pcells += _dx;
    }
}

void ConwayLife::addGlider(int x, int y)
{
    uint8_t *p = _side ? _cells2 : _cells1;
    switch (esp_random() % 4)
    {
        case 0:
            set(p, x - 1, y - 1);
            set(p, x, y - 1);
            set(p, x + 1, y - 1);
            set(p, x + 1, y);
            set(p, x, y + 1);
            break;
        case 1:
            set(p, x + 1, y - 1);
            set(p, x - 1, y);
            set(p, x + 1, y);
            set(p, x, y + 1);
            set(p, x + 1, y + 1);
            break;
        case 2:
            set(p, x, y - 1);
            set(p, x - 1, y);
            set(p, x - 1 , y + 1);
            set(p, x, y + 1);
            set(p, x + 1, y + 1);
            break;
        case 3:
            set(p, x - 1, y - 1);
            set(p, x, y - 1);
            set(p, x - 1 , y);
            set(p, x + 1, y);
            set(p, x - 1, y + 1);
            break;
    }
}

void ConwayLife::fillRandom()
{
    uint8_t *p = _side ? _cells2 : _cells1;
    for (int i=_dx*_dy/4; i-- > 0;)
    {
        set(p, esp_random() % _dx, esp_random() % _dy);
    }
}

void ConwayLife::randomColor()
{
    switch (esp_random() % 2)
    {
    case 0:
        _colAlive = Color::orangered;
        _colDie = Color::red;
        _colBorn = Color::yellow;
        break;
    case 1:
        _colAlive = Color::green;
        _colDie = Color::darkgreen;
        _colBorn = Color::yellow;
        break;
    }
}

void ConwayLife::generation(const timeinfo &now)
{
    int dim_up = std::max(1, (int)((now.msticks() - _lastdim.msticks()) * 127 / _dimUpDuration));
    int dim_down = std::max(1, (int)((now.msticks() - _lastdim.msticks()) * 127 / _dimDownDuration));
    if (!_stopping)
    {
        dim_down = dim_up;
    }
    _lastdim = now;
    bool generate = (now.msticks() - _lastgen.msticks())  > _generateInterval;
    if (generate)
    {
        _lastgen = now;
    }

    uint8_t *psrc = _side ? _cells2 : _cells1;
    uint8_t *ptgt = _side ? _cells1 : _cells2;
    _side = _side ? 0 : 1;
    bool busy = false;

    for (int y=0; y<_dy; ++y)
    {
        for (int x = 0; x<_dx; ++x)
        {
            uint8_t cell = psrc[x + y * _dx];

            if (generate)
            {
                uint8_t neighbours = 0;
                if (!_stopping)
                {
                    neighbours = 
                        occupation(psrc,x-1,y-1) +
                        occupation(psrc,x,y-1) +
                        occupation(psrc,x+1,y-1) +
                        occupation(psrc,x-1,y) +
                        occupation(psrc,x+1,y) +
                        occupation(psrc,x-1,y+1) +
                        occupation(psrc,x,y+1) +
                        occupation(psrc,x+1,y+1);
                }
                bool born = neighbours == 3;
                bool die = neighbours < 2 || neighbours > 3;
                bool alive = cell & 0x80;

                if (alive && die)
                {
                    cell &= 0x7F;
                }
                else if (!alive && born)
                {
                    cell |= 0x80;
                }
            }
            if (cell & 0x80)
            {
                cell = std::min(255, cell + dim_up);
            }
            else if (cell > 0)
            {
                cell = std::max(0, cell - dim_down);
                busy = true;
            }
            ptgt[x + y * _dx] = cell;
        }
    }
    if (_stopping && !busy)
    {
        printf("stopping conway\n");
        _enabled = false;
    }
}
