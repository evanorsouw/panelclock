
#ifndef _CONWAYLIFE_H_
#define _CONWAYLIFE_H_

#include <string.h>

#include "applicationcontext.h"
#include "color.h"
#include "graphics.h"
#include "timeinfo.h"

class ConwayLife
{
private:
    ApplicationContext &_ctx; 
    int _maxdx;
    int _maxdy;
    uint8_t *_cells;        // cell: bit7=alive, bits6..0=intensity
    int _dx;
    int _dy;
    uint8_t *_cells1;
    uint8_t *_cells2;
    timeinfo _lastgen;
    timeinfo _lastdim;
    int _side;
    int _generateInterval;
    int _dimUpDuration;
    int _dimDownDuration;
    uint64_t _enableStart;
    bool _enabled;
    bool _stopping;
    Color _colAlive;
    Color _colBorn;
    Color _colDie;

public:
    ConwayLife(ApplicationContext &appdata, int dx, int dy);

    void init(int dx, int dy);
    bool enable(bool on);
    int dx() const { return _dx; }
    int dy() const { return _dy; }
    void render(Graphics &graphics, const timeinfo &now);
    void addGlider(int x, int y);
    void fillRandom();
    void randomColor();

private:
    void set(uint8_t *p, int x, int y)
    {
        x = (x + _dx) % _dx;
        y = (y + _dy) % _dy;
        p[x + y * _dx] |= 0x80;
    }
    int occupation(uint8_t *p, int x, int y)
    {
        x = (x + _dx) % _dx;
        y = (y + _dy) % _dy;
        return (p[x + y * _dx] & 0x80) ? 1 : 0;
    }
    void generation(const timeinfo &now);
};

#endif
