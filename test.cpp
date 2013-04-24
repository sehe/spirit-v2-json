#include <fstream>
#include "JSON.hpp"

// util
static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_wstring(given));
}

void initializer_test()
{
    using namespace JSON;

    Number radius(42);
    Value v = radius;
    v = L"Hello";
    v = radius * radius * 3.14;

    auto const document = Object {
            { L"number", 314e-2 },
            { L"string", L"hello\ngoodbye" },
            { L"array" , Array { 
                    L"text", 
                    42,
                    Object { { L"dummy", Null() } } 
                }
            },
            { L"bool" , False() },
            { L"radius", radius },
            { String { 10, L'=' }, String { 10, L'*' } }
    };

    std::cout << document[L"bool"]   << std::endl;
    std::cout << document[L"number"] << std::endl;
    std::cout << document[L"string"] << std::endl;
    std::cout << document[L"array"]  << std::endl;
    std::cout << document[L"bool"]   << std::endl;
    std::cout << document            << std::endl;
}

void roundtrip_test()
{
    auto 
        value   = JSON::readFrom(std::ifstream("testcases/test1.json")),
        verify1 = roundtrip(value),
        verify2 = roundtrip(verify1);

    std::cout << "value <=> verify #1 text match:\t" << std::boolalpha << (to_string(value) == to_string(verify1)) << "\n";
    std::cout << "value <=> verify #2 text match:\t" << std::boolalpha << (to_string(verify1) == to_string(verify2)) << "\n";

    std::cout << "value <=> verify #1 equality match:\t" << std::boolalpha << (value == verify1) << "\n";
    std::cout << "value <=> verify #2 equality match:\t" << std::boolalpha << (verify1 == verify2) << "\n";

    std::cout << verify2;
}

int main()
{
    initializer_test();
    roundtrip_test();
}
