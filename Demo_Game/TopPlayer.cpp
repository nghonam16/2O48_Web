#include "TopPlayer.h"
#include <fstream>
#include <algorithm>
#include <filesystem>

std::vector<Player> getTopPlayers() {
    std::vector<Player> top;
    std::ifstream file("data/leaderboard.bin", std::ios::binary);
    if (!file) return top;

    int count;
    file.read(reinterpret_cast<char*>(&count), sizeof(int));

    for (int i = 0; i < count; ++i) {
        Player p;
        size_t nameLen;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
        p.username.resize(nameLen);
        file.read(&p.username[0], nameLen);
        file.read(reinterpret_cast<char*>(&p.score), sizeof(int));
        top.push_back(p);
    }

    file.close();
    return top;
}

void updateTopPlayers(const Player& newPlayer) {
    std::vector<Player> top = getTopPlayers();
    top.push_back(newPlayer);

    std::sort(top.begin(), top.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
        });

    if (top.size() > 5) top.resize(5);

    std::ofstream file("data/leaderboard.bin", std::ios::binary);
    int count = top.size();
    file.write(reinterpret_cast<char*>(&count), sizeof(int));
    for (const auto& p : top) {
        size_t nameLen = p.username.size();
        file.write(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
        file.write(p.username.c_str(), nameLen);
        file.write(reinterpret_cast<const char*>(&p.score), sizeof(int));
    }

    file.close();
}
