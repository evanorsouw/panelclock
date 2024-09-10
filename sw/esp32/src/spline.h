
#ifndef _SPLINE_H_
#define _SPLINE_H_

#include <vector>
#include <cmath>

struct point {
    point() :x(0), y(0) {}
    point(float ax, float ay) : x(ax), y(ay) {}
    float x;
    float y;
};

class Spline
{
private:
    std::vector<point> _controlPoints;
    int _degree;
    std::vector<float> _knots;

public:
    Spline() : Spline(std::vector<point>{ point(0,0) }, 1) {}
    Spline(std::vector<point> controlpoints, int degree);

    point get(float t);

private:
    float basisFunction(int i, int k, float t);
};

#endif
