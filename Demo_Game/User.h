#ifndef USER_H
#define USER_H

#include <string>

bool registerAccount(const std::string& username, const std::string& password);
bool loginAccount(const std::string& username, const std::string& password);

#endif // USER_H
