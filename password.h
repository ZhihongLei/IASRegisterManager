#ifndef PASSWORD_H
#define PASSWORD_H
#include <QString>

#define USER_NOT_EXISTS_ERROR -1
#define PASSWORD_NOT_CORRECT_ERROR -2
#define USER_ALREADY_EXISTS_ERROR -3

int login(const QString &username, const QString &password);
int create_user(const QString &username, const QString &password, int db_role);

#endif // PASSWORD_H
