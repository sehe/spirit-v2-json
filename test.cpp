#include "JSON.hpp"

namespace {
    using namespace JSON;
    struct make_love : boost::static_visitor<bool>
    {
        bool operator()(Object& a, Object const& b) const {
            for(auto el: b.values) 
                recurse(a[el.first], el.second);
            return true;
        }
        template<typename T, typename U> bool operator()(T& a, U const& b)  const 
            { return false; }

      private:
        void recurse(Value& a, Value const& b) const {
            if (!boost::apply_visitor(*this, a, b))
                a = b;
        }
    };
}

int main()
{
    auto jsonA = JSON::parse("{ \"a\":\"1\", \"b\":\"2\", \"c\":{\"a\":\"1\", \"b\":\"2\"} }");
    auto jsonB = JSON::parse("{ \"b\":42, \"c\":{\"a\":\"1new\"}, \"q\":[3.14,null] }");

    if (boost::apply_visitor(make_love(), jsonA, jsonB))
        std::cout << "Merged: " << jsonA;
    else
        std::cerr << "Couldn't merge '" << jsonA << "' with '" << jsonB << "'\n";
}
