#include <fstream>
#include "JSON.hpp"
#include <boost/algorithm/string/regex.hpp>
#include <unordered_set>

static std::unordered_set<std::wstring> readRareWordList()
{
    std::unordered_set<std::wstring> result;

    std::wifstream ifs("testcases/rarewords.txt");
    std::wstring line;
    while (std::getline(ifs, line))
        result.insert(std::move(line));

    return result;
}

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
        int i = 0;
        for(auto& v : arr.values) {
            if (i++) // skip the first element in all arrays
                boost::apply_visitor(*this, v);
        }
    }

    /////////////////////////////////////
    // do replacements on strings
    void operator()(JSON::String& s) const {
        using namespace boost;

        const static auto rareWords = readRareWordList();
        const static auto replacement = std::wstring(L"__RARE__");

        if (rareWords.find(s.value) != rareWords.end())
            s.value = replacement;
    }
};

int main()
{
    auto document = JSON::readFrom(std::ifstream("testcases/test3.json"));

    boost::apply_visitor(RareWords(), document);

    std::cout << document;
}
