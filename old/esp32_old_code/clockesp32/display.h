
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <windows.h>
#undef min
#undef max
#include <string>
#include "sg.h"

class display
{
private:
    std::string _port;
    int _dx;
    int _dy;
    bool _isOpen;
    HANDLE _handle;

public:
    display(std::string port, int dx, int dy)
    {
        _port = port;
        _dx = dx;
        _dy = dy;
    }

    bool send(Bitmap *image)
    {
        return send(image, 0, 0, _dx, _dy);
    }

    bool send(Bitmap* image, int x, int y, int dx, int dy)
    {
        if (image->width() != _dx || image->height() != _dy)
            return false;   // image <=> display size mismatch

        unsigned char *bytes = new unsigned char[_dx * 3 + 10];
        int n = 0;

        dx = std::max(0, std::min(_dx, dx));
        dy = std::max(0, std::min(_dy, dy));

        for (int iy = y; iy < y + dy; ++iy)
        {
            n = 0;
            int adr = iy * _dx + x;
            auto ptr = image->pixptr(x, iy);

            bytes[n++] = 1;
            bytes[n++] = adr / 256;
            bytes[n++] = adr % 256;
            bytes[n++] = 2;
            bytes[n++] = dx;
            for (int ix = dx; ix-- > 0;)
            {
                n += ptr.copyright(bytes + n);
            }
            write(bytes, n);
        }
        return true;
    }

    bool write(byte *bytes, int n)
    {
        if (!open())
            return false;

        DWORD nwritten;
        auto result = WriteFile(_handle, bytes, n, &nwritten, NULL);
        if (!result || n != nwritten)
        {
            close();
            return false;
        }
        return true;
    }

private:
    bool open()
    {
        if (_isOpen)
            return true;

        std::wstring stemp = std::wstring(_port.begin(), _port.end());
        _handle = CreateFile((LPCWSTR)stemp.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (_handle == INVALID_HANDLE_VALUE)
            return false;

        DCB parms;
        if (!GetCommState(_handle, &parms))
            return false;

        parms.BaudRate = 3000000;
        parms.ByteSize = 8;
        parms.StopBits = ONESTOPBIT;
        parms.Parity = NOPARITY;
        if (!SetCommState(_handle, &parms))
            return false;

        _isOpen = true;
        return true;
    }

    void close()
    {
        CloseHandle(_handle);
        _isOpen = false;
    }
};

#endif
