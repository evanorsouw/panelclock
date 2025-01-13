
#include "version.h"

// major.minor.buildno
// buildno is incremented with each build.
// major.minor must be updated manually.
#define VERSION "1.0.1196"

Version Version::application()
{
    return Version(VERSION);
}
