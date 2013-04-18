#include <fstream>
#include <boost/regex/pending/unicode_iterator.hpp> // unicode, please
#include "JSON.hpp"

// util
static JSON::Value roundtrip(JSON::Value const& given) {
    return JSON::parse(to_string(given));
}

int main()
{
    // read full stdin
    std::ifstream ifs("testcases/test1.json");
    ifs.unsetf(std::ios::skipws);
    std::istream_iterator<char, char> it(ifs), pte;
    const std::string raw(it, pte);

    // wrapping std::istream_iterator<char> doesn't work (probably requires
    // forward/bidi iterators)
    typedef boost::u8_to_u32_iterator<std::string::const_iterator> Conv2Utf32;
    const std::wstring input(Conv2Utf32(begin(raw)), Conv2Utf32(end(raw)));

    // actual test
    auto 
        value   = JSON::parse(input),
        verify1 = roundtrip(value),
        verify2 = roundtrip(verify1);

    std::cout << "value <=> verify #1 match:\t" << std::boolalpha << (to_string(value) == to_string(verify1)) << "\n";
    std::cout << "value <=> verify #2 match:\t" << std::boolalpha << (to_string(value) == to_string(verify2)) << "\n";

    // output helpers
    boost::utf8_output_iterator<std::ostream_iterator<char>> to_utf8(std::cout);
    auto dump = [&to_utf8](JSON::Value const& v)  { auto s = to_string(v); std::copy(std::begin(s), std::end(s), to_utf8); };

    dump(verify2);
}
