
#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <settings.h>

#define ENG_STANDARD_TIME "standard"
#define ENG_DAYLIGHT_SAVING "daylight saving"
#define ENG_OPTION_SNAP "snap"
#define ENG_OPTION_SMOOTH "smooth"
#define ENG_BOOTSCREEN "show bootscreen"
#define ENG_NO_BOOTSCREEN "no bootscreen"
#define ENG_USBDOWN "usbdown"
#define ENG_USBUP "usbup"
#define ENG_NO_INTERNET "no internet"
#define ENG_CONNECTING "connecting"
#define ENG_UPDATING "updating"
#define ENG_SCANNING "scanning"
#define ENG_AUTOMATIC "automatic"
#define ENG_MANUAL "manual"
#define ENG_NTP_SERVER "NTP server"
#define ENG_NTP_INTERVAL "NTP interval"

#define ENG_VERSION "version"
#define ENG_TZ "TZ"
#define ENG_TZ_CUSTOM "timezone"
#define ENG_YEAR "year"
#define ENG_DATE "date"
#define ENG_TIME_MODE "timemode"
#define ENG_TIME "time"
#define ENG_IP "ip"
#define ENG_WIFI "wifi"
#define ENG_PASS "pass"
#define ENG_LANG "lang"
#define ENG_WEATHER_SOURCE "weathersource"
#define ENG_WEATHER_KEY "key"
#define ENG_WEATHER_LOC "loc"
#define ENG_WEATHER "weer"
#define ENG_BOOT "boot"
#define ENG_SEC "sec"
#define ENG_FLIP "Flip"
#define ENG_EXIT "exit"

#define ENG_MONDAY "monday"
#define ENG_TUESDAY "tuesday"
#define ENG_WEDNESDAY "wednesday"
#define ENG_THURSDAY "thursday"
#define ENG_FRIDAY "friday"
#define ENG_SATURDAY "saturday"
#define ENG_SUNDAY "sunday"
#define ENG_MON "mon"
#define ENG_TUE "tue"
#define ENG_WED "wed"
#define ENG_THU "thu"
#define ENG_FRI "fri"
#define ENG_SAT "sat"
#define ENG_SUN "sun"

#define ENG_JANUARY "january" 
#define ENG_FEBRUARY "february"
#define ENG_MARCH "march"
#define ENG_APRIL "april"
#define ENG_MAY_LONG "may"
#define ENG_JUNE "june"
#define ENG_JULY "july"
#define ENG_AUGUST "august"
#define ENG_SEPTEMBER "september"
#define ENG_OCTOBER "october"
#define ENG_NOVEMBER "november"
#define ENG_DECEMBER "december"

#define ENG_JAN "jan" 
#define ENG_FEB "feb"
#define ENG_MAR "mar"
#define ENG_APR "apr"
#define ENG_MAY_SHORT "may"
#define ENG_JUN "jun"
#define ENG_JUL "jul"
#define ENG_AUG "aug"
#define ENG_SEP "sep"
#define ENG_OCT "oct"
#define ENG_NOV "nov"
#define ENG_DEC "dec"

class Translator
{
private:
    Setting *_lang;
public:
    Translator(Setting *targetLanguage) : _lang(targetLanguage) {}

    const std::string &translate(const std::string &english);
};

#endif
