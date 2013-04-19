#include <fstream>
#include <boost/regex/pending/unicode_iterator.hpp> // unicode, please
#include "JSON.hpp"
#include <boost/algorithm/string/regex.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/phoenix.hpp>

struct RareWords : boost::static_visitor<> {

    /////////////////////////////////////
    // do nothing by default
    template <typename T> void operator()(T&&) const { /* leave all other things unchanged */ }

    /////////////////////////////////////
    // recurse arrays and objects
    void operator()(JSON::Object& obj) const { 
        for(auto& v : obj.values) {
            //RareWords::operator()(v.first); /* to replace in field names (?!) */
            boost::apply_visitor(*this, v.second);
        }
    }

    void operator()(JSON::Array& arr) const {
        for(auto& v : arr.values) {
            boost::apply_visitor(*this, v);
        }
    }

    /////////////////////////////////////
    // do replacements on strings
    void operator()(JSON::String& s) const {
        using namespace boost;
        using namespace adaptors;
        using namespace phoenix::arg_names;

        const static std::vector<std::wstring> rareWords { 
            L"late" , 
            L"populate" , 
            L"convicts" ,
        };
        const static auto&& regexen = rareWords 
            | transformed(phoenix::construct<wregex>(L"^" + arg1 + L"$"));

        const static std::wstring replacement = L"__RARE__";

        for (auto&& word : regexen)
            replace_all_regex(s.value, word, replacement);
    }
};

void dump(JSON::Value const& v)
{
    boost::utf8_output_iterator<std::ostream_iterator<char>> to_utf8(std::cout);
    auto s = to_string(v); 
    std::copy(std::begin(s), std::end(s), to_utf8);
}

int main()
{
    auto document = JSON::readFrom(std::ifstream("testcases/test3.json"));

    boost::apply_visitor(RareWords(), document);

    dump(document);
}
