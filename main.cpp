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
#include <QFileDevice>
#include "document_editor.h"

QString gDatabase, gDBHost, gUser, gPassword, mathjax_root;
QHash<QString, QString> NamingKey2Value;
int main(int argc, char *argv[])
{
    gDatabase = "ias";
    gDBHost = "localhost";
    gUser = "root";
    gPassword = "lzh931229";
    mathjax_root = "/Users/zhihonglei/tools/MathJax";
    QApplication a(argc, argv);


    //QString file = QFileDialog::getOpenFileName(nullptr, "Select an image file", "", "Images (*.png *.jpg *.jpeg *.pdf)");
    //std::cout << file.toUtf8().constData() <<std::endl;
    //QFileDialog dialog;
    //dialog.setNameFilter("Images (*.png *.jpg *.jpeg *.pdf)");
    //dialog.show();
    //EditLaTeXDialog dialog;
    //dialog.show();
    RegisterManager register_manager;
    //EditDocumentDialog e(EditDocumentDialog::BLOCK, "1");
    //LoginDialog login;
    //QObject::connect(&login, SIGNAL(logged_in(QString)), &register_manager, SLOT(on_loggedin(QString)));
    //login.show();

    return a.exec();
}
