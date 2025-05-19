#include <crow.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <mutex>
#include <json.hpp>
#include <algorithm>

using json = nlohmann::json;

struct UserData {
    std::string username;
    std::string password;
    std::vector<std::vector<int>> matrix;
    int score = 0;
};

struct GameState {
    int matrix[4][4];
    int score;
};

bool loadGameState(const std::string& username, GameState& state) {
    std::ifstream inFile("save_" + username + ".json");
    if (!inFile.is_open()) return false;

    json j;
    inFile >> j;
    inFile.close();

    if (j.contains("matrix") && j.contains("score")) {
        for (int i = 0; i < 4; ++i)
            for (int k = 0; k < 4; ++k)
                state.matrix[i][k] = j["matrix"][i][k];
        state.score = j["score"];
        return true;
    }
    return false;
}

bool saveGameState(const std::string& username, const GameState& state) {
    json j;
    j["matrix"] = json::array();
    for (int i = 0; i < 4; ++i) {
        j["matrix"].push_back(json::array());
        for (int k = 0; k < 4; ++k)
            j["matrix"][i].push_back(state.matrix[i][k]);
    }
    j["score"] = state.score;

    std::ofstream outFile("save_" + username + ".json");
    if (!outFile.is_open()) return false;
    outFile << j.dump(4);
    outFile.close();
    return true;
}

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

// Hàm trả về file tĩnh theo đường dẫn trong thư mục ./static
crow::response serveStaticFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) {
        return crow::response(404, "File not found");
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    crow::response res;
    if (filepath.ends_with(".html")) res.set_header("Content-Type", "text/html");
    else if (filepath.ends_with(".js")) res.set_header("Content-Type", "application/javascript");
    else if (filepath.ends_with(".css")) res.set_header("Content-Type", "text/css");
    else if (filepath.ends_with(".json")) res.set_header("Content-Type", "application/json");
    else res.set_header("Content-Type", "application/octet-stream");

    res.write(content);
    return res;
}

int main() {
    loadUsers();

    crow::SimpleApp app;

    // Phục vụ trang index.html khi truy cập /
    CROW_ROUTE(app, "/")([]() {
        return serveStaticFile("./static/index.html");
        });

    // Phục vụ các file tĩnh từ /static/...
    CROW_ROUTE(app, "/static/<string>")([](const std::string& filename) {
        std::string filepath = "./static/" + filename;
        return serveStaticFile(filepath);
        });

    // Các API (giữ nguyên như bạn đã viết)
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

        if (!body.contains("username") || !body.contains("matrix") || !body.contains("score")) {
            return crow::response(400, "Invalid data");
        }

        std::string username = body["username"];
        GameState state;
        auto mat = body["matrix"];

        for (int i = 0; i < 4; ++i)
            for (int k = 0; k < 4; ++k)
                state.matrix[i][k] = mat[i][k];

        state.score = body["score"];

        bool ok = saveGameState(username, state);

        if (ok) {
            json res;
            res["reply"] = "Đã lưu thành công!";
            return crow::response(200, res.dump());
        }
        else {
            return crow::response(500, "Lỗi khi lưu");
        }
        });


    CROW_ROUTE(app, "/api/load")
        ([](const crow::request& req) {
        auto url_params = crow::query_string(req.url);
        auto username = url_params.get("username");
        if (!username) return crow::response(400, "Missing username");

        GameState state;
        if (loadGameState(username, state)) {
            json res;
            res["matrix"] = json::array();
            for (int i = 0; i < 4; ++i) {
                res["matrix"].push_back(json::array());
                for (int k = 0; k < 4; ++k)
                    res["matrix"][i].push_back(state.matrix[i][k]);
            }
            res["score"] = state.score;
            return crow::response(200, res.dump());
        }
        // Nếu chưa có file lưu, trả về dữ liệu trống (null)
        json empty;
        empty["matrix"] = nullptr;
        empty["score"] = nullptr;
        return crow::response(200, empty.dump());
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

    CROW_ROUTE(app, "/new-game").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        if (!body.contains("username")) return crow::response(400, "Missing username");

        std::string username = body["username"];
        GameState newState = { 0 };
        newState.score = 0;
        // Khởi tạo matrix toàn 0

        bool ok = saveGameState(username, newState);
        if (ok) return crow::response(200, "New game started");
        else return crow::response(500, "Lỗi tạo game mới");
    });

    CROW_ROUTE(app, "/resume")([](const crow::request& req) {
        auto url_params = crow::query_string(req.url);
        auto username = url_params.get("username");
        if (!username) return crow::response(400, "Missing username");

        GameState state;
        if (loadGameState(username, state)) {
            return crow::response(200, "Resume OK");
        }
        else {
            return crow::response(404, "No saved game");
        }
    });



    std::cout << "Server running on http://localhost:18080\n";

    app.port(18080).multithreaded().run();

    return 0;
}
