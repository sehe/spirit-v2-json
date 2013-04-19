#include <fstream>
#include "JSON.hpp"

// util
static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_wstring(given));
}

int main()
{
    auto 
        value   = JSON::readFrom(std::ifstream("testcases/test1.json")),
        verify1 = roundtrip(value),
        verify2 = roundtrip(verify1);

    std::cout << "value <=> verify #1 match:\t" << std::boolalpha << (to_string(value) == to_string(verify1)) << "\n";
    std::cout << "value <=> verify #2 match:\t" << std::boolalpha << (to_string(value) == to_string(verify2)) << "\n";

    std::cout << verify2;
}
