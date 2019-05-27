#include "user_management_dialog.h"
#include "ui_user_management_dialog.h"
#include "create_user_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include <QMessageBox>
#include <QTableWidgetItem>

UserManagementDialog::UserManagementDialog(const QString& myself, QWidget *parent) :
    QDialog(parent),
    myself_(myself),
    ui(new Ui::UserManagementDialog)
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"global_user.username", "def_db_role.db_role"},
                                    {{{"global_user","db_role_id"}, {"def_db_role", "db_role_id"}}}, items);
    for (auto item : items)
    {
        int row = ui->tableWidgetUser->rowCount();
        ui->tableWidgetUser->insertRow(row);
        ui->tableWidgetUser->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableWidgetUser->setItem(row, 1, new QTableWidgetItem(item[1]));
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
        ui->tableWidgetUser->setItem(row, 0, new QTableWidgetItem(create_user.get_username()));
        ui->tableWidgetUser->setItem(row, 1, new QTableWidgetItem(create_user.get_db_role()));
    }
}

void UserManagementDialog::on_pushButtonRemoveUser_clicked()
{
    int row = ui->tableWidgetUser->currentRow();
    if (row < 0) return;
    DataBaseHandler dbhandler(gDBHost, gDatabase);

    QString username = ui->tableWidgetUser->item(row, 0)->text();
    if (username == myself_)
    {
        QMessageBox::warning(this, "Remove User", "Cannot remove yourself!");
        return;
    }
    if (QMessageBox::question(this, "Remove User", "Are you sure you want to remove " + username,
                              QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) return;
    if (dbhandler.delete_items("global_user", "username", username)) ui->tableWidgetUser->removeRow(row);
    else QMessageBox::warning(this, "Remove User", QString("Removing user failed\nError message: ") + dbhandler.get_error_message());

}

void UserManagementDialog::on_tableWidgetUser_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveUser->setEnabled(currentRow>=0);
}
