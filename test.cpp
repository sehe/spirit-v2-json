//#define BOOST_SPIRIT_DEBUG
//#define PRETTY_PRINT // well, sort of; multiline for starters
#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include "JSON.hpp"
#include <iomanip>
#include <fstream>

BOOST_FUSION_ADAPT_STRUCT(JSON::String, (std::string, value))
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

    // TODO FIXME escapes...
    std::ostream& operator<<(std::ostream& os, String const& v) { 
        os << '"';

        bool cchar_pending = false;
        char pending;

        for (auto ch: v.value) {
            if (cchar_pending) {
                cchar_pending = false;
                os << "\\u" << std::setw(2) << std::setfill('0') << std::hex << ((int) pending)
                            << std::setw(2) << std::setfill('0') << std::hex << ((int) ch);
            } 
            else if (ch == '"' || ch == '\\')
                os << '\\' << ch;
            else if (ch>=0 && ch<=0x1f) {
                cchar_pending = true;
                pending = ch;
            }
            else // TODO unicode
                os << ch;
        }

        return os << '"'; 
    }

    std::ostream& operator<<(std::ostream& os, Number    const& v) { return os << v.value; }
    std::ostream& operator<<(std::ostream& os, Undefined const& v) { return os << "undefined"; }
    std::ostream& operator<<(std::ostream& os, False     const& v) { return os << "false";     }
    std::ostream& operator<<(std::ostream& os, Null      const& v) { return os << "null";      }
    std::ostream& operator<<(std::ostream& os, True      const& v) { return os << "true";      }
    std::ostream& operator<<(std::ostream& os, Value  const& v) { 
        using boost::phoenix::arg_names::arg1;
        return boost::apply_visitor(make_visitor<std::ostream&>(os << arg1), v);
    }

#ifdef PRETTY_PRINT
    static const char brace_open   [] = "\n{\n";
    static const char brace_close  [] = "\n}\n";
    static const char bracket_open [] = "\n[\n";
    static const char bracket_close[] = "\n]\n";
    static const char value_sep    [] = ",\n";
#else                                 
    static const char brace_open   [] = "{";
    static const char brace_close  [] = "}";
    static const char bracket_open [] = "[";
    static const char bracket_close[] = "]";
    static const char value_sep    [] = ",";
#endif

    std::ostream& operator<<(std::ostream& os, Object const& v) {
        int n = 0;
        os << brace_open;
        for(auto& x : v.values) {
            if (n++) os << value_sep;
            os << x.first << ':' << x.second;
        }
        return os << brace_close;
    }
    std::ostream& operator<<(std::ostream& os, Array const& v) {
        int n = 0;
        os << bracket_open;
        for(auto& x : v.values) {
            if (n++) os << value_sep;
            os << x;
        }
        return os << bracket_close;
    }

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

template <typename It, typename Skipper = qi::space_type>
    struct parser : qi::grammar<It, JSON::Value(), Skipper>
{
    parser() : parser::base_type(json)
    {
        // 2.1 values
        value = qi::attr_cast<False> (qi::lit("false")) 
              | qi::attr_cast<Null>  (qi::lit("null")) 
              | qi::attr_cast<True>  (qi::lit("true"))
              | object 
              | array 
              | number 
              | string
              ;

        // 2.2 objects
        object = '{' >> -(member % ',') >> '}';
        member = string >> ':' >> value;

        // 2.3 Arrays
        array = '[' >> -(value % ',') >> ']';

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
        string = qi::lexeme [ '"' >> *char_ >> '"' ];

        static qi::uint_parser<uint32_t, 16, 4, 4> _4HEXDIG;

        char_ = +(~qi::char_("\"\\")) [ qi::_val += qi::_1 ] |
                   qi::lit("\x5C") >> (   // \ (reverse solidus)
                   qi::lit("\x22") [ qi::_val += '"'  ] | // "    quotation mark  U+0022
                   qi::lit("\x5C") [ qi::_val += '\\' ] | // \    reverse solidus U+005C
                   qi::lit("\x2F") [ qi::_val += '/'  ] | // /    solidus         U+002F
                   qi::lit("\x62") [ qi::_val += '\b' ] | // b    backspace       U+0008
                   qi::lit("\x66") [ qi::_val += '\f' ] | // f    form feed       U+000C
                   qi::lit("\x6E") [ qi::_val += '\n' ] | // n    line feed       U+000A
                   qi::lit("\x72") [ qi::_val += '\r' ] | // r    carriage return U+000D
                   qi::lit("\x74") [ qi::_val += '\t' ] | // t    tab             U+0009
                   qi::lit("\x75") >> _4HEXDIG [ qi::_val += phx::static_cast_<char>((qi::_1 >> 8) & 0xff),
                                                 qi::_val += phx::static_cast_<char>((qi::_1)      & 0xff) ] // uXXXX                U+XXXX
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
    qi::rule<It, std::string()> char_;
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
    static const parser<It, qi::space_type> p;

    try
    {
        return qi::phrase_parse(f,l,p,qi::space,value);
    } catch(const qi::expectation_failure<It>& e)
    {
        // expectation points not currently used, but we could tidy up the grammar to bail on unexpected tokens
        std::string frag(e.first, e.last);
        std::cerr << e.what() << "'" << frag << "'\n";
        return false;
    }
}

} // namespace JSON

namespace { // TEST utils
    std::string to_string(JSON::Value const& json) {
        return boost::lexical_cast<std::string>(json);
    }

    JSON::Value parse(std::string const& input) {
        auto f(begin(input)), l(end(input));

        JSON::Value parsed;
        if (!JSON::tryParseJson(f, l, parsed))
            throw "whoops";

        return parsed;
    }

    JSON::Value roundtrip(JSON::Value const& given) {
        return parse(to_string(given));
    }
}

int main()
{
    // read full stdin
    std::ifstream ifs("testcases/test1.json");
    ifs.unsetf(std::ios::skipws);
    std::istream_iterator<char> it(ifs), pte;
    const std::string input(it, pte);

    auto value = parse(input);

    std::cout << "Dump of value:\n======================================\n" << value << "\n";

    const auto verify = roundtrip(value);
    if (to_string(value) == to_string(verify))
        std::cout << "Roundtrip success!\n";
    else
    {
        std::cout << "Roundtrip FAILED:\n";
        std::cout << "Dump of verify:\n======================================\n" << verify << "\n";
    }

    const auto verify2 = roundtrip(verify);
    if (to_string(value) == to_string(verify2))
        std::cout << "Roundtrip #2 success!\n";
    else
    {
        std::cout << "Roundtrip FAILED:\n";
        std::cout << "Dump of verify2:\n======================================\n" << verify2 << "\n";
    }
}
