#include "open_chip_dialog.h"
#include "ui_open_chip_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

OpenChipDialog::OpenChipDialog(const QString& user_id, QWidget *parent) :
    QDialog(parent),
    user_id_(user_id),
    ui(new Ui::OpenChipDialog)
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    QVector<QPair<QString, QString> > ext_fields = {{"chip_chip", "chip_id"},
                                                    {"chip_chip", "chip_name"},
                                                    {"global_user", "username"},
                                                    {"chip_chip", "owner"},
                                                    {"chip_chip", "register_width"},
                                                    {"chip_chip", "address_width"},
                                                    {"chip_chip", "msb_first"}};
    dbhandler.show_items_inner_join(ext_fields,
                                     {{"chip_chip", "global_user"}}, {{"owner", "user_id"}}, items, "", "order by chip_id");
    for (const QVector<QString> &item : items)
    {
        int row = ui->tableWidgetChip->rowCount();
        ui->tableWidgetChip->insertRow(row);
        for (int i = 0; i < item.size(); i++) ui->tableWidgetChip->setItem(row, i, new QTableWidgetItem(item[i]));
    }

    ui->tableWidgetChip->setColumnHidden(0, true);
    ui->tableWidgetChip->setColumnHidden(3, true);
    ui->tableWidgetChip->setColumnHidden(6, true);
    ui->tableWidgetChip->setColumnWidth(1, 200);
    ui->tableWidgetChip->setColumnWidth(2, 150);
    ui->tableWidgetChip->setColumnWidth(4, 100);
    ui->tableWidgetChip->setColumnWidth(5, 100);

    if (items.size() == 0) ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else ui->tableWidgetChip->setCurrentCell(0, 0);
}


OpenChipDialog::~OpenChipDialog()
{
    delete ui;
}

QString OpenChipDialog::get_chip_id() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 0)->text();
}

QString OpenChipDialog::get_chip_name() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 1)->text();
}

QString OpenChipDialog::get_owner() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 2)->text();
}

QString OpenChipDialog::get_owner_id() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 3)->text();
}

QString OpenChipDialog::get_project_role_id() const
{
    return project_role_id_;
}

int OpenChipDialog::get_register_width() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 4)->text().toInt();
}

int OpenChipDialog::get_address_width() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 5)->text().toInt();
}

bool OpenChipDialog::is_msb_first() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 6)->text() == "1" ? true : false;
}

void OpenChipDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool OpenChipDialog::check_project_role()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("chip_designer", item, {"project_role_id"}, {{"chip_id", get_chip_id()}, {"user_id", user_id_}});
    if (item.size() == 0)
    {
        QMessageBox::warning(this, "Open Chip", "You are not in any role in this project!");
        project_role_id_ = "-1";
        return false;
    }
    project_role_id_ = item[0];
    return true;
}

bool OpenChipDialog::sanity_check()
{
    return check_project_role();
}
