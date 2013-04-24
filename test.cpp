#include <fstream>
#include "JSON.hpp"

// util
static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_wstring(given));
}

void initializer_test()
{
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

    //typedef boost::u8_to_u32_iterator<std::string::const_iterator> Conv2Utf32;
    //const std::wstring input(Conv2Utf32(begin(raw)), Conv2Utf32(end(raw)));
}

int main()
{
    initializer_test();
    return 0;
    auto 
        value   = JSON::readFrom(std::ifstream("testcases/rapptz.json")),
        verify1 = roundtrip(value),
        verify2 = roundtrip(verify1);

    std::cout << "value <=> verify #1 match:\t" << std::boolalpha << (to_string(value) == to_string(verify1)) << "\n";
    std::cout << "value <=> verify #2 match:\t" << std::boolalpha << (to_string(verify1) == to_string(verify2)) << "\n";

    std::cout << verify2;
}
