#ifndef PASSWORD_H
#define PASSWORD_H
#include <unordered_map>

#define USER_NOT_EXISTS_ERROR 1
#define PASSWORD_NOT_CORRECT_ERROR 2
#define USER_ALREADY_EXISTS_ERROR 3

std::unordered_map<std::string, std::string> read_passwords(const std::string &filename);
void write_password(const std::unordered_map<std::string, std::string>& u2pw, const std::string &filename);
int login(const std::string &username, const std::string &password);
int create_user(const std::string &username, const std::string &password);

#endif // PASSWORD_H
