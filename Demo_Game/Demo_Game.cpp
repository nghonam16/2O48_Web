#include "include/crow_all.h"
#include "User.h"
#include "TopPlayer.h"
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

// Giả lập dữ liệu người dùng
std::map<std::string, std::string> userDB;

// Cấu trúc lưu thông tin người chơi
struct Player {
    std::string username;
    int score;
};

// Đọc nội dung file vào string
std::string read_file(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) return "";
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

// Ghi trạng thái game mặc định (cho New Game)
void resetGame() {
    std::ofstream save("save.json");
    save << R"({"matrix":[[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]],"score":0})";
    save.close();
}

// Kiểm tra xem có trạng thái lưu không
bool hasSavedGame() {
    std::ifstream save("save.json");
    return save.good();
}

// Lưu tài khoản mới (giản lược - chỉ lưu trong map)
bool registerAccount(const std::string& username, const std::string& password) {
    if (userDB.count(username)) return false;
    userDB[username] = password;
    return true;
}

// Giả lập lấy danh sách top người chơi
std::vector<Player> getTopPlayers() {
    // TODO: Đọc từ file nhị phân nếu có
    return {
        {"alice", 2048},
        {"bob", 1024},
        {"carol", 512}
    };
}

int main() {
    crow::SimpleApp app;

    // Serve trang menu chính
    CROW_ROUTE(app, "/")([] {
        return read_file("static/menu.html");
        });

    // Serve file tĩnh
    CROW_ROUTE(app, "/static/<string>")([](std::string filename) {
        std::string content = read_file("static/" + filename);
        if (content.empty()) return crow::response(404);
        return crow::response(content);
        });

    // Hiển thị game.html
    CROW_ROUTE(app, "/game.html")([] {
        return read_file("static/game.html");
        });

    // ===== API SECTION =====

    // New Game: reset trạng thái
    CROW_ROUTE(app, "/new-game").methods("POST"_method)([] {
        resetGame();
        return crow::response(200);
        });

    // Resume Game: kiểm tra trạng thái
    CROW_ROUTE(app, "/resume").methods("GET"_method)([] {
        if (hasSavedGame()) return crow::response(200);
        return crow::response(404);
        });

    // Thoát game
    CROW_ROUTE(app, "/exit").methods("POST"_method)([] {
        return crow::response("Tạm biệt!");
        });

    // Đăng ký tài khoản
    CROW_ROUTE(app, "/api/register").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        bool success = registerAccount(username, password);

        crow::json::wvalue res;
        res["success"] = success;
        res["reply"] = success ? "Đăng ký thành công!" : "Tài khoản đã tồn tại!";
        return crow::response(res);
        });

    // Đăng nhập
    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        bool match = userDB.count(username) && userDB[username] == password;

        crow::json::wvalue res;
        res["message"] = match ? "Đăng nhập thành công!" : "Sai tài khoản hoặc mật khẩu!";
        return crow::response(res);
        });

    // Save trạng thái người chơi (ma trận + điểm)
    CROW_ROUTE(app, "/api/save").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("username") || !data.has("matrix") || !data.has("score"))
            return crow::response(400, "Thiếu dữ liệu");

        std::string username = data["username"].s();
        auto matrix = data["matrix"];
        int score = data["score"].i();

        std::ofstream out("save/" + username + ".bin", std::ios::binary);
        if (!out) return crow::response(500, "Không thể ghi file");

        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j) {
                int val = matrix[i][j].i();
                out.write(reinterpret_cast<char*>(&val), sizeof(int));
            }

        out.write(reinterpret_cast<char*>(&score), sizeof(int));
        out.close();

        return crow::response(crow::json::wvalue{ {"message", "Lưu trạng thái thành công!"} });
        });

    // Load trạng thái người chơi từ file .bin
    CROW_ROUTE(app, "/api/load").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("username"))
            return crow::response(400, "Thiếu tên người dùng");

        std::string username = data["username"].s();
        std::ifstream in("save/" + username + ".bin", std::ios::binary);
        if (!in) return crow::response(404, "Không tìm thấy file lưu");

        int matrix[4][4], score;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                in.read(reinterpret_cast<char*>(&matrix[i][j]), sizeof(int));

        in.read(reinterpret_cast<char*>(&score), sizeof(int));
        in.close();

        crow::json::wvalue res;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                res["matrix"][i][j] = matrix[i][j];

        res["score"] = score;
        return crow::response(res);
        });

    // Trả về top người chơi
    CROW_ROUTE(app, "/api/top")([] {
        crow::json::wvalue result;
        result["players"] = crow::json::wvalue::list();

        for (const auto& p : getTopPlayers()) {
            crow::json::wvalue item;
            item["username"] = p.username;
            item["score"] = p.score;
            result["players"].emplace_back(std::move(item));
        }

        return crow::response(result);
        });

    // Chạy app
    app.port(18080).multithreaded().run();
}
