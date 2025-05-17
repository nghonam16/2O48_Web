#include "include/crow_all.h"  // nếu bạn để crow_all.h trong thư mục include
#include <fstream>
#include <sstream>

std::string read_file(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) return "";
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

int main() {
    crow::SimpleApp app;

    // Trả về giao diện chính
    CROW_ROUTE(app, "/")([] {
        return read_file("static/index.html");
        });

    // Trả về các file tĩnh (style.css, script.js)
    CROW_ROUTE(app, "/static/<string>")([](std::string filename) {
        std::string path = "static/" + filename;
        std::string content = read_file(path);
        if (content.empty()) return crow::response(404);
        return crow::response(content);
        });

    // API trả về lời chào (demo kết nối frontend → backend)
    CROW_ROUTE(app, "/api/hello").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data) return crow::response(400);

        std::string name = data["name"].s();
        std::string message = "Xin chào, " + name + " từ C++ backend!";

        crow::json::wvalue res;
        res["reply"] = message;
        return crow::response{ res };
        });

    app.port(18080).multithreaded().run();
}
