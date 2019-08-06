#include "change_password_dialog.h"
#include "ui_change_password_dialog.h"
#include "database_handler.h"
#include <QMessageBox>
#include <QDebug>
#include <QRegExpValidator>

ChangePasswordDialog::ChangePasswordDialog(const QString& username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePasswordDialog),
    username_(username)
{
    ui->setupUi(this);
    ui->lineEditPassword->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9`~!@#$%^&*()_+=\\-\\[\\]{}\\\\|;':\",.<>/?]+")));
    ui->lineEditConfirm->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9`~!@#$%^&*()_+=\\-\\[\\]{}\\\\|;':\",.<>/?]+")));
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}

void ChangePasswordDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool ChangePasswordDialog::sanity_check()
{
    if (!check_password()) return false;
    if (ui->lineEditPassword->text() != ui->lineEditConfirm->text())
    {
        QMessageBox::warning(this, "Change Password", "Passwords entered do not match!\nPlease try again.");
        ui->lineEditPassword->clear();
        ui->lineEditConfirm->clear();
        ui->lineEditPassword->setFocus();
        return false;
    }
    return true;
}

bool ChangePasswordDialog::check_password()
{
    if (ui->lineEditPassword->text() == "")
    {
        QMessageBox::warning(this, "Change Password", "Password must not be empty!");
        return false;
    }
    if (ui->lineEditPassword->text().size() < 6)
    {
        QMessageBox::warning(this, "Change Password", "Password must contain at least 6 characters!");
        return false;
    }
    return true;
}

bool ChangePasswordDialog::change_password()
{
    if (DataBaseHandler::update_items("global_user", {{"username", username_}}, {{"password", ui->lineEditPassword->text()}}))
    {
        DataBaseHandler::commit();
        QMessageBox::information(this, "Change Password", "Password successfully changed!");
        return true;
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Change Password", "Unable to change password.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
}
