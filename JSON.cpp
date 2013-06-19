//#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_UNICODE
#include "JSON.hpp"
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/qi.hpp>
#include <iomanip>
// unicode, please
#include <boost/regex/pending/unicode_iterator.hpp>

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

    ///////////////////////////////////////////////////////////////
    // Qi Parser
    ///////////////////////////////////////////////////////////////

    namespace qi         = boost::spirit::qi;
    namespace encoding   = qi::standard_wide;
    //namespace encoding = qi::unicode;

    template <typename It, typename Skipper = encoding::space_type>
        struct parser : qi::grammar<It, Value(), Skipper>
    {
        parser() : parser::base_type(json)
        {
            truefalse.add
                (L"true", true)
                (L"false", false);

            // 2.1 values
            value = qi::attr_cast<Null>  (qi::lit(L"null")) 
                  | truefalse
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
            // Note our spirit grammar takes a shortcut, as the RFC specification is more restrictive:
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
            const static qi::real_parser<long double> ldbl;
            const static qi::int_parser <int64_t>     lint;
            number = qi::lexeme [ lint >> !qi::char_(".e0-9") ] | ldbl;

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
        qi::symbols<wchar_t, bool> truefalse;
        qi::rule<It, std::pair<std::wstring, Value>(),  Skipper> member;
        qi::rule<It, Value(),  Skipper> json, value;
        qi::rule<It, Object(), Skipper> object;
        qi::rule<It, Array(),  Skipper> array;
        //
        qi::rule<It, Value()>    number;
        qi::rule<It, std::wstring()> string;
        qi::rule<It, std::wstring()> char_;
    };

    ///////////////////////////////////////////////////////////////
    // Karma Generator
    ///////////////////////////////////////////////////////////////

    namespace karma      = boost::spirit::karma;
    namespace { namespace detail {
        std::wstring unicode_escape(wchar_t ch) {
            std::wostringstream owss;
            owss << L"\\u" << std::setw(4) << std::setfill(L'0') << std::hex << ((int) ch);
            return owss.str();
        } }

        BOOST_PHOENIX_ADAPT_FUNCTION(std::wstring, unicode_escape_, detail::unicode_escape, 1)
    }

    template <typename It, typename Delimiter = qi::unused_type>
        struct generator : karma::grammar<It, Value(), Delimiter>
    {
        generator() : generator::base_type(json)
        {
            const static karma::int_generator <int64_t>     integer;
            const static karma::real_generator<long double> long_double;

            truefalse.add
                (false, L"false")
                (true,  L"true");

            value = karma::attr_cast<Null>  (karma::lit(L"null")) 
                  | truefalse
                  | object
                  | array
                  | integer
                  | long_double
                  | string
                  ;

            object = L'{' << -(member % L',') << L'}';
            member = string << L':' << value;

            array = L'[' << -(value % L',') << L']';

            char_escape.add
                (L'"',  L"\\\"")
                (L'\\', L"\\\\")
              //(L'/',  L"\\/")
                (L'\b', L"\\b")
                (L'\f', L"\\f")
                (L'\n', L"\\n")
                (L'\r', L"\\r")
                (L'\t', L"\\t");

            using karma::_val;
            using karma::_1;

            unicode_escape = 
                karma::eps(_val >= 0x0 && _val <= 0x1f) << 
                encoding::string [ _1 = unicode_escape_(_val) ]
                ;

            char_ = char_escape | unicode_escape | encoding::char_;
            string = L'"' << *char_ << L'"';

            // entry point
            json = value;

            BOOST_SPIRIT_DEBUG_NODES( (json)(value)(object)(member)(array)(string)(char_)(unicode_escape));
        }

      private:
        karma::symbols<bool, std::wstring> truefalse;
        karma::rule<It, std::pair<std::wstring, Value>(),  Delimiter> member;
        karma::rule<It, Value(),  Delimiter>  json, value;
        karma::rule<It, Object(), Delimiter>  object;
        karma::rule<It, Array(),  Delimiter>  array;
        //
        karma::rule<It, std::wstring()>       string;
        karma::rule<It, wchar_t()>            char_;
        karma::rule<It, uint32_t()>           unicode_escape;
        karma::symbols<wchar_t, std::wstring> char_escape;
    };

    template <typename It>
    bool tryParseJson(It& f, It l, Value& value) // note: first iterator gets updated
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

    template <typename It>
    bool tryParseJson(It& f, It l) // note: first iterator gets updated
    {
        Value discard;
        return tryParseJson(f,l,discard);
    }

    std::wstring to_wstring(Value const& json) {
        std::wstring text;
        auto out = std::back_inserter(text);

        static const generator<decltype(out)> g;
        karma::generate(out, g, json);

        return text;
    }

    std::string to_string(Value const& json) {
        std::string result;
        auto out = std::back_inserter(result);

        boost::utf8_output_iterator<decltype(out)> convert(out);

        static const generator<decltype(convert)> g;
        karma::generate(convert, g, json);

        return result;
    }

    std::ostream& operator<<(std::ostream& os, Value const& v)
    {
        const auto wide = to_wstring(v);
        std::copy(begin(wide), end(wide), boost::utf8_output_iterator<std::ostream_iterator<char>>(os)); 
        return os;
    }

    Value parse(std::wstring const& input) {
        auto f(begin(input)), saved = f, l(end(input));

        Value parsed;
        if (!tryParseJson(f, l, parsed))
        {
            std::cerr << "whoops at position #" << std::distance(saved, f) << "\n";
        }

        return parsed;
    }


    Value parse(std::string const& input) {
        auto first(begin(input)), last(end(input));

        typedef boost::u8_to_u32_iterator<decltype(first)> Conv2Utf32;
        Conv2Utf32 f(first), saved = f, l(last);

        Value parsed;
        if (!tryParseJson(f, l, parsed))
        {
            std::cerr << "whoops at position #" << std::distance(saved, f) << "\n";
        }

        return parsed;
    }

    Value readFrom(std::istream& is)
    {
        is.unsetf(std::ios::skipws);
        boost::spirit::istream_iterator it(is), pte;

        typedef boost::u8_to_u32_iterator<decltype(it)> Conv2Utf32;
        Conv2Utf32 f(it), l(pte);

        Value parsed;
        if (!tryParseJson(f, l, parsed))
        {
            std::cerr << "whoops"; // TODO
        }

        return parsed;
    }

    Value readFrom(std::istream&& is)
    {
        return readFrom(is);
    }

} // namespace JSON

