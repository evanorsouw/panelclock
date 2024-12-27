
#ifndef _UTF8ENCODING_H_
#define _UTF8ENCODING_H_

class UTF8Encoding
{
public:
    static int length(const char *s)
    {
        auto n = 0;
        auto idx = 0;
        while (nextCodepoint(s, idx) != 0)
        {
            n++;
        }
        return n;
    }

    static int nextCodepoint(const char* s, int &idx)
    {
        int codepoint = 0;
        if ((s[idx] & 0x80) == 0x00)
        {   // 1 byte
            codepoint = s[idx]; 
            if (codepoint)
                idx++;
        }
        else
        {   
            auto n = 0;
             if ((s[idx] & 0xF8) == 0xF0)
             {  // 4 byte code
                codepoint = (s[idx++] & 0x07 ); 
                n = 3;
             }
             else if ((s[idx] & 0xF0) == 0xE0)
             {  // 3 byte code
                codepoint = (s[idx++] & 0x0F); 
                n = 2;
             }
             else if ((s[idx] & 0xE0) == 0xC0)
             {  // 2 byte code
                codepoint = (s[idx++] & 0x1F); 
                n = 1;
             }
             while (n-- > 0)
             {
                if ((s[idx] & 0xC0) != 0x80)
                    break;  // invalid UTF8
                codepoint = (codepoint << 6) | s[idx++];
             }
        }
        return codepoint;
    }

    static int getCodepoint(const char *s, int idx)
    {
        return nextCodepoint(s, idx);
    }
};

#endif
