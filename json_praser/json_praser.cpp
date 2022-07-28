#include "json_praser.h"

namespace JP{
    #define ISDIGIT1TO9(ch) ((ch)>= '1'&& (ch)<= '9')
    #define ISDIGIT(ch) ((ch)>= '0'&& (ch)<= '9')

    const char* Json_Praser::get_string(VAL& v){
        assert(v.t == JP_STRING);
        return v.s.c_str();
    }

    int Json_Praser::get_string_length(VAL& v){
        assert(v.t == JP_STRING);
        return strlen(v.s.c_str());
    }

    Json_Praser::PRASE_STATE Json_Praser::operator()(string json_str){
        this->json = json_str;
        cur_prasc_idx = 0;
        PRASE_STATE ret;
        jump_space();
        if((ret=prase_json()) == PRASE_OK ){
            jump_space();
            if(cur_prasc_idx< json.size()){
                prase_res.t = JP_NULL;
                ret = PARSE_ROOT_NOT_SINGULAR;
            }
                
        }
        return ret;
    }

    void Json_Praser::init(){
        prase_res.t = JP_NULL;
    }

    void Json_Praser::jump_space(){
        while(cur_prasc_idx< json.size() && (
            json[cur_prasc_idx] == ' ' || 
            json[cur_prasc_idx] == '\t' ||
            json[cur_prasc_idx] == '\b')
        ) cur_prasc_idx++;
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_json(){   
        switch(json[cur_prasc_idx]){
            case 't':
            case 'f': return prase_bool(prase_res);
            case 'n': return prase_null(prase_res);
            case '\"': return prase_string(prase_res);
            default: return prase_number(prase_res);
        }
        
        return PRASE_OK; 
    }

    Json_Praser::PRASE_STATE Json_Praser::prase_string(VAL& v){
        assert(json[cur_prasc_idx] == '\"');
        cur_prasc_idx += 1;
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

    double Json_Praser::get_number(VAL& v){
        assert(v.t == JP_NUMBER);
        return prase_res.number;
    }

    Json_Praser::TYPE Json_Praser::get_type(VAL& v){
        return v.t;
    }
}