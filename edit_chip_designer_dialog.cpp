#include "edit_chip_designer_dialog.h"
#include "ui_edit_chip_designer_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

EditChipDesignerDialog::EditChipDesignerDialog(const QString& chip_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditChipDesignerDialog),
    chip_id_(chip_id),
    mode_(DIALOG_MODE::ADD),
    enabled_(true)
{
    bool success = setup_ui();
    setWindowTitle("Add Chip Designer");
    if (!success) QMessageBox::warning(this, "Add Chip Designer", "Unable to initialize due to database connection issue.\nPlease try again!");
}


EditChipDesignerDialog::EditChipDesignerDialog(const QString& chip_id, const QString& chip_designer, const QString& project_role, bool enabled, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditChipDesignerDialog),
    chip_id_(chip_id),
    mode_(DIALOG_MODE::EDIT),
    enabled_(enabled)
{
    bool success = setup_ui();
    setWindowTitle("Edit Chip Designer");
    for (int i = 0; i < ui->tableWidgetUsers->rowCount(); i++)
        if (ui->tableWidgetUsers->item(i, 1)->text() == chip_designer)
        {
            ui->tableWidgetUsers->setCurrentCell(i, 0);
            break;
        }
    for (int i = 0; i < ui->comboBoxProjectRole->count(); i++)
        if (ui->comboBoxProjectRole->itemText(i) == project_role)
        {
            ui->comboBoxProjectRole->setCurrentIndex(i);
            break;
        }
    ui->tableWidgetUsers->setEnabled(false);
    if (!success) QMessageBox::warning(this, "Edit Chip Designer", "Unable to initialize due to database connection issue.\nPlease try again!");
}

EditChipDesignerDialog::~EditChipDesignerDialog()
{
    delete ui;
}

bool EditChipDesignerDialog::setup_ui()
{
    ui->setupUi(this);
    bool success = true;

    QVector<QVector<QString> > items;
    success = success && DataBaseHandler::show_items("global_user", {"user_id", "username"}, items, "", "order by user_id");
    for (const auto& item : items)
    {
        int row = ui->tableWidgetUsers->rowCount();
        ui->tableWidgetUsers->insertRow(row);
        ui->tableWidgetUsers->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableWidgetUsers->setItem(row, 1, new QTableWidgetItem(item[1]));
    }

    items.clear();
    success = success && DataBaseHandler::show_items("def_project_role", {"project_role_id", "project_role"}, items, "", "order by project_role_id");
    for (const auto& item : items)
    {
        project_role_ids_.push_back(item[0]);
        ui->comboBoxProjectRole->addItem(item[1]);
    }

    if (ui->tableWidgetUsers->rowCount() == 0 || ui->comboBoxProjectRole->count() == 0)
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    else
        ui->tableWidgetUsers->setCurrentCell(0, 0);
    ui->tableWidgetUsers->setColumnHidden(0, true);
    ui->comboBoxProjectRole->setEnabled(enabled_);
    return success;
}

QString EditChipDesignerDialog::get_username() const
{
    int row = ui->tableWidgetUsers->currentRow();
    return ui->tableWidgetUsers->item(row, 1)->text();
}

QString EditChipDesignerDialog::get_user_id() const
{
    int row = ui->tableWidgetUsers->currentRow();
    return ui->tableWidgetUsers->item(row, 0)->text();
}

QString EditChipDesignerDialog::get_project_role() const
{
    return ui->comboBoxProjectRole->currentText();
}

QString EditChipDesignerDialog::get_project_role_id() const
{
    int idx = ui->comboBoxProjectRole->currentIndex();
    return project_role_ids_[idx];
}

QString EditChipDesignerDialog::get_chip_designer_id() const
{
    return chip_designer_id_;
}

void EditChipDesignerDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) QDialog::accept();
}

bool EditChipDesignerDialog::sanity_check()
{
    if (mode_ == DIALOG_MODE::EDIT) return true;
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("chip_designer", {"chip_designer_id"}, {{"chip_id", chip_id_}, {"user_id", get_user_id()}}, items))
    {
        QMessageBox::warning(this, "Add Chip Designer", "Unable to validate designer due to database connection issue.\nPlease try again!");
        return false;
    }
    if (items.size() > 0)
    {
        QMessageBox::warning(this, "Add Chip Designer", "User " + get_username() + " is already a designer of the project!");
        return false;
    }
    return true;
}

bool EditChipDesignerDialog::add_designer()
{
    if (DataBaseHandler::get_next_auto_increment_id("chip_designer", "chip_designer_id", chip_designer_id_) &&
            DataBaseHandler::insert_item("chip_designer", {"chip_designer_id", "chip_id", "user_id", "project_role_id"},
                                        {chip_designer_id_, chip_id_, get_user_id(), get_project_role_id()}))
    {
        DataBaseHandler::commit();
        return true;
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Add Chip Designer", "Unable to add chip designer.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
}

bool EditChipDesignerDialog::edit_designer()
{
    QVector<QString> item;
    if (!DataBaseHandler::show_one_item("chip_designer", item, {"chip_designer_id"}, {{"chip_id", chip_id_}, {"user_id", get_user_id()}}) || item.size() == 0)
    {
        QMessageBox::warning(this, "Edit Chip Designer", "Unable to read designer ID due to database connection issue.\nPlease try again!");
        return false;
    }
    chip_designer_id_ = item[0];
    if(DataBaseHandler::update_items("chip_designer", "chip_designer_id", chip_designer_id_, {{"project_role_id", get_project_role_id()}}))
    {
        DataBaseHandler::commit();
        return true;
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Edit Chip Designer", "Unable to edit designer.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
}
