#ifndef TOP_PLAYER_H
#define TOP_PLAYER_H

#include <string>
#include <vector>

// Cấu trúc lưu thông tin người chơi.
struct Player {
    std::string username;
    int score;
};

// Đọc danh sách top người chơi từ file binary.
// Trả về vector chứa các Player.
std::vector<Player> getTopPlayers();

// Cập nhật bảng xếp hạng với điểm mới.
// Tự động sắp xếp và chỉ lưu top 5 người chơi.
void updateTopPlayers(const Player& newPlayer);

#endif
