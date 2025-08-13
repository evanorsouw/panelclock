
#include "version.h"

// major.minor.buildno
// buildno is incremented with each build.
// major.minor must be updated manually.
#define VERSION "1.2.2133"

Version Version::application()
{
    return Version(VERSION);
}
