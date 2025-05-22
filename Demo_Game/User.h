#pragma once
#include <string>
#include <vector>

struct UserData {
    std::string username;
    std::string password;
    int score;
    std::vector<std::vector<int>> matrix;
};