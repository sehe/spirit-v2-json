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

        Object document( {
                { String { L"number" }, Number { 314e-2 } },
                { String { L"string" }, String { L"hello\ngoodbye" } },
                { String { L"array" }, Array { { 
                        String { L"text" }, 
                        Number { 42 },
                        Object { { String { L"dummy" }, Null() } } 
                    } } 
                },
                { String { L"bool" }, False() },
        } );

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
