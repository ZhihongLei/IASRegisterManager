#include "login_dialog.h"
#include "database_initializer_dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginDialog login;
    DatabaseInitializerDialog init;
    login.show();
    QObject::connect(&login, SIGNAL(logged_in()), &init, SLOT(on_logged_in()));

    return a.exec();
}
