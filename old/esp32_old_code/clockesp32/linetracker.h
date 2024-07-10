#ifndef _LINETRACKER_H_
#define _LINETRACKER_H_

struct scanpoint
{
    scanpoint() {}
    scanpoint(int ax, int ay, int aix, int aiy) : x(ax), y(ay), ix(aix), iy(aiy) {}
    int x;
    int y;
    int ix;
    int iy;
    bool operator==(const scanpoint& rhs) const {
        return x == rhs.x && y == rhs.y && ix == rhs.ix && iy == rhs.iy;
    }
    bool operator!=(const scanpoint& rhs) const {
        return !(*this == rhs);
    }
};

class LineTracker
{
private:
    float _x1, _y1, _x2, _y2, _x3, _y3;
    float _x, _y;
    float _slope;
    int _phase = 0;
    bool _cw;

public:
    LineTracker(float x1, float y1, float x2, float y2, float x3, float y3, bool cw)
    {
        _x = _x1 = x1;
        _y = _y1 = y1;
        _x2 = x2;
        _y2 = y2;
        _x3 = x3;
        _y3 = y3;
        _slope = (_x2 - _x1) / (_y2 - _y1);
        _cw = cw;
    }

    bool next(scanpoint* p)
    {
        p->x = _x;
        p->y = _y;
        if (_cw)
        {
            p->ix = 255 * (_x - std::floor(_x));
            p->iy = 255 * (_y - std::floor(_y));
        }
        else
        {
            p->ix = 255 * (1 - (_x - std::floor(_x)));
            p->iy = 255 * (1 - (_y - std::floor(_y)));
        }

        float t;
        switch (_phase)
        {
        case 0:
            t = 1 - (_y - std::floor(_y));
            _y = std::floor(_y + 1);
            _x = _x + _slope * t;
            if (std::abs(_y2 - _y1) <= 0.0001)
            {
                _x = _x2;
                _y = _y2;
                _phase = 2;
            }
            else if (_y + 1 < _y2)
            {
                _phase = 1;
            }
            else if (_y + 1 <= _y3)
            {
                _phase = 2;
            }
            else
            {
                _phase = 4;
                _x = _x3;
                _y = _y3;
            }
            break;
        case 1:
            _y += 1;
            _x += _slope;
            _phase = (_y + 1 < _y2) ? 1 : 2;
            break;
        case 2:
            t = _y2 - _y;
            _y += 1;
            _x += t * _slope;
            _slope = (_x3 - _x2) / (_y3 - _y2);
            _x += (1 - t) * _slope;
            _phase = (_y < _y3) ? 3 : 4;
            break;
        case 3:
            _y += 1;
            _x += _slope;
            if (_y >= _y3)
            {
                _x = _x3;
                _y = _y3;
                _phase = 4;
            }
            break;
        case 4:
            p->ix = 255 - p->ix;
            p->iy = 255 - p->iy;
            _phase = 5;
            break;
        case 5:
            return false;
        }
        return true;
    }
};


#endif