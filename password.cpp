#include "password.h"
#include <iostream>
#include <sstream>
#include <fstream>

const std::string FILENAME("/Users/zhihonglei/Projects/userpass/passwords.txt");


std::unordered_map<std::string, std::string> read_passwords(const std::string &filename)
{
    std::unordered_map<std::string, std::string> map;
    std::ifstream pass_file(filename);
    std::string line;
    while (std::getline(pass_file, line))
    {
        std::stringstream iss;
        iss << line;
        std::string username, password;
        if (!(iss >> username >> password)) break;
        map[username] = password;
    }
    return map;
}

void write_password(const std::unordered_map<std::string, std::string>& u2pw, const std::string &filename)
{
    std::ofstream outfile(filename);
    for (auto &x : u2pw)
    {
        outfile << x.first << " " << x.second << std::endl;
    }
}

int login(const std::string &username, const std::string &password)
{
    std::unordered_map<std::string, std::string> u2pw = read_passwords(FILENAME);
    //for (auto &x : u2pw) std::cout << x.first << std::endl;
    if (u2pw.find(username) == u2pw.end()) {
        std::cout << "User does not exist" << std::endl;
        return USER_NOT_EXISTS_ERROR;
    }
    if (u2pw[username] != password) {
        std::cout << "Wrong password" << std::endl;
        return PASSWORD_NOT_CORRECT_ERROR;
    }
    else std::cout << "Logged in successfully!" << std::endl;
    return 0;
}

int create_user(const std::string &username, const std::string &password)
{
    std::unordered_map<std::string, std::string> u2pw = read_passwords(FILENAME);
    if (u2pw.find(username) != u2pw.end()) {
        std::cout << "User already exists" << std::endl;
        return USER_ALREADY_EXISTS_ERROR;
    }
    u2pw[username] = password;
    write_password(u2pw, FILENAME);
    return 0;
}

