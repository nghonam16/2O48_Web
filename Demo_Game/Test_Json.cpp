#include "nlohmann/json.hpp"
#include <iostream>

int main() {
    nlohmann::json j;
    j["project"] = "2048";
    j["language"] = "C++";
    std::cout << j.dump(4) << std::endl;
    return 0;
}
