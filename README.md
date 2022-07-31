# c++ json解释器
## 使用方法

```c++
#include "json_parser/json_parser.h"

Json_Parser parser;
//json为需要解析的字符串, 解析结果保存在parser_res;
parser(json);
```
- 对象保存在obj
- string保存在s
- 数字为double保存在number
- 数组保存在a

可以参考节点值的定义：
```c++
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

// t用于判断当前的有效元素
enum TYPE {
    JP_NULL,   //空值
    JP_TRUE,   //bool值true            
    JP_FALSE,  //bool值false
    JP_NUMBER, //数字
    JP_STRING, //字符串
    JP_ARRAY,  //数组
    JP_OBJECT  //对象
};
```
由于上述设计，在获得某个具体的值时不能直接调用，需要通过类型t判断当前节点的有效值，然后再读取，例如：
```c++
    JP::Json_Parser parser;
    parser("\"this is test string\"");
```
你将会得到：

![alt](/images/parser_res.jpg)

读取值的时候务必要判断t的类型然后再读取。对象和数组也是相同
