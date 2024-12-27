
#ifndef _XY_H_
#define _XY_H_

template<class T>
struct xy
{
    xy() : x(0), y(0) {}
    xy(T ix, T iy) : x(ix), y(iy) {}
    T x;
    T y;
};

typedef xy<int> ixy;
typedef xy<float> fxy;

#endif
