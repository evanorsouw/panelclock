
#include <stdio.h>
#include <string.h>

#include "timeinfo.h"
#include "version.h"

static char _version[30];

const char *version()
{
    while (_version[0] == 0)
    {
        strcpy(_version, VERSION);
        char mon[10];
        int year, day, hour, min, sec;
        if (sscanf(__DATE__, "%s %d %d", mon, &day, &year) != 3)
            break;
        if (sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec) != 3)
            break;
        for(int i=0; i<12; ++i)
        {
            if (!strcasecmp(timeinfo::MonthsCompact[i], mon))
            {
                sprintf(_version, "%s-%04d%02d%02d%02d%02d%02d", VERSION, year, i+1, day, hour, min, sec); 
            }
        }
    }
    return _version;
}