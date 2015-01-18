#include "json.hpp"
#include <iostream>

void ToggleVisible(JSON::Value& json) {
    for (auto& i : as_array(as_object(json)["array"])) {
        auto& o = as_object(i);
        if (o.has_key("id") && (o["id"] == JSON::Value {"stackoverflow"})) {
            o["visible"] = JSON::Bool(!as_boolean(o["visible"]));
        }
    }
}

int main() {
    auto doc = JSON::parse(R"(
        {
            "schemaVersion": 1,
            "array": [{
            },
            {
                "id": "stackoverflow",
                "visible": true
            }]
        }
        )");

    ToggleVisible(doc);
    std::cout << doc << "\n";

    ToggleVisible(doc);
    std::cout << doc << "\n";
}
