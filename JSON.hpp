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
        std::wstring value; 
        bool operator==(String const& s) const { return value == s.value; }
    };
}

namespace std
{
    template <> struct hash<JSON::String>
    {
        size_t operator()(JSON::String const& s) const {
            static std::hash<std::wstring> _hash;
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

        Object() = default;
        explicit Object(std::initializer_list<values_t::value_type> init) : values(init) { }

        template <typename T> Value&       operator[](T&& key)       
            { return values[{std::forward<T>(key)}]; }

        template <typename T> Value const& operator[](T&& key) const 
            { return values[{std::forward<T>(key)}]; }
    };

    struct Array
    {
        typedef std::deque<Value> values_t;
        values_t values;

        template <typename T> Value&       operator[](T&& key)       
            { return values[std::forward<T>(key)]; }

        template <typename T> Value const& operator[](T&& key) const 
            { return values[std::forward<T>(key)]; }
    };

    static inline Array&              as_array  (Value& v)       { return boost::get<Array>(v);        } 
    static inline Object&             as_object (Value& v)       { return boost::get<Object>(v);       } 
    static inline std::wstring&       as_wstring(Value& v)       { return boost::get<String>(v).value; } 
    static inline double&             as_double (Value& v)       { return boost::get<Number>(v).value; } 
    static inline Array const&        as_array  (Value const& v) { return boost::get<Array>(v);        } 
    static inline Object const&       as_object (Value const& v) { return boost::get<Object>(v);       } 
    static inline std::wstring const& as_wstring(Value const& v) { return boost::get<String>(v).value; } 
    static inline double const&       as_double (Value const& v) { return boost::get<Number>(v).value; } 

    // standard char streams are assumed to be utf8
    Value readFrom(std::istream&);
    Value readFrom(std::istream&&);

    // standard wides streams are assumed to be utf32
    Value parse(std::wstring const&);
    Value readFrom(std::wistream&);
    Value readFrom(std::wistream&&);

    std::wstring to_wstring(Value const&);
    std::string  to_string(Value const&);

    std::ostream&  operator<<(std::ostream&, Value const&);
    std::wostream& operator<<(std::wostream&, Value const&);
}
