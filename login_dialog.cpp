#include "login_dialog.h"
#include "ui_login_dialog.h"
#include "password.h"
#include <QMessageBox>
#include <iostream>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog){
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

QString LoginDialog::get_username()
{
    return ui->lineEdit_user->text();
}

QString LoginDialog::get_password()
{
    return ui->lineEdit_password->text();
}

bool LoginDialog::sanity_check()
{
    int ret = login(get_username(), get_password());
    if (ret == USER_NOT_EXISTS_ERROR)
    {
        QMessageBox::warning(this, "Login", "User does not exist");
        ui->lineEdit_user->setText("");
        ui->lineEdit_password->setText("");
        ui->lineEdit_user->setFocus();
        return false;
    }
    else if (ret == PASSWORD_NOT_CORRECT_ERROR)
    {
        QMessageBox::warning(this, "Login", "Password is not correct");
        ui->lineEdit_password->setText("");
        return false;
    }
    return true;
}

void LoginDialog::accept()
{
    if (sanity_check())
    {
        emit(logged_in(get_username()));
        QDialog::accept();
    }
}

void LoginDialog::clear()
{
    ui->lineEdit_user->setFocus();
    ui->lineEdit_user->clear();
    ui->lineEdit_password->clear();
}
