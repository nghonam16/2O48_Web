#ifndef USER_H
#define USER_H

#include <string>

// Đăng ký tài khoản mới.
// Trả về true nếu đăng ký thành công, false nếu tài khoản đã tồn tại.
bool registerAccount(const std::string& username, const std::string& password);

// Đăng nhập tài khoản.
// Trả về true nếu username và password đúng.
bool loginAccount(const std::string& username, const std::string& password);

#endif
