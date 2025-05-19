#include "User.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

bool registerAccount(const std::string& username, const std::string& password) {
    std::filesystem::create_directory("users");
    std::string filepath = "users/" + username + ".json";

    // Nếu file tồn tại thì trả về false (đã có tài khoản)
    if (std::filesystem::exists(filepath)) return false;

    // Tạo JSON lưu username và password
    json j;
    j["username"] = username;
    j["password"] = password;

    std::ofstream ofs(filepath);
    if (!ofs) return false;

    ofs << j.dump(4); // ghi file đẹp (indent 4 space)
    ofs.close();
    return true;
}

bool loginAccount(const std::string& username, const std::string& password) {
    std::string filepath = "users/" + username + ".json";
    if (!std::filesystem::exists(filepath)) return false;

    std::ifstream ifs(filepath);
    if (!ifs) return false;

    json j;
    ifs >> j;
    ifs.close();

    // So sánh password
    if (!j.contains("password")) return false;
    return j["password"] == password;
}
