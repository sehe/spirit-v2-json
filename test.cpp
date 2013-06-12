#include <sstream>
#include "JSON.hpp"

int main()
{
    auto document = JSON::readFrom(std::istringstream(
                "{\"RootA\":\"Value in parent node\",\"ChildNode\":{\"ChildA\":[1,2],\"ChildB\":42}}"));

    auto childA = as_object(
                as_object(document)[L"ChildNode"]
            )[L"ChildA"];

    std::cout << childA << std::endl;

    // or use each value
    for(auto& value : as_array(childA).values)
        std::cout << value << std::endl;

    // more advanced:
    JSON::Value expected = JSON::Object {
        { L"RootA", L"Value in parent node" },
        { L"ChildNode", JSON::Object {
                 { L"ChildA", JSON::Array { 1,2 } },
                 { L"ChildB", 42 },
             } },
    };
    std::cout << "Check equality: " << std::boolalpha << (document == expected) << std::endl;
    std::cout << "Serialized: " << document << std::endl;
}
