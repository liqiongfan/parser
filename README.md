# JSON{5} for modern c++
A C++ library for interacting with `JSON` & `JSON5`.

`JSON` is a lightweight data-interchange format. It can represent numbers, strings, list, key-value pairs and so on.

`JSON{5} for modern c++` is a c++ library that allows manipulating JSON values, including serialization and deserialization to and from strings.

# C Language

C Language press this to obtain: [Zua](https://github.com/liqiongfan/Zua)
<https://github.com/liqiongfan/Zua>

# Requirements
+ `g++` or `clang++` with compile arg: `-std=c++11`


# Build & Test
A simple example for running the JSON{5} for modern c++ is to clone the repository and then open the terminal running the commands: `make run`

```shell
$ cd parer
$ make

output:
{"backwardsCompatible":"with JSON","andIn":["arrays"],"positiveSign":1,"andTrailing":8675309.000000,"leadingDecimalPoint":0.867531,"trailingComma":"in objects","hexadecimal":0xdecaf,"lineBreaks":"Look, Mom! No \\n's!","singleQuotes":"I can use \"double quotes\" here","unquoted":"and you can quote me on that"}
```

# How to use ?

Copy `parser.h` and `parser.cpp` into your project, after that, include `parser.h` into your source file.

# Data value

+ C++: `long` store json `integer` value.
+ C++: `double` store json `floating` value.
+ C++: struct `boolean` or C++ `true` or `false` store the json `bool` value
+ C++: struct `null` store json `null` value
+ C++: struct `infinity` store the `+Inf` or `-Inf`
+ C++: struct `hex` store the `hex` value
+ C++: `std::vector<json>` store the `array` value
+ C++: `std::unordered_map<std::string, json>` store the `object` value

# Encoding example

```c++
#include <iostream>
#include "parser.h"

int main()
{
  json::object obj;
  obj["program"] = "json{5} for modern c++";  
  obj["version"] = "v1.0.0";
  obj.insert({"is_ok", json::boolean(true)});

  json::array vec;
  vec.push_back("c++");
  vec.push_back("linux");
  vec.push_back("make");
  vec.push_back(1);
  vec.push_back(json::boolean(false));
  vec.push_back(json::null());

  obj.insert({"items", vec});

  json p = obj;
  std::cout << p.toString() << std::endl;
  
  return 0;
}

output:
{"items":["c++","linux","make",1,true,null],"is_ok":false,"version":"v1.0.0","program":"jsoncpp"}
```


# Decoding Example

```c++
#include <iostream>
#include "parser.h"

int main() {

    std::string buffer = R"(
{
  // comments
  unquoted: 'and you can quote me on that',
  singleQuotes: 'I can use "double quotes" here',
  lineBreaks: "Look, Mom! \
No \\n's!",
  hexadecimal: 0xdecaf,
  leadingDecimalPoint: .8675309, andTrailing: 8675309.,
  positiveSign: +1,
  trailingComma: 'in objects', andIn: ['arrays',],
  "backwardsCompatible": "with JSON",
}
)";
    try {
        parser parser;
        auto v = parser.parse(buffer);
        std::cout << v.toString();
    } catch (std::string &s) {
        std::cout << s << std::endl;
    }

    return 0;
}


output:
{"backwardsCompatible":"with JSON","andIn":["arrays"],"positiveSign":1,"andTrailing":8675309.000000,"leadingDecimalPoint":0.867531,"trailingComma":"in objects","hexadecimal":0xdecaf,"lineBreaks":"Look, Mom! No \\n's!","singleQuotes":"I can use \"double quotes\" here","unquoted":"and you can quote me on that"}
```


# Quick way access data

```C++

#include <iostream>
#include "parser.h"


  json::array vec;
  vec.push_back("c++");
  vec.push_back("linux");
  vec.push_back("make");
  vec.push_back(1);
  vec.push_back(json::boolean(true));
  vec.push_back(json::null());


  json x = vec;
  if (x[0].has()) {
    std::cout << x[0].get_string() << std::endl;
  }


  json::object obj;
  obj.insert({"program", "jsoncpp"});  
  obj.insert({"version", "v1.0.0"});
  obj.insert({"is_ok", false});

  json p = obj;
  if (p["is_ok"].has()) {
    std::cout << p["is_ok"].get_bool().yes << std::endl;
  }
```


# End

Have fun.
