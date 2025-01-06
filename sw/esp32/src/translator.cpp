
#include <algorithm>
#include <string.h>
#include <map>

#include "translator.h"

static std::map<std::string, std::map<std::string, std::string>> _translations = 
{
    { "nl", 
        { 
            { ENG_MONDAY, "maandag" },
            { ENG_TUESDAY, "dinsdag" },
            { ENG_WEDNESDAY, "woensdag" },
            { ENG_THURSDAY, "donderdag" },
            { ENG_FRIDAY, "vrijdag" },
            { ENG_SATURDAY, "zaterdag" },
            { ENG_SUNDAY, "zondag" },
            { ENG_MON, "ma" },
            { ENG_TUE, "di" },
            { ENG_WED, "wo" },
            { ENG_THU, "do" },
            { ENG_FRI, "vr" },
            { ENG_SAT, "za" },
            { ENG_SUN, "zo" },
            { ENG_JANUARY, "januari" },
            { ENG_FEBRUARY, "februari" },
            { ENG_MARCH, "maart" },
            { ENG_MAY_LONG, "mei" },
            { ENG_JUNE, "juni" },
            { ENG_JULY, "juli" },
            { ENG_AUGUST, "augustus" },
            { ENG_OCTOBER, "oktober" },
            { ENG_MAY_SHORT, "mei" },
            { ENG_OCT, "okt" },
            { ENG_YEAR, "jaar" },
            { ENG_DATE, "datum" },
            { ENG_TIME, "tijd" },
            { ENG_TZ, "ZT" },
            { ENG_DAYLIGHT_SAVING, "zomertijd" },
            { ENG_STANDARD_TIME, "wintertijd" },
            { ENG_EXIT, "terug" },
            { ENG_LANG, "taal" },
            { ENG_BOOTSCREEN, "opstartscherm"},
            { ENG_NO_BOOTSCREEN, "geen opstartscherm"},
            { ENG_SCANNING, "zoeken"},
            { ENG_CONNECTING, "verbinden"},
            { ENG_UPDATING, "verversen"}
        }
    },
    { "fr", 
        { 
            { ENG_MONDAY, "lundi" },
            { ENG_TUESDAY, "mardi" },
            { ENG_WEDNESDAY, "mercredi" },
            { ENG_THURSDAY, "jeudi" },
            { ENG_FRIDAY, "vendredi" },
            { ENG_SATURDAY, "samedi" },
            { ENG_SUNDAY, "dimanche" },
            { ENG_JANUARY,  "janvier" },
            { ENG_FEBRUARY, "février" },
            { ENG_MARCH, "mars" },
            { ENG_APRIL, "avril" },
            { ENG_MAY_LONG, "mai" },
            { ENG_JUNE, "juin" },
            { ENG_JULY, "juillet" },
            { ENG_AUGUST, "aqoût" },
            { ENG_SEPTEMBER, "septembre" },
            { ENG_OCTOBER, "octobre" },
            { ENG_NOVEMBER, "novembre" },
            { ENG_DECEMBER, "décembre" },
            { ENG_JAN,  "janv" },
            { ENG_FEB, "févr" },
            { ENG_MAR, "mars" },
            { ENG_APR, "avr" },
            { ENG_MAY_SHORT, "mai" },
            { ENG_JUN, "juin" },
            { ENG_JUL, "juil" },
            { ENG_AUG, "août" },
            { ENG_SEP, "sept" },
            { ENG_OCT, "oct" },
            { ENG_NOV, "nov" },
            { ENG_DEC, "déc" },
            { ENG_YEAR, "annee" },
            { ENG_DATE, "date" },
            { ENG_TIME, "heure" },
            { ENG_TZ, "H.E." },
            { ENG_DAYLIGHT_SAVING, "Heure d'été"  },
            { ENG_STANDARD_TIME, "hiver" },
            { ENG_EXIT, "sortie" },
            { ENG_LANG, "langue" },
            { ENG_BOOTSCREEN, "écran de démarrage"},
            { ENG_NO_BOOTSCREEN, "pas d'écran de démarrage"},
            { ENG_SCANNING, "chercher"},
            { ENG_CONNECTING, "scanner"},
            { ENG_UPDATING, "Rafraîchir"}
        }
    }
};

const std::string &Translator::translate(const std::string &english)
{ 
    auto itlang = _translations.find(_lang->asstring());
    if (itlang != _translations.end())
    {
        auto it = itlang->second.find(english);
        if (it != itlang->second.end())
            return it->second;
    }
    return english;
}