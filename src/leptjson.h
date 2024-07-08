#pragma once

// #include <cstddef>
#include <unordered_map>
#include <vector>

/** 解析值的类型 */
enum lept_type
{
    LEPT_NULL,   // NULL
    LEPT_FALSE,  // false
    LEPT_TRUE,   // true
    LEPT_NUMBER, // 数字
    LEPT_STRING, // 字符串
    LEPT_ARRAY,  // 数组
    LEPT_OBJECT, // 对象
};

/** 解析函数返回值 */
enum lept_parse_ret
{
    LEPT_PARSE_OK,                    // 解析成功
    LEPT_PARSE_EXPECT_VALUE,          // 只有空白字符
    LEPT_PARSE_INVALID_VALUE,         // 无效值
    LEPT_PARSE_ROOT_NOT_SINGULAR,     // 一个值后跟一段空白后再跟一个值
    LEPT_PARSE_NUMBER_TOO_BIG,        // 数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK,   // 缺失成对的双引号
    LEPT_PARSE_INVALID_STRING_ESCAPE, // 非法转义
    LEPT_PARSE_INVALID_STRING_CHAR,   // 非法字符
    LEPT_PARSE_INVALID_UNICODE_SURROGATE, // 只有高代理项而欠缺低代理项，或是低代理项不在合法码点范围
    LEPT_PARSE_INVALID_UNICODE_HEX,          // \u 后不是 4 位十六进位数字
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, // 数组没有包围的括号
    LEPT_PARSE_MISS_KEY,                     // 没有key
    LEPT_PARSE_MISS_COLON,                   // 没有冒号
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET,  // 没有预期的逗号或花括号
};

struct lept_value
{
    std::vector<char> s;                              // 字符串
    std::vector<lept_value> a;                        // 数组
    double n;                                         // 数字
    bool b;                                           // 布尔值
    std::vector<std::pair<lept_value, lept_value>> o; //对象

    lept_type type = LEPT_NULL;
};

lept_parse_ret lept_parse(lept_value &v, const char *json); // 解析json文本

lept_type lept_get_type(const lept_value &v); // 获取解析值的类型

bool lept_get_boolean(const lept_value &v); // 获取布尔值
void lept_set_boolean(lept_value &v, bool b);

double lept_get_number(const lept_value &v); // 获取数值类型的值
void lept_set_number(lept_value &v, double n);

const char *lept_get_string(const lept_value &v); // 获取字符串
void lept_set_string(lept_value &v, const char *s);

const lept_value &lept_get_array_element(const lept_value &v, std::vector<lept_value>::size_type index); // 获取数组元素

const lept_value &lept_get_object_value(const lept_value &v, const lept_value &k); // 获取对象值