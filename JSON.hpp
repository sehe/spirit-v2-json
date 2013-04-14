#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <unordered_map> // replace with <map> or <tr1/unordered_map> if desired
#include <deque>

namespace JSON
{
    template <typename Tag> struct Literal { };

    typedef Literal<struct tag_undefined> Undefined;
    typedef Literal<struct tag_false> False;
    typedef Literal<struct tag_null>  Null;
    typedef Literal<struct tag_true>  True;

    struct Object;
    struct Array;

    struct String { 
        std::string value; 
        bool operator==(String const& s) const { return value == s.value; }
    };
}

namespace std
{
    template <> struct hash<JSON::String>
    {
        size_t operator()(JSON::String const& s) const {
            static std::hash<std::string> _hash;
            return _hash(s.value);
        }
    };
}

namespace JSON
{
    struct Number { double value;      };

    typedef boost::variant<
            Undefined, // not legal as a JSON result
            False, 
            Null, 
            True, 
            boost::recursive_wrapper<Object>, 
            boost::recursive_wrapper<Array>,
            String,
            Number
        > Value;

    struct Object
    {
        typedef std::unordered_map<String, Value> values_t;
        values_t values;
    };

    struct Array
    {
        typedef std::deque<Value> values_t;
        values_t values;
    };
}
