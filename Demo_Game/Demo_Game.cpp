#include "include/crow_all.h"
#include <fstream>
#include <sstream>
#include <map>

std::map<std::string, std::string> userDB;

std::string read_file(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) return "";
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

// Reset trạng thái game
void resetGame() {
    std::ofstream ofs("data/savegame.txt");
    ofs << "NEW_GAME_STATE";
    ofs.close();
}

// Kiểm tra trạng thái đã lưu
bool hasSavedGame() {
    std::ifstream ifs("data/savegame.txt");
    return ifs.good();
}

int main() {
    crow::SimpleApp app;

    // Trả về giao diện menu chính
    CROW_ROUTE(app, "/")([] {
        return read_file("static/menu.html");
        });

    // Phục vụ file tĩnh
    CROW_ROUTE(app, "/static/<string>")([](std::string filename) {
        std::string path = "static/" + filename;
        std::string content = read_file(path);
        if (content.empty()) return crow::response(404);
        return crow::response(content);
        });

    // API - New Game
    CROW_ROUTE(app, "/new-game").methods("POST"_method)([] {
        resetGame();
        return crow::response(200);
        });

    // API - Resume Game
    CROW_ROUTE(app, "/resume").methods("GET"_method)([] {
        if (hasSavedGame()) return crow::response(200);
        return crow::response(404);
        });

    // API - Exit
    CROW_ROUTE(app, "/exit").methods("POST"_method)([] {
        // Tùy ý xử lý, frontend có thể tự đóng trang
        return crow::response("Tạm biệt!");
        });

    app.port(18080).multithreaded().run();

    CROW_ROUTE(app, "/api/register").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400);

        std::string username = data["username"].s();
        std::string password = data["password"].s();

        if (userDB.count(username)) {
            return crow::response{ crow::json::wvalue{{"message", "Tài khoản đã tồn tại!"}} };
        }

        userDB[username] = password;
        return crow::response{ crow::json::wvalue{{"message", "Đăng ký thành công!"}} };
        });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400);

        std::string username = data["username"].s();
        std::string password = data["password"].s();

        if (userDB.count(username) && userDB[username] == password) {
            return crow::response{ crow::json::wvalue{{"message", "Đăng nhập thành công!"}} };
        }

        return crow::response{ crow::json::wvalue{{"message", "Sai tài khoản hoặc mật khẩu!"}} };
        });

    CROW_ROUTE(app, "/api/save").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400, "Invalid JSON");

        std::string username = data["username"].s();
        auto matrix = data["matrix"];
        int score = data["score"].i();

        // Ghi file nhị phân (ví dụ đơn giản)
        std::ofstream out("save/" + username + ".txt");  // bạn có thể dùng .bin nếu muốn
        if (out) {
            out << "Username: " << username << "\n";
            out << "Score: " << score << "\n";
            out << "Matrix:\n";
            for (auto& row : matrix) {
                for (auto& val : row) {
                    out << val.i() << " ";
                }
                out << "\n";
            }
            out.close();
        }

        crow::json::wvalue res;
        res["reply"] = "Lưu trạng thái thành công cho " + username;
        return crow::response{ res };
        });

    // Route API: Tải lại trạng thái người chơi
    CROW_ROUTE(app, "/api/load").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("username"))
            return crow::response(400);

        std::string username = data["username"].s();
        std::string filename = "save/" + username + ".txt";

        std::ifstream ifs(filename);
        if (!ifs) {
            crow::json::wvalue res;
            res["error"] = "Không tìm thấy trạng thái.";
            return crow::response(404, res);
        }

        int score;
        int matrix[4][4];

        ifs >> score;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                ifs >> matrix[i][j];

        crow::json::wvalue res;
        res["score"] = score;

        crow::json::wvalue mat_json;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                mat_json[i][j] = matrix[i][j];

        res["matrix"] = mat_json;

        return crow::response{ res };
        });

}

// Route khởi tạo game mới
CROW_ROUTE(app, "/new-game").methods("POST"_method)([]() {
    // TODO: Reset trạng thái người chơi ở đây
    std::ofstream save("save.json");
    save << R"({"matrix":[[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]],"score":0})";
    save.close();

    return crow::response(200);
    });

// Route resume game
CROW_ROUTE(app, "/resume").methods("GET"_method)([]() {
    std::ifstream save("save.json");
    if (!save) return crow::response(404);
    return crow::response(200);  // Có thể gửi lại data ở đây nếu cần
    });

// Route game.html (hiển thị lưới 2048 sau khi nhấn New Game / Resume)
CROW_ROUTE(app, "/game.html")([] {
    return read_file("static/game.html");  // Tạo file này
    });
