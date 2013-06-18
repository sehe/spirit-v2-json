#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <map>
#include <deque>

namespace JSON
{
    template <typename Tag> struct Literal 
    { 
        bool operator==(Literal const&) const { return true;  } 
        bool operator< (Literal const&) const { return false; } 
    };

    typedef Literal<struct tag_undefined> Undefined;
    typedef Literal<struct tag_false> False;
    typedef Literal<struct tag_null>  Null;
    typedef Literal<struct tag_true>  True;

    struct Object;
    struct Array;

    struct String { 
        std::wstring value; 
        String(const wchar_t* v) : value(v) { }
        String(std::wstring const& v = {}) : value(v) { }

        template <typename A1, typename A2, typename... Args>
            /*explicit*/ String(A1&& a1, A2&& a2, Args&&... args) 
                : value(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<Args>(args)...) 
            { }

        bool operator==(String const& o) const { return value == o.value; } // unordered_map
        bool operator< (String const& o) const { return value < o.value;  } // map
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
    typedef boost::variant<
            Undefined, // not legal as a JSON result
            False, 
            Null, 
            True, 
            boost::recursive_wrapper<Object>, 
            boost::recursive_wrapper<Array>,
            String,
            int64_t,
            long double
        > Value;

    struct Object
    {
        typedef std::/*unordered_*/map<String, Value> values_t;
        values_t values;

        Object() = default;
        explicit Object(std::initializer_list<values_t::value_type> init) : values(init) { }

        Value&       operator[](String const& key)       { return values[key]; }
        Value const& operator[](String const& key) const { return values.at(key); }

        bool operator==(Object const& o) const { return values == o.values; }
    };

    struct Array
    {
        typedef std::deque<Value> values_t;
        values_t values;

        Array() = default;
        /*explicit*/ Array(std::initializer_list<Value> values) : values(values) {}

        template <typename T> Value&       operator[](T&& key)       
            { return values[std::forward<T>(key)]; }

        template <typename T> Value const& operator[](T&& key) const 
            { return values[std::forward<T>(key)]; }

        bool operator==(Array const& o) const { return values == o.values; }
    };

    static inline Array&              as_array  (Value& v)       { return boost::get<Array>(v);        } 
    static inline Object&             as_object (Value& v)       { return boost::get<Object>(v);       } 
    static inline std::wstring&       as_wstring(Value& v)       { return boost::get<String>(v).value; } 
    static inline long double&        as_double (Value& v)       { return boost::get<long double>(v);  } 
    static inline int64_t&            as_int64  (Value& v)       { return boost::get<int64_t>(v);      } 
    static inline Array const&        as_array  (Value const& v) { return boost::get<Array>(v);        } 
    static inline Object const&       as_object (Value const& v) { return boost::get<Object>(v);       } 
    static inline std::wstring const& as_wstring(Value const& v) { return boost::get<String>(v).value; } 
    static inline long double const&  as_double (Value const& v) { return boost::get<long double>(v);  } 
    static inline int64_t const&      as_int64  (Value const& v) { return boost::get<int64_t>(v);      } 

    // standard char streams are assumed to be utf8
    Value readFrom(std::istream&);
    Value readFrom(std::istream&&);

    Value parse(std::wstring const&);
    Value parse(std:: string const&);
    std::wstring to_wstring(Value const&);
    std::string  to_string (Value const&);

    std::ostream&  operator<<(std::ostream&,  Value const&);
}
