#include "login_dialog.h"
#include "ui_login_dialog.h"
#include "../database_handler.h"
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("Login");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::accept()
{
    QString hostname = ui->lineEditHostname->text(),
            username = ui->lineEditUser->text(),
            password = ui->lineEditPassword->text();
    if (hostname  == "")
    {
        QMessageBox::warning(this, "Database Login", "Please specify a hostname!");
        return;
    }
    if (username  == "")
    {
        QMessageBox::warning(this, "Database Login", "Username is empty!");
        return;
    }
    if (password  == "")
    {
        QMessageBox::warning(this, "Database Login", "Password is empty!");
        return;
    }

    if (DataBaseHandler::initialize(hostname, "", username, password))
    {
        emit(logged_in());
        close();
    }
    else {
        QMessageBox::warning(this, "Database Login", "Unable to login to the database.\nError message: " + DataBaseHandler::get_error_message());
    }
}
