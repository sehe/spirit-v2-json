// #define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

template <typename It, typename Skipper = qi::space_type>
    struct parser : qi::grammar<It, Skipper>
{
    parser() : parser::base_type(json)
    {
        // 2.1 values
        value = qi::lit("false") | "null" | "true" | object | array | number | string;

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

        static const qi::uint_parser<uint32_t, 16, 4, 4> _4HEXDIG;

        char_ = ~qi::char_("\"\\") |
               qi::char_("\x5C") >> (       // \ (reverse solidus)
                   qi::char_("\x22") |      // "    quotation mark  U+0022
                   qi::char_("\x5C") |      // \    reverse solidus U+005C
                   qi::char_("\x2F") |      // /    solidus         U+002F
                   qi::char_("\x62") |      // b    backspace       U+0008
                   qi::char_("\x66") |      // f    form feed       U+000C
                   qi::char_("\x6E") |      // n    line feed       U+000A
                   qi::char_("\x72") |      // r    carriage return U+000D
                   qi::char_("\x74") |      // t    tab             U+0009
                   qi::char_("\x75") >> _4HEXDIG )  // uXXXX                U+XXXX
               ;

        // entry point
        json = value;

        BOOST_SPIRIT_DEBUG_NODES(
                (json)(value)(object)(member)(array)(number)(string)(char_));
    }

  private:
    qi::rule<It, Skipper> json, value, object, member, array, number, string;
    qi::rule<It> char_;
};

template <typename It>
bool tryParseAsJson(It& f, It l) // note: first iterator gets updated
{
    static const parser<It, qi::space_type> p;

    try
    {
        return qi::phrase_parse(f,l,p,qi::space);
    } catch(const qi::expectation_failure<It>& e)
    {
        // expectation points not currently used, but we could tidy up the grammar to bail on unexpected tokens
        std::string frag(e.first, e.last);
        std::cerr << e.what() << "'" << frag << "'\n";
        return false;
    }
}

int main()
{
#if 0
    // read full stdin
    std::cin.unsetf(std::ios::skipws);
    std::istream_iterator<char> it(std::cin), pte;
    const std::string input(it, pte);

    // set up parse iterators
    auto f(begin(input)), l(end(input));
#else
    const std::string input("foo([1, 2, 3], \"some more stuff\")");

    // set to start of JSON
    auto f(begin(input)), l(end(input));
    std::advance(f, 4);
#endif

    bool ok = tryParseAsJson(f, l); // updates f to point after the start of valid JSON

    if (ok) 
        std::cout << "Non-JSON part of input starts after valid JSON: '" << std::string(f, l) << "'\n";
    return ok? 0 : 255;
}
