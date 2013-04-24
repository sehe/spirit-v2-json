#define BOOST_SPIRIT_UNICODE
// #define PRETTY_PRINT // well, sort of; multiline for starters
#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include "JSON.hpp"
#include <iomanip>
// unicode, please
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/spirit/include/phoenix.hpp>

BOOST_FUSION_ADAPT_STRUCT(JSON::String, (std::wstring, value))
BOOST_FUSION_ADAPT_STRUCT(JSON::Number, (double, value))
BOOST_FUSION_ADAPT_STRUCT(JSON::Object, (JSON::Object::values_t, values))
BOOST_FUSION_ADAPT_STRUCT(JSON::Array,  (JSON::Array ::values_t, values))

namespace JSON {

    namespace {
        template <typename Ret, typename PolyFunc>
        struct VisitorWrap : boost::static_visitor<Ret> {
            VisitorWrap(PolyFunc&& f) : _f(std::forward<PolyFunc>(f)) {}

            template <typename T> Ret operator()(T const& v) const {
                return _f(v);
            }
            template <typename T, typename U> Ret operator()(T const& v, U const& w) const {
                return _f(v, w);
            }
        private:
            PolyFunc _f;
        };

        template <typename Ret = void, typename PolyFunc>
        VisitorWrap<Ret, PolyFunc> make_visitor(PolyFunc&& f) {
            return { std::forward<PolyFunc>(f) };
        }
    }

    std::wostream& operator<<(std::wostream& os, String const& v) {
        os << L'"';

        for (auto ch: v.value) {
            if      (ch == L'"' || ch == L'\\') os << L'\\' << ch;
            else if (ch == L'\b'              ) os << L"\\b";
            else if (ch == L'\n'              ) os << L"\\n";
            else if (ch == L'\r'              ) os << L"\\r";
            else if (ch == L'\f'              ) os << L"\\f";
            else if (ch>=0 && ch<=0x1f        ) 
                os << L"\\u" << std::setw(4) << std::setfill(L'0') << std::hex << ((int) ch);
            else
                os << ch;
        }

        return os << L'"';
    }

    std::wostream& operator<<(std::wostream& os, Number    const& v) { return os << v.value; }
    std::wostream& operator<<(std::wostream& os, Undefined const& v) { return os << L"undefined"; }
    std::wostream& operator<<(std::wostream& os, False     const& v) { return os << L"false";     }
    std::wostream& operator<<(std::wostream& os, Null      const& v) { return os << L"null";      }
    std::wostream& operator<<(std::wostream& os, True      const& v) { return os << L"true";      }
    std::wostream& operator<<(std::wostream& os, Value  const& v) {
        using boost::phoenix::arg_names::arg1;
        return boost::apply_visitor(make_visitor<std::wostream&>(os << arg1), v);
    }

#ifdef PRETTY_PRINT
    static const wchar_t brace_open   [] = L"\n{\n";
    static const wchar_t brace_close  [] = L"\n}\n";
    static const wchar_t bracket_open [] = L"\n[\n";
    static const wchar_t bracket_close[] = L"\n]\n";
    static const wchar_t value_sep    [] = L",\n";
#else
    static const wchar_t brace_open   [] = L"{";
    static const wchar_t brace_close  [] = L"}";
    static const wchar_t bracket_open [] = L"[";
    static const wchar_t bracket_close[] = L"]";
    static const wchar_t value_sep    [] = L",";
#endif

    std::wostream& operator<<(std::wostream& os, Object const& v) {
        int n = 0;
        os << brace_open;
        for(auto& x : v.values) {
            if (n++) os << value_sep;
            os << x.first << L':' << x.second;
        }
        return os << brace_close;
    }
    std::wostream& operator<<(std::wostream& os, Array const& v) {
        int n = 0;
        os << bracket_open;
        for(auto& x : v.values) {
            if (n++) os << value_sep;
            os << x;
        }
        return os << bracket_close;
    }

namespace qi         = boost::spirit::qi;
namespace phx        = boost::phoenix;
namespace encoding   = qi::standard_wide;
//namespace encoding = qi::unicode;

template <typename It, typename Skipper = encoding::space_type>
    struct parser : qi::grammar<It, JSON::Value(), Skipper>
{
    parser() : parser::base_type(json)
    {
        // 2.1 values
        value = qi::attr_cast<False> (qi::lit(L"false")) 
              | qi::attr_cast<Null>  (qi::lit(L"null")) 
              | qi::attr_cast<True>  (qi::lit(L"true"))
              | object
              | array
              | number
              | string
              ;

        // 2.2 objects
        object = L'{' >> -(member % L',') >> L'}';
        member = string >> L':' >> value;

        // 2.3 Arrays
        array = L'[' >> -(value % L',') >> L']';

        // 2.4.  Numbers
        // Note out spirit grammar takes a shortcut, as the RFC specification is more restrictive:
        //
        // However non of the above affect any structure characters (:,{}[] and double quotes) so it doesn't
        // matter for the current purpose. For full compliance, this remains TODO:
        //
        //    Numeric values that cannot be represented as sequences of digits
        //    (such as Infinity and NaN) are not permitted.
        //     number = [ minus ] int [ frac ] [ exp ]
        //     decimal-point = %x2E       ; .
        //     digit1-9 = %x31-39         ; 1-9
        //     e = %x65 / %x45            ; e E
        //     exp = e [ minus / plus ] 1*DIGIT
        //     frac = decimal-point 1*DIGIT
        //     int = zero / ( digit1-9 *DIGIT )
        //     minus = %x2D               ; -
        //     plus = %x2B                ; +
        //     zero = %x30                ; 0
        number = qi::double_; // shortcut :)

        // 2.5 Strings
        string = qi::lexeme [ L'"' >> *char_ >> L'"' ];

        static qi::uint_parser<wchar_t, 16, 4, 4> _4HEXDIG;

        char_ = +(
                ~encoding::char_(L"\"\\")) [ qi::_val += qi::_1 ] |
                   qi::lit(L"\x5C") >> (                    // \ (reverse solidus)
                   qi::lit(L"\x22") [ qi::_val += L'"'  ] | // "    quotation mark  U+0022
                   qi::lit(L"\x5C") [ qi::_val += L'\\' ] | // \    reverse solidus U+005C
                   qi::lit(L"\x2F") [ qi::_val += L'/'  ] | // /    solidus         U+002F
                   qi::lit(L"\x62") [ qi::_val += L'\b' ] | // b    backspace       U+0008
                   qi::lit(L"\x66") [ qi::_val += L'\f' ] | // f    form feed       U+000C
                   qi::lit(L"\x6E") [ qi::_val += L'\n' ] | // n    line feed       U+000A
                   qi::lit(L"\x72") [ qi::_val += L'\r' ] | // r    carriage return U+000D
                   qi::lit(L"\x74") [ qi::_val += L'\t' ] | // t    tab             U+0009
                   qi::lit(L"\x75")                         // uXXXX                U+XXXX
                        >> _4HEXDIG [ qi::_val += qi::_1 ]
                );

        // entry point
        json = value;

        BOOST_SPIRIT_DEBUG_NODES(
                (json)(value)(object)(member)(array)(number)(string)(char_));
    }

  private:
    qi::rule<It, std::pair<String, Value>(),  Skipper> member;
    qi::rule<It, JSON::Value(),  Skipper> json, value;
    qi::rule<It, JSON::Object(), Skipper> object;
    qi::rule<It, JSON::Array(),  Skipper> array;
    //
    qi::rule<It, Number()>  number;
    qi::rule<It, String()>  string;
    qi::rule<It, std::wstring()> char_;
};

template <typename It>
bool tryParseJson(It& f, It l) // note: first iterator gets updated
{
    Value discard;
    return tryParseJson(f,l,discard);
}

template <typename It>
bool tryParseJson(It& f, It l, JSON::Value& value) // note: first iterator gets updated
{
    static const parser<It, encoding::space_type> p;

    try
    {
        return qi::phrase_parse(f,l,p,encoding::space,value);
    } catch(const qi::expectation_failure<It>& e)
    {
        boost::utf8_output_iterator<std::ostream_iterator<char>> to_utf8(std::cerr);
        // expectation points not currently used, but we could tidy up the
        // grammar to bail on unexpected tokens (future)
        std::cerr << e.what() << L"'";
        std::copy(e.first, e.last, to_utf8); 
        std::cerr << "'\n";
        return false;
    }
}

std::wstring to_wstring(JSON::Value const& json) {
    return boost::lexical_cast<std::wstring>(json);
}

std::string to_string(JSON::Value const& json) {
    std::ostringstream oss;
    oss << json;
    return oss.str();
}

JSON::Value parse(std::wstring const& input) {
    auto f(begin(input)), l(end(input));

    JSON::Value parsed;
    if (!JSON::tryParseJson(f, l, parsed))
    {
        std::cerr << "whoops at position #" << std::distance(begin(input), f) << "\n";
    }

    return parsed;
}

Value readFrom(std::istream& is)
{
    is.unsetf(std::ios::skipws);
    boost::spirit::istream_iterator it(is), pte;

    typedef boost::u8_to_u32_iterator<decltype(it)> Conv2Utf32;
    Conv2Utf32 f(it), l(pte);

    JSON::Value parsed;
    if (!JSON::tryParseJson(f, l, parsed))
    {
        std::cerr << "whoops"; // TODO
    }

    return parsed;
}

Value readFrom(std::istream&& is)
{
    return readFrom(is);
}

Value readFrom(std::wistream& is)
{
    is.unsetf(std::ios::skipws);
    std::istreambuf_iterator<wchar_t> a(is.rdbuf()), b;
    return parse({ a, b }); // no streaming - no spirit::wistream_iterator AFAIK
}

Value readFrom(std::wistream&& is)
{ 
    return readFrom(is); 
}

std::ostream& operator<<(std::ostream& os, Value const& v)
{
    std::wostringstream oss;
    oss << v;
    auto wide = oss.str();

    std::copy(begin(wide), end(wide), boost::utf8_output_iterator<std::ostream_iterator<char>>(os)); 
    return os;
}
//std::wostream& operator<<(std::wostream&, Value const&);

} // namespace JSON

