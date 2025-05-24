#include "User.h"

User::User() {
    score = 0;
    matrix = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
}

User::User(const std::string& u, const std::string& p) {
    username = u;
    password = p;
    score = 0;
    matrix = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
}
