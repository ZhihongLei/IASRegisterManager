#include "add_chip_designer_dialog.h"
#include "ui_add_chip_designer_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QTableWidgetItem>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

AddChipDesignerDialog::AddChipDesignerDialog(const QString& chip_id, QWidget *parent) :
    QDialog(parent),
    chip_id_(chip_id),
    ui(new Ui::AddChipDesignerDialog)
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("global_user", {"user_id", "username"}, items, "", "order by user_id");
    for (const auto& item : items)
    {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(item[1]));
    }

    items.clear();
    dbhandler.show_items("def_project_role", {"project_role_id", "project_role"}, items, "", "order by project_role_id");
    for (const auto& item : items)
    {
        project_role_ids_.push_back(item[0]);
        ui->comboBox->addItem(item[1]);
    }

    if (ui->tableWidget->rowCount() == 0 || ui->comboBox->count() == 0)
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    else
        ui->tableWidget->setCurrentCell(0, 0);

    ui->tableWidget->setColumnHidden(0, true);
}

AddChipDesignerDialog::~AddChipDesignerDialog()
{
    delete ui;
}

QString AddChipDesignerDialog::get_username() const
{
    int row = ui->tableWidget->currentRow();
    return ui->tableWidget->item(row, 1)->text();
}

QString AddChipDesignerDialog::get_user_id() const
{
    int row = ui->tableWidget->currentRow();
    return ui->tableWidget->item(row, 0)->text();
}

QString AddChipDesignerDialog::get_project_role() const
{
    return ui->comboBox->currentText();
}

QString AddChipDesignerDialog::get_project_role_id() const
{
    int idx = ui->comboBox->currentIndex();
    return project_role_ids_[idx];
}

QString AddChipDesignerDialog::get_chip_designer_id() const
{
    return chip_designer_id_;
}

void AddChipDesignerDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool AddChipDesignerDialog::sanity_check()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("chip_designer", {"chip_designer_id"}, {{"chip_id", chip_id_}, {"user_id", get_user_id()}}, items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, "Add Designer", "User " + get_username() + " is already a designer of the project!");
        return false;
    }
    return true;
}

bool AddChipDesignerDialog::add_designer()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    if (dbhandler.insert_item("chip_designer", {"chip_id", "user_id", "project_role_id"}, {chip_id_, get_user_id(), get_project_role_id()}) && \
            dbhandler.show_items("chip_designer", {"chip_designer_id"}, {{"chip_id", chip_id_}, {"user_id", get_user_id()}}, items))
    {
        chip_designer_id_ = items[0][0];
        return true;
    }
    else
    {
        QMessageBox::warning(this, "Add Designer", QString("Adding designer failed!\nError message: ") + dbhandler.get_error_message());
        return false;
    }

}
