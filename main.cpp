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
