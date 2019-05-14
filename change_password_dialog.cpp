#include "change_password_dialog.h"
#include "ui_change_password_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QMessageBox>

ChangePasswordDialog::ChangePasswordDialog(const QString& username, QWidget *parent) :
    QDialog(parent),
    username_(username),
    ui(new Ui::ChangePasswordDialog)
{
    ui->setupUi(this);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}

bool ChangePasswordDialog::check_password()
{
    // TODO
    return true;
}

bool ChangePasswordDialog::sanity_check()
{
    if (!check_password()) return false;
    if (ui->lineEditNewPassword->text() != ui->lineEditReEnter->text())
    {
        QMessageBox::warning(this, "Change Password", "Passwords entered do not match!\nPlease re-enter.");
        ui->lineEditNewPassword->clear();
        ui->lineEditReEnter->clear();
        return false;
    }
    return true;
}

bool ChangePasswordDialog::change_password()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.update_items("global_user", {{"username", username_}}, {{"password", ui->lineEditNewPassword->text()}}))
    {
        QMessageBox::information(this, "Change Password", "Password successfully changed!");
        return true;
    }
    else
    {
        QMessageBox::warning(this, "Change Password", QString("Password change failed\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}

void ChangePasswordDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}
