#ifndef TOPPLAYER_H
#define TOPPLAYER_H

#include <string>
#include <vector>

// Cấu trúc lưu thông tin người chơi
struct Player {
    std::string username;
    int score;
};

// Lấy danh sách top người chơi (tối đa 5 người)
std::vector<Player> getTopPlayers();

// Cập nhật danh sách top người chơi với người chơi mới
void updateTopPlayers(const Player& newPlayer);

#endif // TOPPLAYER_H
