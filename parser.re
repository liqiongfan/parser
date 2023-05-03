#include "parser.h"

#define YYCURSOR s.begin
#define YYLIMIT s.limit
#define YYCTYPE unsigned char
#define YYMARKER s.marker
#define YYGETCONDITION() s.state
#define YYSETCONDITION(cond) s.state=(yyc##cond)
#define GOTO(cond) goto yyc_##cond
#define SETANDJUMPTO(cond) YYSETCONDITION(cond); GOTO(cond)


std::shared_ptr<kind> make_INT(int i, location loc) {
    return std::make_shared<symbol<int>>(INT, i, loc);
}

std::shared_ptr<kind> make_LONG(long i, location loc) {
    return std::make_shared<symbol<long>>(LONG, i, loc);
}

std::shared_ptr<kind> make_YYEOF(location loc) {
    return std::make_shared<symbol<bool>>(YYEOF, false, loc);
}

std::shared_ptr<kind> make_YYERROR(location loc) {
    return std::make_shared<symbol<bool>>(YYERROR, false, loc);
}

std::shared_ptr<kind> make_DOBLE(double d, location loc) {
    return std::make_shared<symbol<double>>(DOUBLE, d, loc);
}

std::shared_ptr<kind> make_BOOL(bool b, location loc) {
    return std::make_shared<symbol<bool>>(BOOL, b, loc);
}

std::shared_ptr<kind> make_STRING(const std::string &str, location loc) {
    return std::make_shared<symbol<std::string>>(STRING, str, loc);
}

std::shared_ptr<kind> make_NULL(location loc) {
    return std::make_shared<symbol<bool>>(NUL, false, loc);
}

std::shared_ptr<kind> make_SYMBOL(int t, location loc) {
    return std::make_shared<symbol<bool>>(t, false, loc);
}

/* add logic for real implementation */
std::shared_ptr<kind> make_ARRAY(location loc) {
    return std::make_shared<symbol<bool>>(ARRAY, false, loc);
}

std::shared_ptr<kind> make_OBJECT(location loc) {
    return std::make_shared<symbol<bool>>(OBJECT, false, loc);
}

std::shared_ptr<kind> make_NAN(json::nan n,location loc) {
    return std::make_shared<symbol<json::nan>>(NAN, n, loc);
}

std::shared_ptr<kind> make_INF(json::infinity i,location loc) {
    return std::make_shared<symbol<json::infinity>>(INF, i, loc);
}


std::string json::get_string() {
    if (this->is<const char *>()) {
        return std::string(this->get<const char *>());
    } else {
        return this->get<std::string>();
    }
}

std::string json::toString(bool hex)
{
    if (this->is_array()) {
        std::string str = "[";
        auto v = this->get_array();
        for (auto i = 0; i < v.size(); i++) {
            str = str.append(v.at(i).toString(hex));
            if (i < v.size() - 1) {
                str = str.append(",");
            }
        }
        return str.append("]");
    }

    if (this->is_double()) {
        return std::to_string(this->get_double());
    }
    if (this->is_float()) {
        return std::to_string(this->get_float());
    }
    if (this->is_int()) {
        return std::to_string(this->get_int());
    }
    if (this->is_long()) {
        return std::to_string(this->get_long());
    }

    if (this->is_string()) {
        std::string p{};
        for (auto c : this->get_string()) {
            if (c == '"') {
                p = p.append(1, '\\');
            }
            p = p.append(1, c);
        }
        return std::string("\"").append(p).append("\"");
    }
    if (this->is_null()) {
        return "null";
    }

    if (this->is_infinity()) {
        if (this->get_infinity().neg) {
            return "-Infinity";
        } else {
            return "Infinity";
        }
    }

    if (this->is_nan()) {
        return "NaN";
    }

    if (this->is_hex()) {
        if (hex) {
            return this->get_hex().raw_value;
        } else {
            return std::to_string(this->get_hex().value);
        }
    }

    if (this->is_bool()) {
        if (this->get_bool().yes) {
            return "true";
        } else {
            return "false";
        }
    }

    if (this->is_object()) {
        auto i = 0;
        auto m = this->get_object();
        std::string str = "{";
        for (auto begin = m.begin(); begin != m.end(); begin++, i++) {
            str = str.append("\"").append(begin->first).append("\":");
            str = str.append(begin->second.toString(hex));
            if (i < m.size() -1) {
                str = str.append(",");
            }
        }
        return str.append("}");
    }

    return "";
}


template<>
json::json(std::initializer_list<json> t): ptr( new data<array>(t) ) { name = typeid(array).name(); }

template<>
json::json(bool t): ptr( new data<boolean>(t) ) { name = typeid(boolean).name(); }


json parser::parse(const std::string &buffer) {
    s.start = s.begin = (char *)buffer.data();
    s.limit = s.begin + buffer.size();
    s.loc = location();
    s.state = 0;
    return this->parse_value();
}

json parser::parse_value() {
    auto token = yylex(s);
    switch (token->token) {
        case LONG:
            return get_value<long>(token);
        case DOUBLE:
            return get_value<double>(token);
        case BOOL:
            return get_value<bool>(token);
        case STRING:
            return get_value<std::string>(token);
        case NUL:
            return json::null();
        case NAN:
            return get_value<json::nan>(token);
        case INF:
            return get_value<json::infinity>(token);
        case '{':
            return this->parse_object();
        case '[':
            return this->parse_array();
        default:
            throw exitWithError("Unknown token");
    }
}


json::object parser::parse_object() {
    json::object obj;
    more_kvs:
    auto field = yylex(s);
    if (field->token == '}') {
        return obj;
    }
    if (field->token != STRING) {
        throw exitWithError("Unknown token");
    }
    auto colon = yylex(s);
    if (colon->token != ':') {
        throw exitWithError("object field need a colon symbol before value");
    }
    auto value = this->parse_value();
    obj.insert({get_value<std::string>(field), value });

    auto has_more = yylex(s);
    if (has_more->token == ',') {
        goto more_kvs;
    } else if (has_more->token == '}'){
        return obj;
    } else {
        throw exitWithError("object must be end with }.");
    }
}

json::array parser::parse_array() {
    json::array array;
    more_elements:
    auto ele = yylex(s);
    if (ele->token == ']') {
        return array;
    }
    json e;
    switch (ele->token) {
        case LONG:
            e = get_value<long>(ele);
            break;
        case DOUBLE:
            e = get_value<double>(ele);
            break;
        case BOOL:
            e = get_value<bool>(ele);
            break;
        case STRING:
            e = get_value<std::string>(ele);
            break;
        case NUL:
            e = json::null();
            break;
        case NAN:
            e = get_value<json::nan>(ele);
            break;
        case INF:
            e = get_value<json::infinity>(ele);
            break;
        case '{':
            e = this->parse_object();
            break;
        case '[':
            e = this->parse_array();
            break;
        default:
            throw exitWithError("Unknown token");
    }
    array.push_back(e);
    auto deli = yylex(s);
    if (deli->token == ']') {
        return array;
    } else if (deli->token == ',') {
        goto more_elements;
    } else {
        throw exitWithError("array must be end with ].");
    }
}

std::shared_ptr<kind> yylex(scanner &s) {

std:
    std::string data{};
    char *begin = s.begin;
/*!re2c
re2c:indent:top = 1;
re2c:yyfill:enable = 0;

DIGIT   = [0-9];
DIGITNZ = [1-9];
UINT    = "0" | (DIGITNZ DIGIT*);
INT     = ("-"|"+")? UINT;
FLOAT   = (INT) ? "." DIGIT*;
EXP     = (INT | FLOAT) [eE] [+-]? DIGIT+;
HEX     = "0"[xX][a-fA-F0-9]+;
WS      = [ \t\r]+;
NL      = "\r"? "\n";
EOI     = "\000";
LABEL	= [a-zA-Z_\x80-\xff][a-zA-Z0-9_\x80-\xff]*;

<JSON> "{" { s.loc.columns(s.begin - begin); return make_SYMBOL('{', s.loc); }
<JSON> "}" { s.loc.columns(s.begin - begin); return make_SYMBOL('}', s.loc); }
<JSON> "[" { s.loc.columns(s.begin - begin); return make_SYMBOL('[', s.loc); }
<JSON> "]" { s.loc.columns(s.begin - begin); return make_SYMBOL(']', s.loc); }
<JSON> "," { s.loc.columns(s.begin - begin); return make_SYMBOL(',', s.loc); }
<JSON> ":" { s.loc.columns(s.begin - begin); return make_SYMBOL(':', s.loc); }
<JSON> "+"?'Infinity' { s.loc.columns(s.begin - begin); return make_INF(json::infinity(), s.loc);      }
<JSON> '-Infinity' { s.loc.columns(s.begin - begin); return make_INF(json::infinity(true), s.loc);  }
<JSON> 'NaN' { s.loc.columns(s.begin - begin); return make_NAN(json::nan(), s.loc); }
<JSON> EOI { return make_YYEOF(s.loc); }

<JSON> NL {
    s.loc.lines(1);
    goto std;
}
<JSON> WS {
    s.loc.columns(s.begin - begin);
    goto std;
}

<JSON> LABEL {
    std::string v(begin, s.begin);
    return make_STRING(v, s.loc);
}

<JSON> ["] {
    s.loc.columns(s.begin - begin);
    begin = s.begin;
    SETANDJUMPTO(STR);
}

<STR,STR1> EOI {
    s.msg = "double-quotation-marks string unclosed";
    goto syntax_error;
}

<STR> [\n] {
    if (*(s.begin - 2) == '\r') {
        if (*(s.begin - 3) != '\\') {
            s.msg = "new-line string must escaped by slash \\";
            goto syntax_error;
        }
        data.append(begin, s.begin-3);
    } else {
        if (*(s.begin-2) != '\\') {
            s.msg = "new-line string must escaped by slash \\";
            goto syntax_error;
        }
        data.append(begin, s.begin-2);
    }
    begin = s.begin;
    s.loc.lines(1);
    SETANDJUMPTO(STR);
}

<STR>[^"] {
    s.loc.columns(1);
    SETANDJUMPTO(STR);
}

<STR>["] {
    std::string v(begin, s.begin-1);
    if (data.size() >= 1) {
        v = data + v;
    }
    data.clear();
    YYSETCONDITION(JSON);
    return make_STRING(v, s.loc);
}

<JSON> ['] {
    s.loc.columns(1);
    begin = s.begin;
    SETANDJUMPTO(STR1);
}

<STR1> [\n] {
    if (*(s.begin - 2) == '\r') {
        if (*(s.begin - 3) != '\\') {
            goto syntax_error;
        }
        data.append(begin, s.begin-3);
    } else {
        if (*(s.begin-2) != '\\') {
            goto syntax_error;
        }
        data.append(begin, s.begin-2);
    }
    begin = s.begin;
    s.loc.lines(1);
    SETANDJUMPTO(STR1);
}

<STR1>[^'] {
    s.loc.columns(s.begin-begin);
    SETANDJUMPTO(STR1);
}

<STR1>['] {
    std::string v(begin, s.begin-1);
    if (data.size() >= 1) {
        v = data + v;
    }
    data.clear();
    YYSETCONDITION(JSON);
    return make_STRING(v, s.loc);
}

<JSON> INT {
    s.loc.columns(s.begin - begin);
    std::string t(begin, s.begin);
    long l = std::strtol(t.data(), nullptr, 10);
    return make_LONG(l, s.loc);
}

<JSON> FLOAT | EXP {
    s.loc.columns(s.begin - begin);
    std::string t(begin, s.begin);
    double d = std::strtod(t.data(), nullptr);
    return make_DOBLE(d, s.loc);
}

<JSON> 'true' {
    s.loc.columns(s.begin - begin);
    return make_BOOL(true, s.loc);
}

<JSON> 'false' {
    s.loc.columns(s.begin - begin);
    return make_BOOL(false, s.loc);
}

<JSON> HEX {
    s.loc.columns(s.begin - begin);
    std::string t(begin, s.begin);
    long i = std::strtol(t.data(), nullptr, 16);
    return make_LONG(i, s.loc);
}

<JSON> 'null' {
    s.loc.columns(s.begin - begin);
    return make_NULL(s.loc);
}

<JSON> '//' | '#' {
    s.loc.columns(s.begin - begin);
    SETANDJUMPTO(COMMENTS);
}

<COMMENTS>[^\n] {
    s.loc.columns(1);
    SETANDJUMPTO(COMMENTS);
}

<COMMENTS> EOI {
    s.msg = "single-line comments unclosed";
    goto syntax_error;
}

<COMMENTS> '\n' {
     s.loc.lines(1);
     begin = s.begin;
     SETANDJUMPTO(JSON);
}

<JSON> "/*" {
    s.loc.columns(s.begin - begin);
    SETANDJUMPTO(COMMENTS1);
}

<COMMENTS1> '\n' {
    s.loc.lines(1);
    SETANDJUMPTO(COMMENTS1);
}

<COMMENTS1> "*/" {
    s.loc.columns(s.begin - begin);
    begin = s.begin;
    SETANDJUMPTO(JSON);
}


 */
syntax_error:
    return make_YYERROR(s.loc);
}