
#include <algorithm>
#include <cstring>
#include <string>
#include <stdio.h>

#include "font.h"
#include "truetypefont.h"
#include "bitmapfont.h"

std::map<std::string, Font*> Font::_loadedFonts;

Font *Font::getFont(const char *fontname, float dx, float dy)
{
    char fullname[80];
    sprintf(fullname, "%s:%.2f:%.2f", fontname, dx, dy);

    auto  it = _loadedFonts.find(fullname);
    Font *font = nullptr;
    if (it == _loadedFonts.end())
    {
        auto extension = std::strrchr(fontname, '.');
        if (extension == nullptr || std::strcmp(extension, ".ttf") == 0)
        {
            font = TrueTypeFont::getFont(fontname, dx, dy);
            if (font != nullptr)
            {
                _loadedFonts[fullname] = font;
                it = _loadedFonts.find(fullname);
            }
        }
        else if (std::strcmp(extension, ".wmf") == 0)
        {
            font = BitmapFont::getFont(fontname); 
            if (font != nullptr)
            {
                _loadedFonts[fontname] = font;
                it = _loadedFonts.find(fontname);
            }
        }
    }
    else
    {
        font = it->second;
    }
    return font;
}
