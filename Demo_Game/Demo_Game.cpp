#include <crow.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <mutex>
#include <json.hpp>

using json = nlohmann::json;

struct UserData {
    std::string username;
    std::string password; // Bạn có thể mã hóa password cho bảo mật hơn
    std::vector<std::vector<int>> matrix;
    int score = 0;
};

std::map<std::string, UserData> users; // lưu tạm trong bộ nhớ
std::mutex mtx;

// Hàm load dữ liệu user từ file (ví dụ file users.json)
void loadUsers() {
    std::lock_guard<std::mutex> lock(mtx);
    std::ifstream in("users.json");
    if (in) {
        json j;
        in >> j;
        for (auto& [k, v] : j.items()) {
            UserData ud;
            ud.username = k;
            ud.password = v["password"];
            ud.score = v["score"];
            ud.matrix = v["matrix"].get<std::vector<std::vector<int>>>();
            users[k] = ud;
        }
    }
}

// Hàm lưu dữ liệu user vào file
void saveUsers() {
    std::lock_guard<std::mutex> lock(mtx);
    json j;
    for (auto& [k, v] : users) {
        j[k] = {
            {"password", v.password},
            {"score", v.score},
            {"matrix", v.matrix}
        };
    }
    std::ofstream out("users.json");
    out << j.dump(4);
}

crow::response createJsonResponse(const json& j) {
    crow::response res;
    res.set_header("Content-Type", "application/json");
    res.write(j.dump());
    return res;
}

int main() {
    loadUsers();

    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/hello").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string name = body.value("name", "player");
        json res = { {"reply", "Hello, " + name + "!"} };
        return createJsonResponse(res);
        });

    CROW_ROUTE(app, "/api/register").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body.value("username", "");
        std::string password = body.value("password", "");

        if (username.empty() || password.empty()) {
            return createJsonResponse({ {"error", "Username hoặc password không được để trống"} });
        }

        std::lock_guard<std::mutex> lock(mtx);
        if (users.count(username)) {
            return createJsonResponse({ {"error", "Username đã tồn tại"} });
        }

        UserData newUser;
        newUser.username = username;
        newUser.password = password;
        newUser.score = 0;
        newUser.matrix = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
        users[username] = newUser;

        saveUsers();

        return createJsonResponse({ {"reply", "Đăng ký thành công"} });
        });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body.value("username", "");
        std::string password = body.value("password", "");

        std::lock_guard<std::mutex> lock(mtx);
        if (!users.count(username)) {
            return createJsonResponse({ {"error", "Username không tồn tại"} });
        }
        if (users[username].password != password) {
            return createJsonResponse({ {"error", "Sai mật khẩu"} });
        }

        return createJsonResponse({ {"reply", "Đăng nhập thành công"} });
        });

    CROW_ROUTE(app, "/api/save").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body.value("username", "");
        auto matrix = body.value("matrix", json::array());
        int score = body.value("score", 0);

        std::lock_guard<std::mutex> lock(mtx);
        if (!users.count(username)) {
            return createJsonResponse({ {"error", "User không tồn tại"} });
        }

        // Cập nhật trạng thái game
        users[username].matrix = matrix.get<std::vector<std::vector<int>>>();
        users[username].score = score;

        saveUsers();

        return createJsonResponse({ {"reply", "Lưu trạng thái thành công"} });
        });

    CROW_ROUTE(app, "/api/load").methods("GET"_method)([](const crow::request& req) {
        auto url_params = crow::query_string(req.url);
        std::string username = url_params.get("username") ? url_params.get("username") : "";

        std::lock_guard<std::mutex> lock(mtx);
        if (!users.count(username)) {
            return createJsonResponse({ {"error", "User không tồn tại"} });
        }

        json res = {
            {"matrix", users[username].matrix},
            {"score", users[username].score}
        };
        return createJsonResponse(res);
        });

    CROW_ROUTE(app, "/api/top").methods("GET"_method)([](const crow::request& req) {
        std::lock_guard<std::mutex> lock(mtx);
        // Lấy 10 user có điểm cao nhất
        std::vector<UserData> userList;
        for (auto& [k, v] : users) {
            userList.push_back(v);
        }
        std::sort(userList.begin(), userList.end(), [](const UserData& a, const UserData& b) {
            return a.score > b.score;
            });

        json res;
        res["players"] = json::array();
        int limit = std::min(10, (int)userList.size());
        for (int i = 0; i < limit; ++i) {
            res["players"].push_back({
                {"username", userList[i].username},
                {"score", userList[i].score}
                });
        }
        return createJsonResponse(res);
        });

    std::cout << "Server running on http://localhost:18080\n";
    app.port(18080).multithreaded().run();

    return 0;
}
