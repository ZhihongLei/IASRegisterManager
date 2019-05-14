#include "password.h"
#include <QVector>
#include "database_handler.h"
#include "global_variables.h"
#include <iostream>

int login(const QString &username, const QString &password)
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("global_user", item, {"password"}, "username", username);
    if (item.empty()) {
        return USER_NOT_EXISTS_ERROR;
    }
    if (item[0] != password) {
        return PASSWORD_NOT_CORRECT_ERROR;
    }
    return 0;
}

int create_user(const QString &username, const QString &password, int db_role)
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("global_user", item, {"password"}, "username", username);
    if (!item.empty()) {
        std::cout << "User already exists" << std::endl;
        return USER_ALREADY_EXISTS_ERROR;
    }
    return dbhandler.insert_item("global_user", {"username", "password", "db_role"}, {username, password, std::to_string(db_role).c_str()});
}

