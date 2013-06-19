#include <sstream>
#include <fstream>
#include <vector>
#include "JSON.hpp"

// util
static JSON::Value wroundtrip(JSON::Value const& given) {
    return JSON::parse(to_wstring(given));
}

static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_string(given));
}

void initializer_test()
{
    using namespace JSON;

    const Array arr { 
        L"text", 
        42l,
        Object { { L"dummy", Null() } } 
    };

    auto radius = as_int64(arr[1]);

    auto const document = Object {
            { L"number", 314e-2l },
            { L"string", L"hello\ngoodbye" },
            { L"array" , arr },
            { L"bool" , false },
            { L"radius", radius },
            { L"area", radius * radius * 3.14l },
            { std::wstring { 10, L'=' }, std::wstring { 10, L'*' } }
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

void karma_tdd()
{
    using namespace JSON;
    for (auto o : std::vector<Value> { 
                Null(),
                true,
                false,
                std::wstring(L"string"),
                L"s\fecial\tcha\rs \before \nothing \\else gets done \"quote\"",
                43l,
                3.14l,
                Array { Null(), true, false, std::wstring(L"array"), 43l, 3.14l },
            })
    {
        std:: cout << "---"         << std::endl;
        std:: cout << to_string(o)  << std::endl;
        std::wcout << to_wstring(o) << std::endl;
    }
}

void unicode_roundtrip()
{
    auto 
        value   = JSON::readFrom(std::ifstream("testcases/test1.json")),
        verify1 = wroundtrip(value),
        verify2 = roundtrip(verify1);

    std::cout << "value <=> verify #1 text match:\t" << std::boolalpha << (to_string(value) == to_string(verify1)) << "\n";
    std::cout << "value <=> verify #2 text match:\t" << std::boolalpha << (to_string(verify1) == to_string(verify2)) << "\n";

    std::cout << "value <=> verify #1 equality match:\t" << std::boolalpha << (value == verify1) << "\n";
    std::cout << "value <=> verify #2 equality match:\t" << std::boolalpha << (verify1 == verify2) << "\n";

    std::cout << verify2 << "\n";
}

int main()
{
    std::cout << " =================================================\n";
    std::cout << " === karma_tdd\n";
    std::cout << " =================================================\n";
    karma_tdd();

    std::cout << " =================================================\n";
    std::cout << " === unicode_roundtrip\n";
    std::cout << " =================================================\n";
    unicode_roundtrip();

    std::cout << " =================================================\n";
    std::cout << " === initializer_test\n";
    std::cout << " =================================================\n";
    initializer_test();
}
