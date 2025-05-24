#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

struct User {
    std::string username;
    std::string password;
    int score;
    std::vector<std::vector<int>> matrix;

    User();
    User(const std::string& u, const std::string& p);
};

#endif
