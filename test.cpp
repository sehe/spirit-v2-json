#include <sstream>
#include "JSON.hpp"

// util
static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_wstring(given));
}

void roundtrip_test()
{
    auto 
        document = JSON::readFrom(std::istringstream(
                    "{\r\n"
                    "       \"Event\": {\r\n"
                    "             \"attribute_a\": 0.0002,\r\n"
                    "\"attribute_b\": 2e-005,\r\n"
                    "\"attribute_c\": 0.022\r\n"
                    "}\r\n}")),
        verify = roundtrip(document);

    std::cout << verify;
    std::cout << "document <=> verify equal:     \t" << std::boolalpha << (document == verify)                       << "\n";
    std::cout << "document <=> verify text match:\t" << std::boolalpha << (to_string(document) == to_string(verify)) << "\n";
}

void initializer_test()
{
    using namespace JSON;

    const Array arr { 
        L"text", 
        42,
        Object { { L"dummy", Null() } } 
    };

    auto radius = as_double(arr[1]);

    auto const document = Object {
            { L"number", 314e-2 },
            { L"string", L"hello\ngoodbye" },
            { L"array" , arr },
            { L"bool" , False() },
            { L"radius", radius },
            { L"area", radius * radius * 3.14 },
            { String { 10, L'=' }, String { 10, L'*' } }
    };

    std::cout << document[L"bool"]   << std::endl;
    std::cout << document[L"number"] << std::endl;
    std::cout << document[L"string"] << std::endl;
    std::cout << document[L"array"]  << std::endl;
    std::cout << document[L"bool"]   << std::endl;
    std::cout << document[L"radius"] << std::endl;
    std::cout << document[L"area"]   << std::endl;
    std::cout << document            << std::endl;
}

int main()
{
    roundtrip_test();
    initializer_test();
}
