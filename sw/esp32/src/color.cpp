
#include "color.h"

Color Color::transparant = Color(0x00000000);
Color Color::black = Color(0xFF000000);
Color Color::white = Color(0xFFFFFFFF);
Color Color::red = Color(0xFFFF0000);
Color Color::lime = Color(0xFF00FF00);
Color Color::blue = Color(0xFF0000FF);
Color Color::yellow = Color(0xFFFFFF00);
Color Color::cyan = Color(0xFF00FFFF);
Color Color::magenta = Color(0xFFFF00FF);
Color Color::silver = Color(0xFFC0C0C0);
Color Color::gray = Color(0xFF808080);
Color Color::lightgray = Color(0xFFA0A0A0);
Color Color::darkgray = Color(0xFF202020);
Color Color::maroon = Color(0xFF800000);
Color Color::olive = Color(0xFF808000);
Color Color::green= Color(0xFF00FF00);
Color Color::purple = Color(0xFF800080);
Color Color::teal = Color(0xFF008080);
Color Color::navy = Color(0xFF000080);
Color Color::brown = Color(0xFFA52A2A);
Color Color::orangered = Color(0xFFFF4500);
Color Color::skyblue = Color(0xFF87CEEB);
Color Color::pastelred = Color(0xFF6961);
Color Color::darkred = Color(0x8B0000);
Color Color::darksalmon = Color(0xE9967A);
Color Color::lightcoral = Color(0xF08080);

Color Color::gradient(const Color &from, const Color &to, float p)
{
    if (p < 0.0)
        return from;
    if (p > 1.0)
        return to;
    auto dr = to.r() - from.r();
    auto dg = to.g() - from.g();
    auto db = to.b() - from.b();
    return Color(from.r() + dr * p, from.g() + dg * p, from.b() + db * p);
}


