#include "leptjson.h"
#include <cassert>
#include <cerrno>
#include <cmath>
#include <iostream>
// #include <cstdlib>

// #define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
// #define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

struct lept_context // 解析上下文
{
    const char *json; // 当前解析json所指向的部分
};

/* ws = *(%x20 / %x09 / %x0A / %x0D) */
/** 吃掉空白符 */
void lept_parse_whitespace(lept_context &c)
{
    const char *p = c.json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c.json = p;
}

/** 解析字面量 null / false / true */
lept_parse_ret lept_parse_literal(lept_context &c, lept_value &v)
{
    switch (*c.json)
    {
    case 'n': /* null  = "null" */
        if (c.json[1] != 'u' || c.json[2] != 'l' || c.json[3] != 'l')
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c.json += 4;
        v.type = LEPT_NULL;
        return LEPT_PARSE_OK;

    case 'f': /* false = "false" */
        if (c.json[1] != 'a' || c.json[2] != 'l' || c.json[3] != 's' || c.json[4] != 'e')
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c.json += 5;
        v.type = LEPT_FALSE;
        v.b = false;
        return LEPT_PARSE_OK;

    case 't': /* true  = "true" */
        if (c.json[1] != 'r' || c.json[2] != 'u' || c.json[3] != 'e')
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c.json += 4;
        v.type = LEPT_TRUE;
        v.b = true;
        return LEPT_PARSE_OK;

    default:
        return LEPT_PARSE_INVALID_VALUE;
    }
}

/** 解析数字 */
lept_parse_ret lept_parse_number(lept_context &c, lept_value &v)
{
    auto ISDIGIT = [=](char ch) { return ch >= '0' && ch <= '9'; };
    auto ISDIGIT1TO9 = [=](char ch) { return ch >= '1' && ch <= '9'; };

    const char *p = c.json;
    if (*p == '-')
        p++;
    if (*p == '0')
        p++;
    else
    {
        if (!ISDIGIT1TO9(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ;
    }
    if (*p == '.')
    {
        p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ;
    }
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
            ;
    }
    errno = 0;
    v.n = strtod(c.json, NULL);
    if (errno == ERANGE && (v.n == HUGE_VAL || v.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v.type = LEPT_NUMBER;
    c.json = p;
    return LEPT_PARSE_OK;
}

const char *lept_parse_hex4(const char *p, uint32_t &u)
{
    u = 0;
    for (int i = 0; i < 4; i++)
    {
        u <<= 4;
        if (*p >= '0' && *p <= '9')
            u += (*p - '0');
        else if (*p >= 'a' && *p <= 'f')
            u += (*p - 'a' + 10);
        else if (*p >= 'A' && *p <= 'F')
            u += (*p - 'A' + 10);
        else
            return NULL;
        p++;
    }
    return p;
}

void lept_encode_utf8(std::vector<char> &c, uint32_t u)
{
    if (u <= 0x7f)
    {
        uint8_t x = 0;
        x |= u;
        c.push_back(x);
    }
    else if (u <= 0x7ff)
    {
        uint8_t x = 0b11000000;
        x |= u >> 6;
        c.push_back(x);

        x = 0b10000000;
        x |= u & 0b00000111111;
        c.push_back(x);
    }
    else if (u <= 0xffff)
    {
        uint8_t x = 0b11100000;
        x |= u >> 12;
        c.push_back(x);

        x = 0b10000000;
        x |= (u >> 6) & 0b0000111111;
        c.push_back(x);

        x = 0b10000000;
        x |= u & 0b0000000000111111;
        c.push_back(x);
    }
    else if (u <= 0x10ffff)
    {
        uint8_t x = 0b11110000;
        x |= u >> 18;
        c.push_back(x);

        x = 0b10000000;
        x |= (u >> 12) & 0b000111111;
        c.push_back(x);

        x = 0b10000000;
        x |= (u >> 6) & 0b000000000111111;
        c.push_back(x);

        x = 0b10000000;
        x |= u & 0b000000000000000111111;
        c.push_back(x);
    }
}

/** 解析字符串 */
lept_parse_ret lept_parse_string(lept_context &c, lept_value &v)
{
    const char *p = c.json;
    std::vector<char> res;

    assert(*p == '"');
    p++;
    while (true)
    {
        switch (*p)
        {
        case '\\':
            p++;
            switch (*p++)
            {
            case '"':
                res.push_back('\"');
                break;
            case '\\':
                res.push_back('\\');
                break;
            case '/':
                res.push_back('/');
                break;
            case 'b':
                res.push_back('\b');
                break;
            case 'f':
                res.push_back('\f');
                break;
            case 'n':
                res.push_back('\n');
                break;
            case 'r':
                res.push_back('\r');
                break;
            case 't':
                res.push_back('\t');
                break;
            case 'u': {
                uint32_t u = 0;
                if (!(p = lept_parse_hex4(p, u)))
                    return LEPT_PARSE_INVALID_UNICODE_HEX;
                if (u >= 0xD800 && u <= 0xDBFF)
                { // surrogate pair
                    if (*p++ != '\\')
                        return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                    if (*p++ != 'u')
                        return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                    uint32_t u2 = 0;
                    if (!(p = lept_parse_hex4(p, u2)))
                        return LEPT_PARSE_INVALID_UNICODE_HEX;
                    if (u2 < 0xDC00 || u2 > 0xDFFF)
                        return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                    u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                }
                lept_encode_utf8(res, u);
                break;
            }
            default:
                return LEPT_PARSE_INVALID_STRING_ESCAPE;
            }
            break;
        case '"':
            c.json = ++p;
            res.push_back('\0');
            v.s = res;
            v.type = LEPT_STRING;
            return LEPT_PARSE_OK;
        case '\0':
            return LEPT_PARSE_MISS_QUOTATION_MARK;
        default:
            if ((unsigned char)*p < 0x20)
            {
                return LEPT_PARSE_INVALID_STRING_CHAR;
            }
            res.push_back(*p++);
        }
    }
}

lept_parse_ret lept_parse_value(lept_context &c, lept_value &v); // 前向声明
/** 解析数组 */
lept_parse_ret lept_parse_array(lept_context &c, lept_value &v)
{
    assert(*c.json == '[');
    c.json++;
    lept_parse_whitespace(c);
    if (*c.json == ']')
    {
        c.json++;
        v = lept_value();
        v.type = LEPT_ARRAY;
        return LEPT_PARSE_OK;
    }

    while (true)
    {
        lept_value e;
        lept_parse_ret ret;

        lept_parse_whitespace(c);
        if ((ret = lept_parse_value(c, e)) != LEPT_PARSE_OK)
            return ret;
        v.a.push_back(e);

        lept_parse_whitespace(c);
        if (*c.json == ',')
        {
            c.json++;
        }
        else if (*c.json == ']')
        {
            c.json++;
            v.type = LEPT_ARRAY;
            return LEPT_PARSE_OK;
        }
        else
            return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
    }
}

/** 解析对象 */
lept_parse_ret lept_parse_object(lept_context &c, lept_value &v)
{
    assert(*c.json == '{');
    c.json++;
    lept_parse_whitespace(c);
    if (*c.json == '}')
    {
        c.json++;
        v = lept_value();
        v.type = LEPT_OBJECT;
        return LEPT_PARSE_OK;
    }

    lept_parse_ret ret = LEPT_PARSE_OK;
    while (true)
    {
        lept_parse_whitespace(c);
        if (*c.json != '"')
            return LEPT_PARSE_MISS_KEY;

        lept_value k;
        ret = lept_parse_string(c, k);
        if (ret != LEPT_PARSE_OK)
            break;

        lept_parse_whitespace(c);
        if (*c.json != ':')
        {
            ret = LEPT_PARSE_MISS_COLON;
            break;
        }
        c.json++;
        lept_parse_whitespace(c);

        lept_value kv;
        ret = lept_parse_value(c, kv);
        if (ret != LEPT_PARSE_OK)
            break;
        v.o.push_back({k, kv});

        lept_parse_whitespace(c);
        if (*c.json == ',')
            c.json++;
        else if (*c.json == '}')
        {
            c.json++;
            break;
        }
        else
            return LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
    }

    return ret;
}

/* value = null / false / true / number / string / array / object */
lept_parse_ret lept_parse_value(lept_context &c, lept_value &v)
{
    switch (*c.json)
    {
    case 'n':
    case 'f':
    case 't':
        return lept_parse_literal(c, v);
    case '\0':
        return LEPT_PARSE_EXPECT_VALUE;
    case '"':
        return lept_parse_string(c, v);
    case '[':
        return lept_parse_array(c, v);
    case '{':
        return lept_parse_object(c, v);
    default:
        return lept_parse_number(c, v);
    }
}

/************************************************************************************************ */

lept_parse_ret lept_parse(lept_value &v, const char *json)
{
    lept_context c; // 定义一个上下文
    c.json = json;

    lept_parse_whitespace(c);
    lept_parse_ret ret = lept_parse_value(c, v);
    if (ret == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(c);
        if (*c.json != '\0')
        {
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

/************************************************************************************************ */

lept_type lept_get_type(const lept_value &v)
{
    return v.type;
}

bool lept_get_boolean(const lept_value &v)
{
    assert(v.type == LEPT_TRUE || v.type == LEPT_FALSE);
    return v.b;
}

void lept_set_boolean(lept_value &v, bool b)
{
    v = lept_value();
    v.type = b ? LEPT_TRUE : LEPT_FALSE;
    v.b = b;
}

double lept_get_number(const lept_value &v)
{
    assert(v.type == LEPT_NUMBER);
    return v.n;
}

void lept_set_number(lept_value &v, double n)
{
    v = lept_value();
    v.type = LEPT_NUMBER;
    v.n = n;
}

const char *lept_get_string(const lept_value &v)
{
    assert(v.type == LEPT_STRING);
    return v.s.data();
}

void lept_set_string(lept_value &v, const char *s)
{
    v = lept_value();
    v.type = LEPT_STRING;
    v.s.clear();
    for (int i = 0;; i++)
    {
        v.s.push_back(s[i]);
        if (s[i] == '\0')
            break;
    }
}

const lept_value &lept_get_array_element(const lept_value &v, std::vector<lept_value>::size_type index)
{
    assert(v.type == LEPT_ARRAY);
    assert(index < v.a.size());
    return v.a[index];
}

const lept_value &lept_get_object_value(const lept_value &v, const lept_value &k)
{
    // TODO
    return v.o[0].first;
}