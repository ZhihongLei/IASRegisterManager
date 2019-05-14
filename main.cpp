#include "register_manager.h"
#include "password.h"
#include "login_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QApplication>
#include <QtWebEngineWidgets>
#include <QUrl>
#include <QDialog>
#include <edit_latex_dialog.h>


QString gDatabase, gDBHost, gUser, gPassword;
int main(int argc, char *argv[])
{
    gDatabase = "ias";
    gDBHost = "localhost";
    gUser = "root";
    gPassword = "lzh931229";
    QApplication a(argc, argv);

    //EditLaTeXDialog dialog;
    //dialog.show();
    RegisterManager register_manager;
    LoginDialog login;
    QObject::connect(&login, SIGNAL(logged_in(QString)), &register_manager, SLOT(on_loggedin(QString)));
    login.show();

    return a.exec();
}
