
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <esp_heap_caps.h>

#include "bitmapfont.h"

#if 0
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

BitmapFont::~BitmapFont()
{
    heap_caps_free(_filedata);
    _filedata = nullptr;
    printf("unloaded font='%s' from memory\n", _fontname);
    delete _fontname;
}

BitmapFont *BitmapFont::getFont(const char *fontname)
{
    BitmapFont *font = nullptr;
    FILE *fp = nullptr;
    void *memory = nullptr;

    printf("loading font='%s'", fontname);
    do
    {
        auto path = std::string("/spiffs/") + fontname;
        struct stat info;
        if (stat(path.c_str(), &info) < 0)
        {
            printf(" - not found\n");
            break;
        }
        auto memory = heap_caps_malloc_prefer(info.st_size, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT);
        if (memory == nullptr)
        {
            printf(" - not enough memory\n");
            break;
        }
        fp = fopen(path.c_str(), "rb");
        if (fp == nullptr)
        {
            printf(" - cannot open\n");
            break;
        }
        auto n = fread(memory, 1, info.st_size, fp);
        if (n != info.st_size)
        {
            printf(" - failed to read all %ld bytes\n", info.st_size);
            break;
        }
        font = loadFont(fontname, memory, (int)info.st_size);

    } while (false);

    if (fp != nullptr)
    {
        fclose(fp);
    }
    if (font == nullptr && memory != nullptr)
    {
        free(memory);
    }
    printf("\n");
    return font;
}

textinfo BitmapFont::textsize(const char *txt, int n) const
{
    textinfo size(0, height());
    if (n == 0) n = -1;
    while (*txt && n-- != 0)
    {
        auto csize = charsize(*txt++);
        if (size.dx > 0)
            size.dx += _info->tracking;
        size.dx += csize.dx;
    }
    return size;
}

textinfo BitmapFont::charsize(char c) const
{
    auto it = std::find_if(_glyphs.begin(), _glyphs.end(), [=](std::pair<uint16_t, glyphInfo*> kv) { return kv.first == (uint16_t)c; });
    if (it == _glyphs.end())
        return textinfo();
    return textinfo(it->second->width, height());
}

BitmapFont *BitmapFont::loadFont(const char *fontname, void *filedata, int filesize)
{
    auto info = (bitmapfont*)filedata;
    fromBE(info->glyphcount);
    fromBE(info->offsetDescription);
    fromBE(info->offsetGlyphTable);
    
    if (info->version != 1)
    {
        printf(" - version='%d' not supported\n", info->version);
        return nullptr;
    }
    printf(" glyphcount='%d'", info->glyphcount);

    auto pglyph = (glyphInfo *)((uint8_t *)filedata + info->offsetGlyphTable);
    
    uint8_t maxWidth = 0;
    uint8_t maxHeight = 0;
    std::map<uint32_t, glyphInfo*> glyphs;
    for (int i=0; i<info->glyphcount; ++i)
    {
        fromBE(pglyph->unicode);
        glyphs[pglyph->unicode] = pglyph;
        maxWidth = std::max(maxWidth, pglyph->width);
        maxHeight = std::max(maxHeight, pglyph->height);
        auto size = sizeof(glyphInfo) + ((pglyph->width + 7) / 8) * pglyph->height - 1;
        LOG("-> '%c'/'%d' %d bytes\n", pglyph->unicode, pglyph->unicode, size);
        pglyph = (glyphInfo*)((uint8_t*)pglyph + size);
    }

    auto font = new BitmapFont();
    font->_fontname = fontname;
    font->_filedata = filedata;
    font->_info = info;
    font->_maxWidth = maxWidth;
    font->_maxHeight = maxHeight;
    font->_glyphs = std::move(glyphs);

    printf(" (size=%d, @%p)\n", filesize, filedata);

    return font;
}

void BitmapFont::fromBE(uint16_t &word)
{
    uint16_t bl = 0x1234;
    if (((uint8_t *)&bl)[0] == 0x34)
    {
        word = ((word & 0xFF) << 8) | ((word >> 8) & 0xFF);
    }
}

BitmapFont::glyphInfo* BitmapFont::getGlyph(uint32_t codepoint) const
{
    auto it = std::find_if(_glyphs.begin(), _glyphs.end(), [=](std::pair<uint16_t, glyphInfo *> kv) { return kv.first == codepoint; });
    if (it == _glyphs.end())
        return nullptr;
    return it->second;
}
