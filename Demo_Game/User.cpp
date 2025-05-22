#include <fstream>
#include <map>
#include "User.h"

extern std::map<std::string, UserData> users;

void saveUsers() {
    std::ofstream out("users.dat", std::ios::binary);
    if (!out) return;

    int userCount = users.size();
    out.write(reinterpret_cast<char*>(&userCount), sizeof(userCount));

    for (const auto& [username, user] : users) {
        // Ghi username
        int nameLen = user.username.size();
        out.write(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        out.write(user.username.c_str(), nameLen);

        // Ghi password
        int passLen = user.password.size();
        out.write(reinterpret_cast<char*>(&passLen), sizeof(passLen));
        out.write(user.password.c_str(), passLen);

        // Ghi điểm
        out.write(reinterpret_cast<const char*>(&user.score), sizeof(user.score));

        // Ghi ma trận 4x4
        for (const auto& row : user.matrix) {
            for (int val : row) {
                out.write(reinterpret_cast<const char*>(&val), sizeof(val));
            }
        }
    }

    out.close();
}
