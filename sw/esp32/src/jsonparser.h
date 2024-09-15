
#ifndef _JSONPARSER_H_
#define _JSONPARSER_H_

#include <vector>
#include <functional>
#include "buffer.h"

enum class JsonToken { StartObject, EndObject, StartArray, EndArray, Colon, Number, String, Bool, Null, NextElement, NeedMore, Error };
enum class TokenState { Idle, ParseString, ParseNumber, ParseLiteral };
enum class ParseState { Idle, NameOrEnd, ValueOrEnd, NextElementOrEnd, Name, Colon, Value };
enum class NumberState { Sign, Integer1, Integer, Fraction1, Fraction, Done };
enum class JsonItem { Null, Bool, Number, String, Object, Array, Close, End, Error };

struct JsonEntry
{
    const char *name;
    JsonItem item;
    double number;
    const char *string;
    bool boolean;
};

class JsonParser
{
private:
    std::function<bool(const JsonEntry &)> _handler;
    const char *_buf;
    int _buflen;
    int _ihead;
    TokenState _tokenstate;
    ParseState _parsestate;
    NumberState  _numberstate;
    std::vector<bool> _nestdepth;
    JsonEntry _currentToken;
    Buffer _name;
    Buffer _string;
    bool _negative;
    double _digitscale;
    bool _finished;
            
public:
    /**
     * @brief Construct a new JsonParser using the provided handler
     * 
     * @param handler lambda that is called for each received Json object/array.
     * The handler returns true if we are interested in nested values of this
     * object/array and false if not. When returning false, nested values of 
     * the array/object are no provided, but siblings and higher levels still are. 
     * The return value has no meaning for booleans, numbers, strings or null values, 
     * but only for objects and arrays. When the handler returns false on the start
     * of an object/array (based on name) then no subsequent close is given to
     * the handler. When the handler only returns false after accepting the 
     * object/array, an associated close will be passed.
     * When the passed object returns an error code, the string property contains
     * a human readable description of the reason. 
     * An error is the last call that will be made to the handler
     */
    JsonParser(std::function<bool(const JsonEntry &)> handler);

    /**
     * @brief call each time when new json data arrives.
     * When 0 is passed for the length it is an indication that the json stream 
     * finished. After that, this object is no longer usable.
     * 
     * @param buf the buffer that contains (part of) the json data.
     * @param len the length of the data, 0 when there is nothing more.
     * @return false when the parser needs more data and true when it is completed.
     * A completed parser will ignore any more passed data.
     */
    bool parse(const char *buf, int len);

private:
    void parse();
    void skipWhites();
    bool eof() const { return _ihead == _buflen; }
    char head() const { return eof() ? 0 : _buf[_ihead]; }
    char next() { return eof() ? 0 : _buf[_ihead++]; }
    void back() { _ihead = (_ihead == 0 ? 0 : _ihead - 1); }

    JsonToken nextToken();
    JsonToken parseStringToken();
    JsonToken parseNumberToken();
    JsonToken parseLiteralToken();

    void finishWithUnexpectedToken(JsonToken token);
    void finishWithError(const char *msg);
    void finishWithItem(JsonItem item);
    void returnItem(JsonItem item);
};

#endif
