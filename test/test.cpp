#include "leptjson.h"
#include <cmath>
#include <cstring>
#include <iostream>

int main_ret = 0;
int test_count = 0;
int test_pass = 0;

template <typename T> void expect_eq_base(bool equality, const T &expect, const T &actual, const char *file, int line)
{
    test_count++;
    if (equality)
        test_pass++;
    else
    {
        std::cerr << file << ':' << line << ": expect: " << expect << " actual: " << actual << '\n';
        main_ret = 1;
    }
}

template <typename T> void expect_eq(const T &expect, const T &actual, const char *file, int line)
{
    expect_eq_base(expect == actual, expect, actual, file, line);
}
template <> void expect_eq(const double &expect, const double &actual, const char *file, int line)
{
    auto x = std::abs(expect - actual);
    expect_eq_base(x <= 1e-5, expect, actual, file, line);
}
template <> void expect_eq(const float &expect, const float &actual, const char *file, int line)
{
    auto x = std::abs(expect - actual);
    expect_eq_base(x <= 1e-5, expect, actual, file, line);
}
void expect_eq(const char *expect, const char *actual, const char *file, int line) // 重载函数实现字符串比较
{
    expect_eq_base(0 == strcmp(expect, actual), expect, actual, file, line);
}

#define EXPECT_EQ(expect, actual) expect_eq(expect, actual, __FILE__, __LINE__)

#define TEST_ERROR(error, json)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        lept_value v;                                                                                                  \
        EXPECT_EQ(error, lept_parse(v, json));                                                                         \
    } while (0)

#define TEST_NUMBER(expect, json)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        lept_value v;                                                                                                  \
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, json));                                                                 \
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(v));                                                                      \
        EXPECT_EQ(expect, lept_get_number(v));                                                                         \
    } while (0)

#define TEST_STRING(expect, json)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        lept_value v;                                                                                                  \
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, json));                                                                 \
        EXPECT_EQ(LEPT_STRING, lept_get_type(v));                                                                      \
        EXPECT_EQ(expect, lept_get_string(v));                                                                         \
    } while (0)

/************************************************************************************** */

/** 测试与null相关的内容 */
void test_parse_null()
{
#if 1 // 解析null值
    {
        lept_value v;
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "null"));
        EXPECT_EQ(LEPT_NULL, lept_get_type(v));
    }
#endif

#if 1 // 访问null值
    {
        lept_value v;
        EXPECT_EQ(LEPT_NULL, lept_get_type(v));
    }
#endif
}

/** 测试与true相关的内容 */
void test_parse_true()
{
    lept_value v;
    lept_set_boolean(v, false);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "true"));
    EXPECT_EQ(LEPT_TRUE, lept_get_type(v));
    EXPECT_EQ(true, lept_get_boolean(v));
}

/** 测试与false相关的内容 */
void test_parse_false()
{
    lept_value v;
    lept_set_boolean(v, true);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "false"));
    EXPECT_EQ(LEPT_FALSE, lept_get_type(v));
    EXPECT_EQ(false, lept_get_boolean(v));
}

void test_parse_error_value()
{
#if 1 // 空白
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
#endif

#if 1 // 无效
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");
#endif

#if 1 // 字符 空白 之后还有字符
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
#endif
}

/** 测试与number相关的内容 */
void test_parse_number()
{
#if 1 // 常规测试
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
#endif

#if 1                                                                // 边界值测试
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");           /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308"); /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
#endif

#if 1 /* invalid number */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
#endif

#if 1                                                 /* invalid number */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
#endif

#if 1 // test_parse_number_too_big
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif

#if 1 // 访问number
    lept_value v;
    lept_set_number(v, -0.12);
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "12.788"));
    EXPECT_EQ(12.788, lept_get_number(v));
#endif
}

void test_parse_string()
{
#if 1 // 常规测试
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#endif

#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif

#if 1
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif

#if 1 // test_parse_invalid_string_escape
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
#endif

#if 1 // test_parse_invalid_string_char
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
#endif

#if 1 // 访问字符串
    lept_value v;
    EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "\"Hello world\\n\""));
    EXPECT_EQ(0, strcmp("Hello world\n", lept_get_string(v)));
#endif

#if 1 // test_parse_invalid_unicode_hex
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
#endif

#if 1 // test_parse_invalid_unicode_surrogate
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
#endif
}

void test_parse_array()
{
#if 1
    {
        lept_value v;
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "[ ]"));
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(v));
        EXPECT_EQ((std::vector<lept_value>::size_type)0, v.a.size());
    }
#endif

#if 1
    {
        lept_value v;
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "[ null , false , true , 123 , \"abc\" ]"));
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(v));

        const lept_value &e0 = lept_get_array_element(v, 0);
        EXPECT_EQ(LEPT_NULL, lept_get_type(e0));

        const lept_value &e1 = lept_get_array_element(v, 1);
        EXPECT_EQ(LEPT_FALSE, lept_get_type(e1));
        EXPECT_EQ(false, lept_get_boolean(e1));

        const lept_value &e2 = lept_get_array_element(v, 2);
        EXPECT_EQ(LEPT_TRUE, lept_get_type(e2));
        EXPECT_EQ(true, lept_get_boolean(e2));

        const lept_value &e3 = lept_get_array_element(v, 3);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e3));
        EXPECT_EQ(123., lept_get_number(e3));

        const lept_value &e4 = lept_get_array_element(v, 4);
        EXPECT_EQ(LEPT_STRING, lept_get_type(e4));
        EXPECT_EQ("abc", lept_get_string(e4));
    }
#endif

#if 1
    {
        lept_value v;
        EXPECT_EQ(LEPT_PARSE_OK, lept_parse(v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(v));

        const lept_value &e0 = lept_get_array_element(v, 0);
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(e0));
        EXPECT_EQ((std::vector<lept_value>::size_type)0, e0.a.size());

        const lept_value &e1 = lept_get_array_element(v, 1);
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(e1));
        EXPECT_EQ((std::vector<lept_value>::size_type)1, e1.a.size());
        const lept_value &e1e0 = lept_get_array_element(e1, 0);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e1e0));
        EXPECT_EQ(0., lept_get_number(e1e0));

        const lept_value &e2 = lept_get_array_element(v, 2);
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(e2));
        EXPECT_EQ((std::vector<lept_value>::size_type)2, e2.a.size());
        const lept_value &e2e0 = lept_get_array_element(e2, 0);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e2e0));
        EXPECT_EQ(0., lept_get_number(e2e0));
        const lept_value &e2e1 = lept_get_array_element(e2, 1);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e2e1));
        EXPECT_EQ(1., lept_get_number(e2e1));

        const lept_value &e3 = lept_get_array_element(v, 3);
        EXPECT_EQ(LEPT_ARRAY, lept_get_type(e3));
        EXPECT_EQ((std::vector<lept_value>::size_type)3, e3.a.size());
        const lept_value &e3e0 = lept_get_array_element(e3, 0);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e3e0));
        EXPECT_EQ(0., lept_get_number(e3e0));
        const lept_value &e3e1 = lept_get_array_element(e3, 1);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e3e1));
        EXPECT_EQ(1., lept_get_number(e3e1));
        const lept_value &e3e2 = lept_get_array_element(e3, 2);
        EXPECT_EQ(LEPT_NUMBER, lept_get_type(e3e2));
        EXPECT_EQ(2., lept_get_number(e3e2));
    }
#endif
}

void test_parse_object()
{
#if 1
    TEST_ERROR(LEPT_PARSE_OK, "{}");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(LEPT_PARSE_MISS_KEY, "{{}:1,");
    // TEST_ERROR(LEPT_PARSE_MISS_KEY, "{\"a\":1,");
#endif

#if 1
    TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}");
#endif

#if 1
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
#endif
}

/************************************************************************************** */

void test_parse()
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_error_value();

    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}