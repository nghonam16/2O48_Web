#include "TopPlayer.h"
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<Player> getTopPlayers() {
    std::vector<Player> top;
    std::ifstream file("data/leaderboard.bin", std::ios::binary);
    if (!file) return top;

    int count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(int));
    if (!file || count <= 0) return top;

    for (int i = 0; i < count; ++i) {
        Player p;
        size_t nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
        if (!file || nameLen == 0) break;

        p.username.resize(nameLen);
        file.read(&p.username[0], nameLen);
        file.read(reinterpret_cast<char*>(&p.score), sizeof(int));
        if (!file) break;

        top.push_back(std::move(p));
    }
    file.close();
    return top;
}

void updateTopPlayers(const Player& newPlayer) {
    // Đảm bảo thư mục tồn tại
    if (!fs::exists("data")) {
        fs::create_directories("data");
    }

    std::vector<Player> top = getTopPlayers();
    top.push_back(newPlayer);

    std::sort(top.begin(), top.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
        });

    if (top.size() > 5) top.resize(5);

    std::ofstream file("data/leaderboard.bin", std::ios::binary);
    if (!file) return;

    int count = static_cast<int>(top.size());
    file.write(reinterpret_cast<char*>(&count), sizeof(int));
    for (const auto& p : top) {
        size_t nameLen = p.username.size();
        file.write(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
        file.write(p.username.c_str(), nameLen);
        file.write(reinterpret_cast<const char*>(&p.score), sizeof(int));
    }
    file.close();
}
