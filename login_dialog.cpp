#include "login_dialog.h"
#include "ui_login_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
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

void LoginDialog::set_username(const QString &username)
{
    ui->lineEditUser->setText(username);
}

void LoginDialog::set_password(const QString &password)
{
    ui->lineEditPassword->setText(password);
}

void LoginDialog::set_save_password(bool save)
{
    ui->checkBoxSavePassword->setChecked(save);
}

QString LoginDialog::get_username() const
{
    return ui->lineEditUser->text();
}

QString LoginDialog::get_password() const
{
    return ui->lineEditPassword->text();
}

bool LoginDialog::save_password() const
{
    return ui->checkBoxSavePassword->isChecked();
}

bool LoginDialog::login()
{
    if (get_username() == "")
    {
        QMessageBox::warning(this, "Login", "Username is empty!");
        return false;
    }

    if (get_password() == "")
    {
        QMessageBox::warning(this, "Login", "Password is empty!");
        return false;
    }

    QVector<QString> item;
    if (!DataBaseHandler::show_one_item("global_user", item, {"password"}, "username", get_username()))
    {
        QMessageBox::warning(this, "Login", "Unable to validate user account due to database connection issue.\nPlease try again!");
        return false;
    }
    if (item.empty())
    {
        QMessageBox::warning(this, "Login", "User does not exist");
        ui->lineEditUser->setText("");
        ui->lineEditPassword->setText("");
        ui->lineEditUser->setFocus();
        return false;
    }
    if (item[0] != get_password())
    {
        QMessageBox::warning(this, "Login", "Password is not correct!");
        ui->lineEditPassword->setText("");
        return false;
    }
    return true;
}

void LoginDialog::clear()
{
    ui->lineEditPassword->clear();
}

void LoginDialog::accept()
{
    if (login())
    {
        emit(logged_in(get_username()));
        QDialog::accept();
    }
}
