
#ifndef _VERSION_H_
#define _VERSION_H_

#include <stdio.h>

class Version
{
private:
    int _major;
    int _minor;
    int _build;
    char _txt[30];

public:
    Version() : Version("0.0.0") {}
    Version(const char *version)
    {
        auto n = sscanf(version, "%d.%d.%d", &_major, &_minor, &_build);
        if (n != 3)
        {
            _major = _minor = _build = 0;
        }
        snprintf(_txt, sizeof(_txt), "%d.%d.%d", _major, _minor, _build);
    }
    static Version application();

    int major() const { return _major; }
    int minor() const { return _minor; }
    int build() const { return _build; }

    bool valid() const { return _major != 0 || _minor != 0 || _build != 0; }

    bool operator < (const Version &rhs) const { 
        if (_major != rhs._major) return _major < rhs._major;
        if (_minor != rhs._minor) return _minor < rhs._minor;
        return _build < rhs._build;
    }
    bool operator > (const Version &rhs) const { 
        if (_major != rhs._major) return _major > rhs._major;
        if (_minor != rhs._minor) return _minor > rhs._minor;
        return _build > rhs._build;
    }
    bool operator >= (const Version &rhs) const { 
        return !(*this < rhs);
    }
    bool operator <= (const Version &rhs) const { 
        return !(*this > rhs);
    }
    bool operator != (const Version &rhs) const { 
        return *this < rhs || *this > rhs;
    }
    bool operator == (const Version &rhs) const { 
        return !(*this != rhs);
    }

    const char *astxt() const { return _txt; }
};

#endif
