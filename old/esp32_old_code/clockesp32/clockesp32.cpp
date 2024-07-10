// clockesp32.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include "display.h"
#include "sg.h"
#include "schrift.h"

int main()
{
    display disp("\\\\.\\com10", 128, 64);

    auto bitmap = Bitmap::create(128, 64, 3, Bitmap::FLG_COMPACT);
    bitmap->clear(Color::black);

    Graphics graphics(bitmap);

    int count = 0;
    float thickness = 3;
    for (;;)
    {
        count++;
        std::cout << "line loop " << count << "\n";

        for (float a=0.15; a<6.2831; a+=0.001)
        //for (float a=0.0; a<70; a+=0.35)
        {
            float ox = cos(a) * 15;
            float oy = sin(a) * 15;
            graphics.clear(Color::black);
            graphics.line1(64 + 32, 32, 64 + 32 + ox, 32 + oy, Color::blue);
            graphics.line(31.5, 31.5, 31.5+ox, 31.5+oy, Color::red, 15, &disp);

            //graphics.setfont(60, Color::orangered).text(ox, oy, "Magma");

            disp.send(bitmap);
        }

        //float angle = 0;
        //for (float i = 0; i < 100; i += 0.25)
        //{
        //    graphics.clear(0);
        //    graphics.scanline(2 + i, 10 + i, i, Color::red);

        //    disp.send(bitmap);
        //    std::this_thread::sleep_for(std::chrono::milliseconds(5));
        //}
    }
}