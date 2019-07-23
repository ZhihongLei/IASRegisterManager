#include "open_chip_dialog.h"
#include "ui_open_chip_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include "edit_chip_dialog.h"
#include "database_utils.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

OpenChipDialog::OpenChipDialog(const QString& username, const QString& user_id,  bool can_add_chip, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenChipDialog),
    username_(username),
    user_id_(user_id),
    mode_(DIALOG_MODE::OPEN_CHIP)
{
    bool success = setup_ui();
    ui->pushButtonAddChip->setVisible(can_add_chip);
    if (!success) QMessageBox::warning(this, "Open Chip", "Unable to initialize due to database connection issue.\nPlease try again!");
}

OpenChipDialog::OpenChipDialog(const QString& username, const QString& user_id, QString active_chip_id, QWidget *parent):
    QDialog(parent),
    ui(new Ui::OpenChipDialog),
    username_(username),
    user_id_(user_id),
    active_chip_id_(active_chip_id),
    mode_(DIALOG_MODE::MANAGE_CHIP)
{
    bool success = setup_ui();
    ui->pushButtonAddChip->setVisible(true);
    ui->pushButtonRemoveChip->setVisible(true);
    setWindowTitle("Chip Manager");
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    if (!success) QMessageBox::warning(this, "Chip Manager", "Unable to initialize due to database connection issue.\nPlease try again!");
}

OpenChipDialog::~OpenChipDialog()
{
    delete ui;
}

bool OpenChipDialog::setup_ui()
{
    ui->setupUi(this);
    bool success = true;

    QVector<QVector<QString> > items;
    QVector<QString> extended_fields = {"chip_chip.chip_id",
                                    "chip_chip.chip_name",
                                    "global_user.username",
                                    "chip_chip.owner",
                                    "chip_chip.register_width",
                                    "chip_chip.address_width",
                                    "chip_chip.msb_first", ""
                                    "chip_chip.freeze"};
    success = success && DataBaseHandler::show_items_inner_join(extended_fields, {{{"chip_chip", "owner"}, {"global_user", "user_id"}}}, items, "", "order by chip_id");

    QHash<QString, int> recents;
    QSettings settings("global_settings.ini", QSettings::IniFormat);
    settings.beginGroup("recent_projects");

    int i = 1;
    QString p = settings.value("project" + QString::number(i)).toString();
    while (p != "")
    {
        recents[p] = i;
        i++;
        p = settings.value("project" + QString::number(i)).toString();
    }

    for (auto& item: items)
    {
        if (recents.contains(item[0])) item.push_back(QString::number(recents[item[0]]));
        else item.push_back(QString::number(INT_MAX));
        item[6] = (item[6] == "1" ? "true" : "false" );
        item[7] = (item[7] == "1" ? "true" : "false" );
    }
    qSort(items.begin(), items.end(),
          [](const QVector<QString>& a, const QVector<QString>& b){return a[a.size()-1].toInt() < b[b.size()-1].toInt();});

    for (const QVector<QString> &item : items)
    {
        int row = ui->tableWidgetChip->rowCount();
        ui->tableWidgetChip->insertRow(row);
        for (int i = 0; i < item.size()-1; i++) ui->tableWidgetChip->setItem(row, i, new QTableWidgetItem(item[i]));
    }

    ui->tableWidgetChip->setColumnHidden(0, true);
    ui->tableWidgetChip->setColumnHidden(3, true);
    ui->tableWidgetChip->setColumnWidth(1, 200);
    ui->tableWidgetChip->setColumnWidth(2, 120);
    ui->tableWidgetChip->setColumnWidth(4, 100);
    ui->tableWidgetChip->setColumnWidth(5, 100);
    ui->tableWidgetChip->setColumnWidth(6, 100);

    if (ui->tableWidgetChip->rowCount() == 0) ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else ui->tableWidgetChip->setCurrentCell(0, 0);
    ui->pushButtonAddChip->setVisible(false);
    ui->pushButtonRemoveChip->setVisible(false);
    return success;
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

bool OpenChipDialog::msb_first() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 6)->text() == "true" ? true : false;
}

bool OpenChipDialog::frozen() const
{
    int row = ui->tableWidgetChip->currentRow();
    return ui->tableWidgetChip->item(row, 7)->text() == "true" ? true : false;
}

void OpenChipDialog::on_pushButtonAddChip_clicked()
{
    EditChipDialog new_chip(username_, user_id_, this);
    if (new_chip.exec() == QDialog::Accepted && new_chip.add_chip())
    {
        QVector<QString> items = {new_chip.get_chip_id(),
                                  new_chip.get_chip_name(),
                                  new_chip.get_owner(),
                                  new_chip.get_owner_id(),
                                  QString::number(new_chip.get_register_width()),
                                  QString::number(new_chip.get_address_width()),
                                  new_chip.msb_first() ? "true" : "false"};

        int row = ui->tableWidgetChip->rowCount();
        ui->tableWidgetChip->insertRow(row);
        for (int i = 0; i < items.size(); i++) ui->tableWidgetChip->setItem(row, i, new QTableWidgetItem(items[i]));
        ui->tableWidgetChip->setCurrentCell(row, 0);
    }
}

void OpenChipDialog::on_pushButtonRemoveChip_clicked()
{
    int row = ui->tableWidgetChip->currentRow();
    QString chip_id = ui->tableWidgetChip->item(row, 0)->text();
    if (chip_id == active_chip_id_)
    {
        QMessageBox::warning(this, "Remove Chip", "You cannot remove active chip!");
        return;
    }
    if (QMessageBox::warning(this,
                         "Remove Chip",
                         "Are you sure you want to remove this chip?\nEverything belonging to this chip will also be removed.\nThis operation is not reversible!",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (!DatabaseUtils::remove_chip(chip_id))
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove Chip", "Unable to remove this chip.Error Message: " + DataBaseHandler::get_error_message());
        return;
    }
    DataBaseHandler::commit();
    ui->tableWidgetChip->removeRow(row);
}


void OpenChipDialog::on_tableWidgetChip_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveChip->setEnabled(currentRow >= 0);
}

void OpenChipDialog::on_tableWidgetChip_cellDoubleClicked(int row, int column)
{
    if (row < 0 || mode_ == DIALOG_MODE::MANAGE_CHIP) return;
    accept();
}

void OpenChipDialog::accept()
{
    if (mode_ == DIALOG_MODE::MANAGE_CHIP || sanity_check()) QDialog::accept();
}

bool OpenChipDialog::sanity_check()
{
    return check_project_role();
}

bool OpenChipDialog::check_project_role()
{
    QVector<QString> item;
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"def_db_role.full_access_to_all_projects"}, {{{"def_db_role", "db_role_id"}, {"global_user", "db_role_id"}}}, items, "global_user.user_id", user_id_, "") ||
        !DataBaseHandler::show_one_item("chip_designer", item, {"project_role_id"}, {{"chip_id", get_chip_id()}, {"user_id", user_id_}}))
    {
        QMessageBox::warning(this, "Open Chip", "Unable to validate project role due to database connection issue.\nPlease try again!");
        return false;
    }
    if (items.size() > 0 && items[0][0] == "1") return true;
    if (item.size() == 0)
    {
        QMessageBox::warning(this, "Open Chip", "You are not in any role in this project!");
        project_role_id_ = "-1";
        return false;
    }
    project_role_id_ = item[0];
    return true;
}
