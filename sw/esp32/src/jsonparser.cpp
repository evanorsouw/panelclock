
#include <stdio.h>
#include <cstring>
#include <cctype>
#include "jsonparser.h"

JsonParser::JsonParser(std::function<bool(const JsonEntry &)> handler)
{
    _handler = handler;
    _tokenstate = TokenState::Idle;
    _parsestate = ParseState::Idle;
    _finished = false;
    _name.set("");
}

bool JsonParser::parse(const char *buf, int len)
{
    if (!_finished)
    {
        if (len == 0)
        {
            _finished = true;
            if (_parsestate != ParseState::Idle || _nestdepth.empty())
            {
                finishWithError("premature end of stream");
            }
        }
        else
        {
            _buf = buf;
            _buflen = len;
            _ihead = 0;
            parse();
        }
    }
     return _finished;
}

void JsonParser::parse()
{
    while (!eof() && !_finished)
    {
        auto token = nextToken();
        if (token == JsonToken::NeedMore)
            continue;

        switch (_parsestate)
        {
        case ParseState::Idle:
            switch (token)
            {
                case JsonToken::Bool:
                    finishWithItem(JsonItem::Bool);
                    break;
                case JsonToken::Number:
                    finishWithItem(JsonItem::Number);
                    break;
                case JsonToken::Null:
                    finishWithItem(JsonItem::Null);
                    break;
                case JsonToken::String:
                    finishWithItem(JsonItem::Bool);
                    break;
                case JsonToken::StartArray:
                    _parsestate = ParseState::ValueOrEnd;
                    _nestdepth.push_back(true);
                    returnItem(JsonItem::Array);
                    break;
                case JsonToken::StartObject:
                    _parsestate = ParseState::NameOrEnd;
                    _nestdepth.push_back(false);
                    returnItem(JsonItem::Object);
                    break;
                default:
                    finishWithUnexpectedToken(token);
                    break;
            }
            break;
        case ParseState::Name:
        case ParseState::NameOrEnd:
            switch (token)
            {
                case JsonToken::String:
                    _name.copyfrom(_string);
                    _parsestate = ParseState::Colon;
                    break;
                case JsonToken::EndArray:
                case JsonToken::EndObject:
                    if (_parsestate == ParseState::NameOrEnd && !_nestdepth.empty() && _nestdepth.back() == (token == JsonToken::EndArray))
                    {
                        returnItem(JsonItem::End);
                        _parsestate = ParseState::Value;
                    }
                    else
                    {
                        finishWithUnexpectedToken(token);
                    }
                    break;
                default:
                    finishWithUnexpectedToken(token);
                    break;
            }
            break;
        case ParseState::Colon:
            switch (token)
            {
            case JsonToken::Colon:
                _parsestate = ParseState::Value;
                break;
            default:
                finishWithUnexpectedToken(token);
                break;
            }
            break;
        case ParseState::NextElementOrEnd:
            switch (token)
            {
                case JsonToken::NextElement:
                    _parsestate = _nestdepth.back() ? ParseState::Value : ParseState::Name;
                    break;
                case JsonToken::EndArray:
                case JsonToken::EndObject:
                    if (!_nestdepth.empty() && _nestdepth.back() == (token == JsonToken::EndArray))
                    {
                        _nestdepth.pop_back();
                        returnItem(JsonItem::Close);
                        if (_nestdepth.empty())
                        {
                            finishWithItem(JsonItem::End);
                        }
                        else
                        {
                            _parsestate = ParseState::NextElementOrEnd;
                        }
                    }
                    else
                    {
                        finishWithUnexpectedToken(token);
                    }
                    break;
                default:
                    finishWithUnexpectedToken(token);
                    break;
            }
            break;
        case ParseState::Value:
        case ParseState::ValueOrEnd:
            switch (token)
            {
                case JsonToken::Null:
                    returnItem(JsonItem::Null);
                    _parsestate = ParseState::NextElementOrEnd;
                    break;
                case JsonToken::Bool:
                    returnItem(JsonItem::Bool);
                    _parsestate = ParseState::NextElementOrEnd;
                    break;
                case JsonToken::Number:
                    returnItem(JsonItem::Number);
                    _parsestate = ParseState::NextElementOrEnd;
                    break;
                case JsonToken::String:
                    returnItem(JsonItem::String);
                    _parsestate = ParseState::NextElementOrEnd;
                    break;
                case JsonToken::StartArray:
                    _nestdepth.push_back(true);
                    returnItem(JsonItem::Array);
                    _parsestate = ParseState::ValueOrEnd;
                    break;
                case JsonToken::StartObject:
                    _nestdepth.push_back(false);
                    returnItem(JsonItem::Object);
                    _parsestate = ParseState::NameOrEnd;
                    break;
                case JsonToken::EndArray:
                case JsonToken::EndObject:
                    if (!_nestdepth.empty() && _nestdepth.back() == (token == JsonToken::EndArray))
                    {
                        _nestdepth.pop_back();
                        returnItem(JsonItem::Close);
                        if (_nestdepth.empty())
                        {
                            finishWithItem(JsonItem::End);
                        }
                        else
                        {
                            _parsestate = ParseState::NextElementOrEnd;
                        }
                    }
                    else
                    {
                        finishWithUnexpectedToken(token);
                    }
                    break;
                default:
                    finishWithUnexpectedToken(token);
                    break;
            }
            break;
        }
    }
}

JsonToken JsonParser::nextToken()
{
    switch (_tokenstate)
    {
        case TokenState::Idle:
            skipWhites();
            switch (next())
            {
                case ':': return JsonToken::Colon;
                case '{': return JsonToken::StartObject;
                case '}': return JsonToken::EndObject;
                case '[': return JsonToken::StartArray;
                case ']': return JsonToken::EndArray;
                case ',': return JsonToken::NextElement;
                case '"': return parseStringToken();
                case 'f':
                case 't':
                case 'n':
                    back();
                    return parseLiteralToken();
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    back();
                    return parseNumberToken();
                case 0:
                    return JsonToken::NeedMore;
                default:
                    back();
                    return JsonToken::Error;
            }
            break;
        case TokenState::ParseString:
            return parseStringToken();
        case TokenState::ParseNumber:
            return parseNumberToken();
        case TokenState::ParseLiteral:
            return parseLiteralToken();
    }
    return JsonToken::Error;
}

void JsonParser::skipWhites()
{
    for (;;)
    {
        switch (head())
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            next();
            break;
        default:
            return;
        }
    }
}

JsonToken JsonParser::parseStringToken()
{
    if (_tokenstate == TokenState::Idle)
    {
        _tokenstate = TokenState::ParseString;
        _string.clear();
    }  

    while (!eof() && head() != '"')
    {
        _string.add(next());
    }

    if (eof())
        return JsonToken::NeedMore;

    next();
    _string.add(0);
    _tokenstate = TokenState::Idle;
    return JsonToken::String;
}

JsonToken JsonParser::parseNumberToken()
{
    if (_tokenstate == TokenState::Idle)
    {
        _tokenstate = TokenState::ParseNumber;
        _numberstate = NumberState::Sign;
        _currentToken.number = 0.0;
        _negative = false;
    }  

    while (!eof() && _numberstate != NumberState::Done)
    {
        switch (_numberstate)
        {
        case NumberState::Sign:
            if (head() == '-' || head() == '+')
            {
                _negative = next() == '-';
            }
            _numberstate = NumberState::Integer1;
            break;
        case NumberState::Integer1:
            if (std::isdigit(head()))
            {
                _currentToken.number = next() - '0';
                _numberstate = NumberState::Integer;
            }
            else
            {
                _numberstate = NumberState::Done;
            }
            break;
        case NumberState::Integer:
            if (head() == '.')
            {
                next();
                _numberstate = NumberState::Fraction1;
                _digitscale = 0.1f;
            }
            else if (std::isdigit(head()))
            {
                _currentToken.number *= 10;
                _currentToken.number += next() - '0';
            }
            else
            {
                _numberstate = NumberState::Done;
            }
            break;
        case NumberState::Fraction1:
        case NumberState::Fraction:
            if (std::isdigit(head()))
            {
                _currentToken.number += (next() - '0') * _digitscale;
                _digitscale /= 10.0f;
                _numberstate = NumberState::Fraction;
            }
            else if (_numberstate == NumberState::Fraction1)
            {
                // need at least 1 digit after the .
                return JsonToken::Error;
            }
            else
            {
                _numberstate = NumberState::Done;
            }
            break;
        case NumberState::Done:
            break;
        }
    }
     
    if (_numberstate == NumberState::Done)
    {
        _tokenstate = TokenState::Idle;
        return JsonToken::Number;
    }
    return JsonToken::NeedMore;
}

JsonToken JsonParser::parseLiteralToken()
{
    if (_tokenstate == TokenState::Idle)
    {
        _tokenstate = TokenState::ParseLiteral;
        _string.clear();
    }  

    while (std::isalnum(head()))
    {
        _string.add(next());
    }
    if (eof())
        return JsonToken::NeedMore;

    if (!strcmp(_currentToken.string, "true"))
    {
        _currentToken.boolean = true;
        _tokenstate = TokenState::Idle;
        return JsonToken::Bool;
    }
    if (!strcmp(_currentToken.string, "false"))
    {
        _currentToken.boolean = false;
        _tokenstate = TokenState::Idle;
        return JsonToken::Bool;
    }
    if (!strcmp(_currentToken.string, "null"))
    {
        _tokenstate = TokenState::Idle;
        return JsonToken::Null;
    }
    return JsonToken::Error;
}

void JsonParser::finishWithUnexpectedToken(JsonToken token)
{
    char buf[80];
    snprintf(buf, sizeof(buf), "unexpected token=%d", (int)token);
    finishWithError(buf);
}

void JsonParser::finishWithError(const char *msg)
{
    _string.set(msg);
    finishWithItem(JsonItem::Error);
}

void JsonParser::finishWithItem(JsonItem item)
{
    _finished = true;
    returnItem(item);
}

void JsonParser::returnItem(JsonItem type)
{
    _currentToken.item = type;
    _currentToken.name = (const char *)_name.data();
    _currentToken.string = (const char *)_string.data();
    _handler(_currentToken);

    // todo: use skip returnvalue from previous call to skip remainder of current object.

    _name.set("");
}
