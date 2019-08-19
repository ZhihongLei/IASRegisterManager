#include "create_user_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include "ui_create_user_dialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <QRegExpValidator>

CreateUserDialog::CreateUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateUserDialog)
{
    ui->setupUi(this);
    ui->lineEditPassword->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9`~!@#$%^&*()_+=\\-\\[\\]{}\\\\|;':\",.<>/?]+")));
    QVector<QVector<QString>> items;
    bool success = DataBaseHandler::show_items("def_db_role", {"db_role_id", "db_role"}, items, "", "order by db_role_id");
    for (const QVector<QString> &item: items)
    {
        role_ids_.push_back(item[0]);
        ui->comboBoxDBRole->addItem(QString(item[1]));
    }
    if (ui->comboBoxDBRole->count() == 0) ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (!success) QMessageBox::warning(this, "Create User", "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}

CreateUserDialog::~CreateUserDialog()
{
    delete ui;
}

QString CreateUserDialog::get_username() const
{
    return ui->lineEditUserName->text();
}

QString CreateUserDialog::get_user_id() const
{
    return user_id_;
}

QString CreateUserDialog::get_password() const
{
    return ui->lineEditPassword->text();
}

QString CreateUserDialog::get_db_role() const
{
    return ui->comboBoxDBRole->currentText();
}

QString CreateUserDialog::get_db_role_id() const
{
    return role_ids_[ui->comboBoxDBRole->currentIndex()];
}

void CreateUserDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool CreateUserDialog::check_name()
{
    if (get_username() == "")
    {
        QMessageBox::warning(this, "Create User", "Username must not be empty!");
        return false;
    }

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("global_user", {"username"}, "username", get_username(), items))
    {
        QMessageBox::warning(this, "Create User", "Unable to validate username due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    if (items.size() > 0)
    {
        QMessageBox::warning(this, "Create User", "User " + get_username() + " already exists!");
        return false;
    }
    return true;
}

bool CreateUserDialog::check_password()
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

bool CreateUserDialog::sanity_check()
{
    return check_name() && check_password();
}

bool CreateUserDialog::create_user()
{

    if (DataBaseHandler::get_next_auto_increment_id("global_user", "user_id", user_id_) &&
        DataBaseHandler::insert_item("global_user",
                              {"user_id", "username", "password", "db_role_id"},
                              {user_id_, get_username(), get_password(), get_db_role_id()}))
    {
        DataBaseHandler::commit();
        QMessageBox::information(this, "Create User", "User successfully created!");
        return true;
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Create User", "Unable to create user.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
}
