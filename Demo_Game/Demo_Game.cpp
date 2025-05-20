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

std::map<std::string, UserData> users;
std::mutex mtx;

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

crow::response serveStaticFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) return crow::response(404, "File not found");
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

void addCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    loadUsers();

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return serveStaticFile("./static/index.html");
        });

    CROW_ROUTE(app, "/static/<string>")([](const std::string& filename) {
        std::string filepath = "./static/" + filename;
        return serveStaticFile(filepath);
        });

    CROW_ROUTE(app, "/api/register").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username") || !body.has("password")) {
            return crow::response(400, "Dữ liệu không hợp lệ");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        std::lock_guard<std::mutex> lock(mtx);
        if (users.count(username) > 0) {
            return crow::response(409, "Username đã tồn tại");
        }

        UserData newUser;
        newUser.username = username;
        newUser.password = password;
        newUser.score = 0;
        newUser.matrix = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));

        users[username] = newUser;
        saveUsers();

        return crow::response(200, "Đăng ký thành công");
        });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, R"({"error": "Invalid JSON"})");

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        std::lock_guard<std::mutex> lock(mtx);
        if (users.count(username) == 0)
            return crow::response(404, R"({"error": "Username không tồn tại"})");

        if (users[username].password != password)
            return crow::response(401, R"({"error": "Sai mật khẩu"})");

        return crow::response(200, R"({"reply": "Đăng nhập thành công"})");
        });

    CROW_ROUTE(app, "/api/save").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        if (!body.contains("username") || !body.contains("matrix") || !body.contains("score")) {
            return crow::response(400, "Thiếu thông tin");
        }

        std::string username = body["username"];
        GameState state;
        state.score = body["score"];

        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                state.matrix[i][j] = body["matrix"][i][j];

        bool saved = saveGameState(username, state);

        std::lock_guard<std::mutex> lock(mtx);
        if (users.count(username)) {
            users[username].score = state.score;
            users[username].matrix = body["matrix"].get<std::vector<std::vector<int>>>();
            saveUsers();
        }

        if (saved) return crow::response(200, "Saved");
        else return crow::response(500, "Không thể lưu");
        });

    CROW_ROUTE(app, "/api/load")([](const crow::request& req) {
        auto url_params = crow::query_string(req.url);
        auto username = url_params.get("username");
        if (!username) return crow::response(400, R"({"error": "Thiếu username"})");

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
            return createJsonResponse(res);
        }
        return crow::response(404, R"({"error": "Không tìm thấy game"})");
        });

    CROW_ROUTE(app, "/api/top").methods("GET"_method)([](const crow::request& req) {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<UserData> userList;
        for (auto& [k, v] : users)
            userList.push_back(v);

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

        bool ok = saveGameState(username, newState);
        if (ok) return crow::response(200, "New game started");
        else return crow::response(500, "Lỗi tạo game mới");
        });

    CROW_ROUTE(app, "/resume")([](const crow::request& req) {
        auto url_params = crow::query_string(req.url);
        auto username = url_params.get("username");
        if (!username) return crow::response(400, "Missing username");

        GameState state;
        if (loadGameState(username, state)) return crow::response(200, "Resume OK");
        else return crow::response(404, "No saved game");
        });

    std::cout << "Server running on http://localhost:18080\n";
    app.port(18080).multithreaded().run();

    return 0;
}
