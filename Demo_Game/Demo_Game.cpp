#include "crow_all.h"
#include "User.h"
#include "TopPlayer.h"
#include <fstream>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include <json.hpp>

using json = nlohmann::json;

std::unordered_map<std::string, User> users;

void saveUsers() {
    json j;
    for (const auto& pair : users) {
        j[pair.first] = {
            {"username", pair.second.username},
            {"password", pair.second.password},
            {"score", pair.second.score},
            {"matrix", pair.second.matrix}
        };
    }
    std::ofstream file("users.json");
    if (file.is_open()) {
        file << j.dump(4);
    }
}

void loadUsers() {
    std::ifstream file("users.json");
    if (file.is_open()) {
        json j;
        file >> j;
        for (auto& [key, value] : j.items()) {
            User user;
            user.username = value["username"];
            user.password = value["password"];
            user.score = value["score"];
            user.matrix = value["matrix"].get<std::vector<std::vector<int>>>();
            users[user.username] = user;
        }
    }
}

int main() {
    crow::SimpleApp app;
    loadUsers();

    CROW_ROUTE(app, "/")([]() {
        return crow::response(200, "Crow Backend for 2048 Game");
        });

    CROW_ROUTE(app, "/api/register").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        std::string password = body["password"];

        if (users.find(username) != users.end()) {
            return crow::response(400, "Username already exists");
        }

        User user(username, password);
        users[username] = user;
        saveUsers();
        return crow::response(200, "Register successful");
        });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        std::string password = body["password"];

        if (users.find(username) == users.end() || users[username].password != password) {
            return crow::response(401, "Invalid username or password");
        }

        return crow::response(200, "Login successful");
        });

    CROW_ROUTE(app, "/api/save").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        int score = body["score"];
        std::vector<std::vector<int>> matrix = body["matrix"];

        if (users.find(username) == users.end()) {
            return crow::response(404, "User not found");
        }

        users[username].score = score;
        users[username].matrix = matrix;
        saveUsers();
        return crow::response(200, "Game saved");
        });

    CROW_ROUTE(app, "/api/load").methods("GET"_method)([](const crow::request& req) {
        auto username = req.url_params.get("username");
        if (!username || users.find(username) == users.end()) {
            return crow::response(404, "User not found");
        }

        json j;
        j["username"] = users[username].username;
        j["score"] = users[username].score;
        j["matrix"] = users[username].matrix;
        return crow::response{ j.dump() };
        });

    CROW_ROUTE(app, "/new-game").methods("POST"_method)([](const crow::request& req) {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        if (users.find(username) == users.end()) {
            return crow::response(404, "User not found");
        }

        users[username].score = 0;
        users[username].matrix = { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} };
        saveUsers();
        return crow::response(200, "New game started");
        });

    CROW_ROUTE(app, "/resume").methods("GET"_method)([](const crow::request& req) {
        auto username = req.url_params.get("username");
        if (!username || users.find(username) == users.end()) {
            return crow::response(404, "User not found");
        }

        json j;
        j["username"] = users[username].username;
        j["score"] = users[username].score;
        j["matrix"] = users[username].matrix;
        return crow::response{ j.dump() };
        });

    CROW_ROUTE(app, "/api/top").methods("GET"_method)([] {
        std::vector<User> sorted;
        for (const auto& pair : users) {
            sorted.push_back(pair.second);
        }

        std::sort(sorted.begin(), sorted.end(), [](const User& a, const User& b) {
            return a.score > b.score;
            });

        json j = json::array();
        for (int i = 0; i < std::min(5, (int)sorted.size()); i++) {
            j.push_back({
                {"username", sorted[i].username},
                {"score", sorted[i].score}
                });
        }

        return crow::response{ j.dump() };
        });

    CROW_ROUTE(app, "/<string>")([](std::string file) {
        std::ifstream f("frontend/" + file);
        if (!f.is_open()) return crow::response(404);
        std::stringstream buffer;
        buffer << f.rdbuf();
        return crow::response(buffer.str());
        });

    app.port(18080).multithreaded().run();
    return 0;
}
