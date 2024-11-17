
#include <algorithm>
#include <string.h>
#include <map>

#include "translator.h"

static std::map<const char *, std::map<const char *, const char *>> _translations = 
{
        // todo: add month abbreviations
    { "nl", 
        { 
            { "monday", "maandag" },
            { "tuesday", "dinsdag" },
            { "wednesday", "woensdag" },
            { "thursday", "donderdag" },
            { "friday", "vrijdag" },
            { "saturday", "zaterdag" },
            { "sunday", "zondag" },
            { "may", "mei" },
            { "oct", "okt" },
            { "year", "jaar" },
            { "date", "datum" },
            { "time", "tijd" },
            { "DST", "ZT" },
            { "daylight saving", "zomertijd" },
            { "standard", "wintertijd" },
            { "exit", "terug" },
            { "lang", "taal" },
            { "show bootscreen", "opstartscherm"},
            { "no bootscreen", "geen opstartscherm"},
        }
    },
    { "fr", 
        { 
            { "monday", "lundi" },
            { "tuesday", "mardi" },
            { "wednesday", "mercredi" },
            { "thursday", "jeudi" },
            { "friday", "vendredi" },
            { "saturday", "samedi" },
            { "sunday", "dimanche" },
            { "jan",  "janv" },
            { "feb", "févr" },
            { "mar", "mars" },
            { "apr", "avr" },
            { "may", "mai" },
            { "jun", "juin" },
            { "jul", "juil" },
            { "aug", "août" },
            { "sep", "sept" },
            { "oct", "oct" },
            { "nov", "nov" },
            { "dec", "déc" },
            { "year", "annee" },
            { "date", "date" },
            { "time", "heure" },
            { "DST", "H.E."},
            { "daylight saving", "Heure d'été" },
            { "standard", "hiver" },
            { "exit", "sortie" },
            { "lang", "langue" },
        }
    }
};


const char *Translator::translate(const char *english)
{ 
    for (auto it1 : _translations)   
    {
        if (!strcmp(it1.first, _lang->asstring()))
        {
            for(auto it2 : it1.second)
            {
                if (!strcmp(it2.first, english))
                    return it2.second;
            }
            break;
        }
    }
    return english;
}