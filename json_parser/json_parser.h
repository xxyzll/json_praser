#ifndef JSON_PRASER_H
#define JSON_PRASER_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <math.h> 
#include <string.h>
#include <map>

namespace JP{
using namespace std;
class VAL;

class Json_Parser{
public:

    //返回值状态定义
    enum PRASE_STATE {
        PRASE_OK = 0,           //解析成功
        PRASE_INVLID_VAL,      //无效的值
        PARSE_ROOT_NOT_SINGULAR,
        PARSE_NUMBER_TOO_BIG,    //解析数字过大
        PARSE_INVALID_STRING_ESCAPE,
        PARSE_MISS_QUOTATION_MARK,
        PARSE_INVALID_STRING_CHAR, //无效的char
        PARSE_INVALID_UNICODE_HEX,   //无效的码点
        PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
        PRASE_MISS_KEY,
        PRASE_MISS_COLON
    } ;
    //类型定义
    enum TYPE {
        JP_NULL,                //空值
        JP_TRUE,                
        JP_FALSE,
        JP_NUMBER,
        JP_STRING,
        JP_ARRAY,
        JP_OBJECT
    };
    // 节点值定义
    class VAL{
        public:
        VAL():t(JP_NULL){}
        TYPE t;
        
        //数组
        vector<VAL> a;
        //double
        double number;
        //string
        string s;
        //obj
        map<string, VAL> obj;
    };

    Json_Parser(){};

    PRASE_STATE operator()(string json_str);
public:
    PRASE_STATE prase_bool(VAL& v);     //解析bool
    PRASE_STATE prase_null(VAL& v);     //解析null
    PRASE_STATE prase_number(VAL& v);   //解析number
    PRASE_STATE prase_string(VAL& v);   //解析string
    PRASE_STATE prase_array(VAL& v);    //解析array
    PRASE_STATE prase_obj(VAL& v);      //解析obj

    string stringify(const VAL& v);     //string化对象
    void stringify_value(const VAL& v, string &s);    //string switch
    void stringify_string(const VAL& v, string &s);   //string化string

    void init(VAL& v);                  //初始化方法
    bool parse_hex4(unsigned &u);       //解析4位码点
    void encode_utf_8(VAL& v, unsigned u);      //编码utf_8


    void jump_space();                              //跳过空白
    double get_number(const VAL& v);                //得到数字
    string get_string(const VAL& v);                //得到string
    int get_string_length(const VAL& v);            //得到长度
    VAL& get_array_element(VAL& v, int idx);
    int get_array_size(const VAL& v);               //得到数组长度
    int get_obj_size(const VAL& v);                 //得到对象size

    TYPE get_type(VAL& v);              //得到类型
    PRASE_STATE prase_json(VAL& v);     //开始解析

    bool has_key(VAL& v, string key);

    string json;            // 需要解析的json字符串
    int cur_prasc_idx;      // 当前解析的idx
    VAL parse_res;          // 解析后的数据

};
}
#endif