
#include "spline.h"


Spline::Spline(std::vector<point> controlPoints, int degree)
    : _controlPoints(controlPoints)
    , _degree(degree)
{
    int n = controlPoints.size() - 1;
    int m = n + degree + 1;

    // Generate uniform knot vector
    _knots.resize(m + 1);
    for (int i = 0; i <= m; ++i) {
        if (i <= degree) {
            _knots[i] = 0.0;
        } else if (i >= m - degree) {
            _knots[i] = 1.0;
        } else {
            _knots[i] = (i - degree) / (float)(m - 2 * degree);
        }
    }
}

float Spline::basisFunction(int i, int k, float t) 
{
    if (k == 1)
        return (t >= _knots[i] && t < _knots[i + 1]) ? 1.0 : 0.0;

    float denom1 = _knots[i + k - 1] - _knots[i];
    float denom2 = _knots[i + k] - _knots[i + 1];
    float term1 = denom1 == 0 ? 0 : (t - _knots[i]) / denom1 * basisFunction(i, k - 1, t);
    float term2 = denom2 == 0 ? 0 : (_knots[i + k] - t) / denom2 * basisFunction(i + 1, k - 1, t);
    return term1 + term2;
}

point Spline::get(float t) 
{
    point point = {0.0, 0.0};
    int n = _controlPoints.size() - 1;
    for (int i = 0; i <= n; ++i) {
        float b = basisFunction(i, _degree + 1, t);
        point.x += b * _controlPoints[i].x;
        point.y += b * _controlPoints[i].y;
    }
    return point;
}
