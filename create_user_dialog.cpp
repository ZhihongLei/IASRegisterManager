#include "create_user_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include "ui_create_user_dialog.h"
#include <QMessageBox>
#include <QPushButton>

CreateUserDialog::CreateUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateUserDialog)
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString>> items;
    dbhandler.show_items("def_db_role", {"db_role_id", "db_role"}, items, "", "order by db_role_id");
    for (const QVector<QString> &item: items)
    {
        role_ids_.push_back(item[0]);
        ui->comboBoxDBRole->addItem(QString(item[1]));
    }
    if (ui->comboBoxDBRole->count() == 0) ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

CreateUserDialog::~CreateUserDialog()
{
    delete ui;
}

QString CreateUserDialog::get_username()
{
    return ui->lineEditUserName->text();
}

QString CreateUserDialog::get_user_id()
{
    return user_id_;
}

QString CreateUserDialog::get_password()
{
    return ui->lineEditPassword->text();
}

QString CreateUserDialog::get_db_role()
{
    return ui->comboBoxDBRole->currentText();
}

QString CreateUserDialog::get_db_role_id()
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
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("global_user", {"username"}, "username", get_username(), items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, "Create User", "User " + get_username() + " already exists!");
        return false;
    }
    return true;
}

bool CreateUserDialog::check_password()
{
    // TODO
    return true;
}

bool CreateUserDialog::sanity_check()
{
    return check_name() && check_password();
}

bool CreateUserDialog::create_user()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.insert_item("global_user",
                              {"username", "password", "db_role_id"},
                              {get_username(), get_password(), get_db_role_id()}))
    {
        QMessageBox::information(this, "Create User", "User successfully created!");
        QVector<QString> item;
        dbhandler.show_one_item("global_user", item, {"user_id"}, "username", get_username());
        user_id_ = item[0];
        return true;
    }
    else
    {
        QMessageBox::warning(this, "Create User", QString("User not created\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}
