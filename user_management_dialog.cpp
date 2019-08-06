#include "user_management_dialog.h"
#include "ui_user_management_dialog.h"
#include "create_user_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include "database_utils.h"
#include <QMessageBox>

UserManagementDialog::UserManagementDialog(const QString& chip_id, const QString& myself, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserManagementDialog),
    myself_(myself),
    active_chip_id_(chip_id)
{
    ui->setupUi(this);
    ui->tableWidgetUser->setColumnHidden(0, true);
    
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"global_user.user_id", "global_user.username", "def_db_role.db_role"},
                                    {{{"global_user","db_role_id"}, {"def_db_role", "db_role_id"}}}, items))
    {
        QMessageBox::warning(this, "User Manager", "Unable to initialize user manager dialog due to database connection issue.\nPlease try again.");
    }
    for (auto item : items)
    {
        int row = ui->tableWidgetUser->rowCount();
        ui->tableWidgetUser->insertRow(row);
        ui->tableWidgetUser->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableWidgetUser->setItem(row, 1, new QTableWidgetItem(item[1]));
        ui->tableWidgetUser->setItem(row, 2, new QTableWidgetItem(item[2]));
    }
    if (ui->tableWidgetUser->rowCount() > 0) ui->tableWidgetUser->setCurrentCell(0, 0);
}

UserManagementDialog::~UserManagementDialog()
{
    delete ui;
}

void UserManagementDialog::on_pushButtonAddUser_clicked()
{
    CreateUserDialog create_user;
    if (create_user.exec() == QDialog::Accepted && create_user.create_user())
    {
        int row = ui->tableWidgetUser->rowCount();
        ui->tableWidgetUser->insertRow(row);
        ui->tableWidgetUser->setItem(row, 0, new QTableWidgetItem(create_user.get_user_id()));
        ui->tableWidgetUser->setItem(row, 1, new QTableWidgetItem(create_user.get_username()));
        ui->tableWidgetUser->setItem(row, 2, new QTableWidgetItem(create_user.get_db_role()));
    }
}

void UserManagementDialog::on_pushButtonRemoveUser_clicked()
{
    int row = ui->tableWidgetUser->currentRow();
    if (row < 0) return;
    
    QString user_id = ui->tableWidgetUser->item(row, 0)->text(),
            username = ui->tableWidgetUser->item(row, 1)->text();
    if (username == "admin")
    {
        QMessageBox::warning(this, "Remove User", "Account admin is reserved by the system. You cannot remove it!");
        return;
    }
    if (user_id == myself_)
    {
        QMessageBox::warning(this, "Remove User", "You cannot remove yourself!");
        return;
    }

    QVector<QString> item;
    if (DataBaseHandler::show_one_item("chip_designer", item, {"chip_designer_id"}, "user_id", user_id))
    {
        if (item.size() > 0)
        {
            QMessageBox::warning(this, "Remove User", username + " is a designer of the curent chip.\nPlease close the chip and try again.");
            return;
        }
        else
        {
            if (QMessageBox::question(this, "Remove User", "Are you sure you want to remove " + username,
                                      QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) return;
            if (DatabaseUtils::remove_user(myself_, user_id))
            {
                DataBaseHandler::commit();
                ui->tableWidgetUser->removeRow(row);
                return;
            }
            else DataBaseHandler::rollback();
        }
    }
    QMessageBox::warning(this, "Remove User", "Unable to remove user.\nError message: " + DataBaseHandler::get_error_message());
}

void UserManagementDialog::on_tableWidgetUser_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveUser->setEnabled(currentRow>=0);
}
