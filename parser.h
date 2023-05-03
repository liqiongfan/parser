#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include "parser.h"
#include <vector>
#include <unordered_map>
#include <memory>

struct position {
    explicit position(int l, int c) : line(l), column(c) {}
    int line;
    int column;
};

inline
position &operator +=(position &p, int l) {
    p.line += l;
    return p;
}

struct location {
    location(int l, int c) : begin(l, c), end(l, c) { }
    location() : begin(1, 0), end(1, 0) { }
    void step() { begin = end; }
    void columns(int c) { end.column += c; }
    void lines(int l) { end.line += l; end.column = 0; }

    position begin;
    position end;
};

enum {
    YYERROR = -1,
    YYEOF = 0,
    INT = 260,
    LONG = 261,
    DOUBLE = 262,
    BOOL = 263,
    STRING = 264,
    NUL = 265,
    ARRAY = 266,
    OBJECT = 267,
    NAN = 268,
    INF = 269,
    HEX = 270,
};

struct kind {
    int token;
    kind(int t) : token(t) {}
    virtual ~kind() = default;
};

template<typename T>
struct symbol : kind {
    symbol(int t, T v, location l) : kind(t), value(v), loc(l) { }
    T value;
    location loc;
};

template<typename T>
T get_value(std::shared_ptr<kind> v) {
    return dynamic_cast<symbol<T>*>(v.get())->value;
}

struct scanner {
    char *start{nullptr};
    char *begin{nullptr};
    char *limit{nullptr};
    char *marker{nullptr};
    int   state{0};
    location loc{};
    std::string msg{};
};

std::shared_ptr<kind> yylex(scanner &s);


struct base { virtual ~base() = default; };

template<typename _T>
struct data: base {
    data(_T t) : v(t) { }
    _T v;
};

struct json {
    typedef std::vector<json>                       array;
    typedef std::unordered_map<std::string, json>   object;

    json() = default;

    struct nan { };
    struct null{ };
    struct infinity{ bool neg; infinity(bool negative = false): neg(negative){} };
    struct boolean{ bool yes; boolean(bool y=false): yes(y){} };
    struct hex {
        hex() = default;
        hex(long t, std::string v) : value(t), raw_value(v) {}
        long value;
        std::string raw_value;
    };

    template<typename _T>
    json(_T t) { ptr = std::make_shared<data<_T>>(t); name = typeid(t).name(); }

    template<typename _T>
    bool is() { if (typeid(_T).name() == name) { return true; } else { return false; } }

    json operator[](const std::string &key) {
        if (this->is_object()) {
            auto m = this->get_object();
            if (m.count(key) == 1) {
                return m[key];
            }
            return nullptr;
        }

        return nullptr;
    }

    json operator[](const int &i) {
        if (this->is_array()) {
            auto m = this->get_array();
            if (m.size() - 1 < i) {
                return nullptr;
            }
            return m[i];
        }

        return nullptr;
    }

    bool is_array() { return this->is<array>(); }
    bool is_object() { return this->is<object>(); }
    bool is_int() { return this->is<int>(); }
    bool is_long() { return this->is<long>(); }
    bool is_double() { return this->is<double>(); }
    bool is_float() { return this->is<float>(); }
    bool is_string() { return this->is<const char *>() || this->is<std::string>(); }
    bool is_null() { return this->is<null>(); }
    bool is_bool() { return this->is<boolean>(); }
    bool is_infinity() { return this->is<infinity>(); }
    bool is_nan() { return this->is<nan>(); }
    bool is_hex() { return this->is<hex>(); }
    bool is_not_found() { return this->is<std::nullptr_t>(); }
    bool has() { return !this->is<std::nullptr_t>(); }

    int get_int() { return this->get<int>(); }
    long get_long() { return this->get<long>(); }
    double get_double() { return this->get<double>(); }
    float get_float() { return this->get<float>(); }
    infinity get_infinity() { return this->get<infinity>(); }
    boolean get_bool() { return this->get<boolean>(); }
    hex get_hex() { return this->get<hex>(); }

    std::string get_string();
    array get_array() { return this->get<array>();    }
    object get_object() { return this->get<object>(); }

    std::string toString(bool hex = false);
    std::string toPrettyString(bool hex = false);

    template<class T>
    T get() { return this->template cast<T>(); }

    template<typename _T>
    _T cast() { return dynamic_cast<data<_T>*>(ptr.get())->v; }

private:
    bool pretty{false};
    std::string identity{};
    std::shared_ptr<base> ptr{nullptr};
    const char *name = nullptr;
};


struct parser {
    scanner s;
    json parse(const std::string &buffer);
    json parse_value();
    json::object parse_object();
    json::array parse_array();
    json get_token_value(std::shared_ptr<kind> v);
    std::string exitWithError(const std::string &msg) {
        std::string message{};
        if (s.limit == s.begin - 1) {

        } else {
            message = std::string(s.begin, (s.limit - s.begin) > 300 ? s.begin + 300 : s.limit);
        }
        auto pos = message.find_first_of('\n');
        if (pos != std::string::npos) {
            message = message.substr(0, pos);
        }
        return msg + " in [line:column]->" + std::to_string(s.loc.end.line) + ":" + std::to_string(s.loc.end.column) + "\n" + message;
    }
};

std::shared_ptr<kind> make_INT(int i, location loc);

std::shared_ptr<kind> make_YYEOF(location loc);

std::shared_ptr<kind> make_YYERROR(location loc);

std::shared_ptr<kind> make_DOBLE(double d, location loc);

std::shared_ptr<kind> make_BOOL(bool b, location loc) ;

std::shared_ptr<kind> make_HEX(long i, const std::string &v, location loc);

std::shared_ptr<kind> make_STRING(const std::string &str, location loc);

std::shared_ptr<kind> make_NULL(location loc);

std::shared_ptr<kind> make_SYMBOL(int t, location loc);

std::shared_ptr<kind> make_NAN(json::nan n,location loc);

std::shared_ptr<kind> make_INF(json::infinity, location loc);


enum YYCONDTYPE {
    yycJSON,
    yycSTR,
    yycSTR1,
    yycCOMMENTS,
    yycCOMMENTS1,
};

#endif
