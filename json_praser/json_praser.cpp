#include "json_praser.h"

namespace JP{
    #define ISDIGIT1TO9(ch) ((ch)>= '1'&& (ch)<= '9')
    #define ISDIGIT(ch) ((ch)>= '0'&& (ch)<= '9')

    bool Json_Praser::has_key(VAL& v, string key){
        assert(v.t == JP_OBJECT);
        return v.obj.find(key) != v.obj.end();
    }

    string Json_Praser::get_string(const VAL& v){
        assert(v.t == JP_STRING);
        return v.s;
    }

    int Json_Praser::get_obj_size(const VAL& v){
        assert(v.t == JP_OBJECT);
        return v.obj.size();
    }

    int Json_Praser::get_string_length(const VAL& v){
        assert(v.t == JP_STRING);
        return v.s.size();
    }

    void Json_Praser::stringify_string(VAL const& v, string& s){
        static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        s.push_back('"');
        for (int i = 0; i < v.s.size(); i++) {
            unsigned char ch = (unsigned char)v.s[i];
            switch (ch) {
                case '\"': s += "\\\""; break;
                case '\\': s += "\\\\"; break;
                case '\b': s += "\\b";  break;
                case '\f': s += "\\f";  break;
                case '\n': s += "\\n";  break;
                case '\r': s += "\\r";  break;
                case '\t': s += "\\t";  break;
                default:
                    if (ch < 0x20) {
                        s += "\\u00";
                        s += hex_digits[ch >> 4];
                        s += hex_digits[ch & 15];
                    }
                    else
                        s += v.s[i];
            }
        }
        s += '"';
    }

    void Json_Praser::stringify_value(const VAL& v, string& s){
        switch(v.t){
            case JP_NULL: s += "null"; break;
            case JP_FALSE:  s += "false"; break;
            case JP_TRUE:  s += "true"; break;
            case JP_NUMBER: s += to_string(get_number(v)); break;
            case JP_STRING: stringify_string(v, s); break;
            case JP_ARRAY:
                s += '[';
                for (int i = 0; i < v.a.size(); i++) {
                    if (i > 0)
                        s += ',';
                    stringify_value(v, s);
                }
                s += ']';
            break;
            case JP_OBJECT:
                s += '{';
                for (auto it=v.obj.begin(); it != v.obj.end(); it++) {
                    if (it != v.obj.begin())
                        s += ',';
                    VAL e;
                    init(e);
                    e.t = JP_STRING;
                    e.s = it->first;
                    stringify_string(e, s);
                    s += ':';
                    stringify_value(it->second, s);
                }
                s += '}';
            break;
        }
    }

    string Json_Praser::stringify(const VAL& v){
        string ret;
        stringify_value(v, ret);
        return ret;
    }

    Json_Praser::PRASE_STATE Json_Praser::operator()(string json_str){
        init(prase_res);
        this->json = json_str;
        cur_prasc_idx = 0;
        PRASE_STATE ret;
        jump_space();
        if((ret=prase_json(prase_res)) == PRASE_OK ){
            jump_space();
            if(cur_prasc_idx< json.size()){
                prase_res.t = JP_NULL;
                ret = PARSE_ROOT_NOT_SINGULAR;
            }  
        }
        return ret;
    }

    void Json_Praser::init(VAL& v){
        v.t = JP_NULL;
        v.s.clear();
        v.a.clear();
        v.obj.clear();
    }

    void Json_Praser::jump_space(){
        while(cur_prasc_idx< json.size() && (
            json[cur_prasc_idx] == ' ' || 
            json[cur_prasc_idx] == '\t' ||
            json[cur_prasc_idx] == '\b')
        ) cur_prasc_idx++;
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_json(VAL& v){   
        jump_space();
        switch(json[cur_prasc_idx]){
            case 't':
            case 'f': return prase_bool(v);
            case 'n': return prase_null(v);
            case '\"': return prase_string(v);
            case '[': return prase_array(v);
            case '{': return prase_obj(v);
            default: return prase_number(v);
        }
        
        return PRASE_OK; 
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_obj(VAL& v){
        assert(json[cur_prasc_idx] == '{');
        cur_prasc_idx += 1;
        jump_space();
        if(json[cur_prasc_idx] == '}'){
            cur_prasc_idx += 1;
            v.t = JP_OBJECT;
            return PRASE_OK;
        }
        PRASE_STATE ret;
        while(1){
            VAL k;
            init(k);
            if(json[cur_prasc_idx] != '"'){
                ret = PRASE_MISS_KEY;
                break;
            }
            if ((ret = prase_string(k)) != PRASE_OK)
                break;
            VAL val;
            init(val);
            jump_space();
            if(json[cur_prasc_idx] != ':'){
                ret = PRASE_MISS_COLON;
                break;
            }
            cur_prasc_idx += 1;
            if ((ret = prase_json(val)) != PRASE_OK)
                break;
            v.obj[k.s] = val;
            jump_space();
            if(json[cur_prasc_idx] == ','){
                cur_prasc_idx += 1;
                jump_space();
            }else if(json[cur_prasc_idx] == '}'){
                cur_prasc_idx += 1;
                v.t = JP_OBJECT;
                return PRASE_OK;
            }
        }
        init(v);
        return ret;
    }

    bool Json_Praser::parse_hex4(unsigned &u){
        u = 0;
        for (int i = 0; i < 4; i++) {
            char ch = json[cur_prasc_idx++];
            u <<= 4;
            if      (ch >= '0' && ch <= '9')  u |= ch - '0';
            else if (ch >= 'A' && ch <= 'F')  u |= ch - ('A' - 10);
            else if (ch >= 'a' && ch <= 'f')  u |= ch - ('a' - 10);
            else return NULL;
        }
        return true;
    }

    void Json_Praser::encode_utf_8(VAL& v, unsigned u){
        if (u <= 0x7F)
            v.s.push_back(u);
        else if(u<= 0x7FF){
            v.s.push_back(0xC0 | ((u >> 6) & 0xFF));
            v.s.push_back(0x80 | ( u       & 0x3F));
        }else if (u <= 0xFFFF) {
            v.s.push_back(0xE0 | ((u >> 12) & 0xFF));
            v.s.push_back(0x80 | ((u >>  6) & 0x3F));
            v.s.push_back(0x80 | ( u        & 0x3F));
        }else {
            assert(u <= 0x10FFFF);
            v.s.push_back(0xF0 | ((u >> 18) & 0xFF));
            v.s.push_back(0x80 | ((u >> 12) & 0x3F));
            v.s.push_back(0x80 | ((u >>  6) & 0x3F));
            v.s.push_back(0x80 | ( u        & 0x3F));
        }

    }



    Json_Praser::PRASE_STATE Json_Praser::prase_array(VAL& v){
        assert(json[cur_prasc_idx] == '[');
        cur_prasc_idx += 1;
        jump_space();
        if(json[cur_prasc_idx] == ']'){
            cur_prasc_idx += 1;
            v.t = JP_ARRAY;
            return PRASE_OK;
        }
        PRASE_STATE ret;
        while(1){
            VAL e;
            if((ret = prase_json(e)) != PRASE_OK)
                break;
            v.a.push_back(e);
            jump_space();
            if(json[cur_prasc_idx] == ','){
                cur_prasc_idx += 1;
                jump_space();
            }else if(json[cur_prasc_idx] == ']'){
                cur_prasc_idx += 1;
                v.t = JP_ARRAY;
                return PRASE_OK;
            }else{
                ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
                break;
            }
        }
        init(v);
        return ret;
    }   

    Json_Praser::PRASE_STATE Json_Praser::prase_string(VAL& v){
        assert(json[cur_prasc_idx] == '\"');
        cur_prasc_idx += 1;
        unsigned u, u2;

        while(1){
            char ch = json[cur_prasc_idx++];
            switch(ch){
                case '\"': v.t=JP_STRING; return PRASE_OK;
                case '\\':
                    switch(json[cur_prasc_idx++]){
                        case '\"': v.s.push_back('\"'); break;
                        case '\\': v.s.push_back('\\'); break;
                        case '/':  v.s.push_back('/' ); break;
                        case 'b':  v.s.push_back('\b'); break;
                        case 'f':  v.s.push_back('\f'); break;
                        case 'n':  v.s.push_back('\n'); break;
                        case 'r':  v.s.push_back('\r'); break;
                        case 't':  v.s.push_back('\t'); break; 
                        case 'u':
                            if (!(parse_hex4(u))){
                                v.s.clear();
                                return PARSE_INVALID_UNICODE_HEX;
                            } 
                            if (u >= 0xD800 && u <= 0xDBFF) { 
                                if (json[cur_prasc_idx++] != '\\'){
                                    v.s.clear();
                                    return PARSE_INVALID_UNICODE_HEX;
                                }
                                if (json[cur_prasc_idx++] != 'u'){
                                    v.s.clear();
                                    return PARSE_INVALID_UNICODE_HEX;
                                }
                                if (!(parse_hex4(u2))){
                                    v.s.clear();
                                    return PARSE_INVALID_UNICODE_HEX;
                                }
                                if (u2 < 0xDC00 || u2 > 0xDFFF){
                                    v.s.clear();
                                    return PARSE_INVALID_UNICODE_HEX;
                                }
                                u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                            }
                            encode_utf_8(v, u);
                            break;
                        default:
                        v.s.clear();
                        return PARSE_INVALID_STRING_ESCAPE;
                    }
                    break;
                case '\0':
                    v.s.clear();
                    return PARSE_MISS_QUOTATION_MARK;
                default:
                    if ((unsigned char)ch < 0x20) { 
                        v.s.clear();
                        return PARSE_INVALID_STRING_CHAR;
                    }
                    v.s.push_back(ch);
            }
        }
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_number(VAL& v){

        int start = cur_prasc_idx;
        if(json[cur_prasc_idx] == '-')
            cur_prasc_idx += 1;
        
        if(json[cur_prasc_idx] == '0')
            cur_prasc_idx += 1;
        else{
            if(!ISDIGIT1TO9(json[cur_prasc_idx]))
                return PRASE_INVLID_VAL;
            while(ISDIGIT(json[cur_prasc_idx])) 
                cur_prasc_idx++;
        }

        if(json[cur_prasc_idx] == '.'){
            cur_prasc_idx+= 1;
            if(!ISDIGIT(json[cur_prasc_idx]))
                return PRASE_INVLID_VAL;
            while(ISDIGIT(json[cur_prasc_idx])) 
                cur_prasc_idx++;
        }
        
        if(json[cur_prasc_idx] == 'e' || json[cur_prasc_idx] == 'E'){
            cur_prasc_idx+= 1;
            if(json[cur_prasc_idx] == '+' || json[cur_prasc_idx] == '-')
                cur_prasc_idx+= 1;
            if(!ISDIGIT(json[cur_prasc_idx]))
                return PRASE_INVLID_VAL;
            while(ISDIGIT(json[cur_prasc_idx])) 
                cur_prasc_idx++;
        }
        errno = 0;
        v.number = strtod(json.substr(start, cur_prasc_idx-start).c_str(), NULL);
        if (errno == ERANGE && (v.number == HUGE_VAL || v.number == -HUGE_VAL))
            return PARSE_NUMBER_TOO_BIG;

        v.t = JP_NUMBER;
        return PRASE_OK;
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_bool(VAL& v){
        
        if( cur_prasc_idx+3< json.size() && 
            json[cur_prasc_idx] == 't' && 
            json[cur_prasc_idx+1] == 'r' && 
            json[cur_prasc_idx+2] == 'u' &&
            json[cur_prasc_idx+3] == 'e'){
                v.t = JP_TRUE;
                cur_prasc_idx += 4;
        }
        else if(cur_prasc_idx+4< json.size() && 
            json[cur_prasc_idx] == 'f' && 
            json[cur_prasc_idx+1] == 'a' && 
            json[cur_prasc_idx+2] == 'l' &&
            json[cur_prasc_idx+3] == 's' &&
            json[cur_prasc_idx+4] == 'e'
        ){
            v.t = JP_FALSE;
            cur_prasc_idx += 5;
        }else
            return PRASE_INVLID_VAL;
        return PRASE_OK;
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_null(VAL& v){
        if( cur_prasc_idx+3< json.size() && 
            json[cur_prasc_idx] == 'n' && 
            json[cur_prasc_idx+1] == 'u' && 
            json[cur_prasc_idx+2] == 'l' &&
            json[cur_prasc_idx+3] == 'l'){
                v.t = JP_NULL;
                cur_prasc_idx += 4;
        }else
            return PRASE_INVLID_VAL;
        return PRASE_OK;
    }

    double Json_Praser::get_number(const VAL& v){
        assert(v.t == JP_NUMBER);
        return v.number;
    }

    Json_Praser::TYPE Json_Praser::get_type(VAL& v){
        return v.t;
    }

    Json_Praser::VAL& Json_Praser::get_array_element(VAL& v, int idx){
        assert(v.t == JP_ARRAY);
        assert(v.a.size()> idx && idx>= 0);
        return v.a[idx];
    }

    int Json_Praser::get_array_size(const VAL& v){
        assert(v.t == JP_ARRAY);
        return v.a.size();
    }

}

