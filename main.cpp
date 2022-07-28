#include <iostream>
#include "json_praser/json_praser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_NUMBER(expect, json)\
    do {\
        Json_Praser op;\
        EXPECT_EQ_INT(Json_Praser::PRASE_OK, op(json));\
        EXPECT_EQ_INT(Json_Praser::JP_NUMBER, op.get_type(op.prase_res));\
        EXPECT_EQ_DOUBLE(expect, op.get_number(op.prase_res));\
    } while(0)

#define TEST_ERROR(error, json)\
    do {\
        Json_Praser op;\
        op.prase_res.t = Json_Praser::JP_NULL;\
        EXPECT_EQ_INT(error, op(json));\
        EXPECT_EQ_INT(Json_Praser::JP_NULL, op.get_type(op.prase_res));\
    } while(0)

#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

#define TEST_STRING(expect, json)\
    do {\
        Json_Praser op;\
        op.init();\
        EXPECT_EQ_INT(Json_Praser::PRASE_OK, op(json));\
        EXPECT_EQ_INT(Json_Praser::JP_STRING, op.get_type(op.prase_res));\
        EXPECT_EQ_STRING(expect, op.get_string(op.prase_res), op.get_string_length(op.prase_res));\
    } while(0)

static void test_parse_expect_value();
static void test_parse_root_not_singular();
static void test_parse_invalid_value();
static void test_parse_number();
static void test_parse();
static void test_parse_number_too_big();
static void test_parse_string();
static void test_parse_invalid_string_escape();
static void test_parse_invalid_string_char();


int main(int, char**) {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}



static void test_parse_expect_value() {
    using namespace JP;
    Json_Praser op;
    EXPECT_EQ_INT(Json_Praser::PRASE_OK, op("true"));
    EXPECT_EQ_INT(Json_Praser::PRASE_OK, op("false"));
    EXPECT_EQ_INT(Json_Praser::PRASE_OK, op("null"));
}

static void test_parse_root_not_singular() {
    using namespace JP;
    Json_Praser op;
    EXPECT_EQ_INT(Json_Praser::PARSE_ROOT_NOT_SINGULAR, op("true x"));
    EXPECT_EQ_INT(Json_Praser::PARSE_ROOT_NOT_SINGULAR, op("false 1"));
    EXPECT_EQ_INT(Json_Praser::PARSE_ROOT_NOT_SINGULAR, op("null 1"));

     /* invalid number */
    TEST_ERROR(Json_Praser::PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(Json_Praser::PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(Json_Praser::PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_invalid_value() {
    using namespace JP;
    Json_Praser op;
    EXPECT_EQ_INT(Json_Praser::PRASE_INVLID_VAL, op("nul"));
    EXPECT_EQ_INT(Json_Praser::PRASE_INVLID_VAL, op("?"));

    /* invalid number */
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "+0");
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "+1");
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, ".123"); /* at least one digit before '.' */
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "1.");   /* at least one digit after '.' */
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "INF");
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "inf");
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "NAN");
    TEST_ERROR(Json_Praser::PRASE_INVLID_VAL, "nan");
}

static void test_parse() {
    test_parse_expect_value();
    test_parse_root_not_singular();
    test_parse_invalid_value();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();

    test_parse_number();
    test_parse_number_too_big();
    test_parse_string();
}

static void test_parse_number() {
    using namespace JP;
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

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_number_too_big() {
    using namespace JP;
    TEST_ERROR(Json_Praser::PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(Json_Praser::PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_string() {
    using namespace JP;
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); 
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); 
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); 
}

static void test_parse_invalid_string_escape() {
    using namespace JP;
    TEST_ERROR(Json_Praser::PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(Json_Praser::PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(Json_Praser::PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(Json_Praser::PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    using namespace JP;
    TEST_ERROR(Json_Praser::PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(Json_Praser::PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}