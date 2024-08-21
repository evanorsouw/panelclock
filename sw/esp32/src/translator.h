
#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <settings.h>

class Translator
{
private:
    Setting *_lang;
public:
    Translator(Setting *targetLanguage) : _lang(targetLanguage) {}

    const char *translate(const char *english);
};

#endif