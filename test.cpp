#include <sstream>
#include <fstream>
#include <vector>
#include "JSON.hpp"

// util
static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_string(given));
}

void initializer_test()
{
    using namespace JSON;

    const Array arr { 
        "text", 
        42l,
        Object { { "dummy", Null() } } 
    };

    auto radius = as_int64(arr[1]);

    auto const document = Object {
            { "number", 314e-2l },
			{ "string", "hello\ngoodbye" },
			{ "array" , arr },
			{ "bool" , Bool(false) },
            { "radius", radius },
            { "area", radius * radius * 3.14l },
            { std::string { 10, '=' }, std::string { 10, '*' } }
    };

    std::cout << document["bool"]   << std::endl;
    std::cout << document["number"] << std::endl;
    std::cout << document["string"] << std::endl;
    std::cout << document["array"]  << std::endl;
    std::cout << document["bool"]   << std::endl;
    std::cout << document["radius"] << std::endl;
    std::cout << document["area"]   << std::endl;
    std::cout << document            << std::endl;
}

void karma_tdd()
{
    using namespace JSON;
    for (auto o : std::vector<Value> { 
                Null(),
                Bool(true),
                Bool(false),
                std::string("string"),
                "s\fecial\tcha\rs \before \nothing \\else gets done \"quote\"",
                43l,
                3.14l,
                Array { Null(), Bool(true), Bool(false), std::string("array"), 43l, 3.14l },
            })
    {
        std::cout.clear();
        std::cout << "---"         << std::endl;
        std::cout << to_string(o)  << std::endl;
    }
}

void unicode_roundtrip()
{
    auto 
        value   = JSON::readFrom(std::ifstream("testcases/test1.json")),
        verify1 = roundtrip(value),
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
