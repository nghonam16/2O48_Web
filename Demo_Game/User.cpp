#include "User.h"
#include <fstream>
#include <filesystem>

bool registerAccount(const std::string& username, const std::string& password) {
    std::filesystem::create_directory("users");

    std::ifstream infile("users/" + username + ".bin", std::ios::binary);
    if (infile.good()) return false;

    std::ofstream outfile("users/" + username + ".bin", std::ios::binary);
    if (!outfile) return false;

    size_t nameLen = username.size();
    size_t passLen = password.size();

    outfile.write(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
    outfile.write(username.c_str(), nameLen);

    outfile.write(reinterpret_cast<char*>(&passLen), sizeof(size_t));
    outfile.write(password.c_str(), passLen);

    outfile.close();
    return true;
}

bool loginAccount(const std::string& username, const std::string& password) {
    std::ifstream infile("users/" + username + ".bin", std::ios::binary);
    if (!infile) return false;

    size_t nameLen, passLen;
    std::string storedUsername, storedPassword;

    infile.read(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
    storedUsername.resize(nameLen);
    infile.read(&storedUsername[0], nameLen);

    infile.read(reinterpret_cast<char*>(&passLen), sizeof(size_t));
    storedPassword.resize(passLen);
    infile.read(&storedPassword[0], passLen);

    infile.close();
    return storedPassword == password;
}
