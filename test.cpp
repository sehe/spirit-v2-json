#include "JSON.hpp"

namespace {
    using namespace JSON;
    struct make_love : boost::static_visitor<void>
    {
        bool top_level;
        make_love(bool top_level = true) : top_level(top_level) {}

        void operator()(Object& a, Object const& b) const
        {
            for(auto el: b.values)
            {
                if (a.values.end() != a.values.find(el.first))
                {
                    boost::apply_visitor(make_love(false), a[el.first], el.second);
                } else
                {
                    a.values[el.first] = el.second;
                }
            }
        }

        template<typename T> void operator()(T& a, T const& b)  const
            {
                if (!top_level)
                    a = b;
            }

        template<typename T, typename U> void operator()(T& a, U const& b)  const
            {
                std::cerr << "Warning: incompatible arguments (" << typeid(T).name() << ", " << typeid(U).name() << ")\n";
            }
    };
}

int main()
{
    auto jsonA = JSON::parse("{ \"a\":\"1\", \"b\":\"2\", \"c\":{\"a\":\"1\", \"b\":\"2\"} }");
    auto jsonB = JSON::parse("{ \"b\":\"2new\", \"c\":{\"a\":\"1new\"} }");

    boost::apply_visitor(make_love(), jsonA, jsonB);

    std::cout << jsonA;
}
