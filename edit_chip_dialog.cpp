#include "edit_chip_dialog.h"
#include "ui_edit_chip_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>

EditChipDialog::EditChipDialog(const QString& creater_id, QWidget *parent) :
    QDialog(parent),
    creater_id_(creater_id),
    ui(new Ui::EditChipDialog)
{
    ui->setupUi(this);
    ui->lineEditRegWidth->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));
    ui->lineEditAddrWidth->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));
}

EditChipDialog::~EditChipDialog()
{
    delete ui;
}


QString EditChipDialog::get_chip_name() const
{
    return ui->lineEditChipName->text();
}

QString EditChipDialog::get_chip_id() const
{
    return chip_id_;
}

int EditChipDialog::get_address_width() const
{
    if (ui->lineEditAddrWidth->text() == "") return -1;
    return ui->lineEditAddrWidth->text().toInt();
}

int EditChipDialog::get_register_width() const
{
    if (ui->lineEditRegWidth->text() == "") return -1;
    return ui->lineEditRegWidth->text().toInt();
}

bool EditChipDialog::check_name()
{
    if (get_chip_name() == "")
    {
        QMessageBox::warning(this, "Add Chip", "Chip name must not be empty!");
        return false;
    }
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> >items;
    dbhandler.show_items("chip_chip", {"chip_name"}, "chip_name", get_chip_name(), items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, "Add Chip", "Chip " + get_chip_name() + " already exists!");
        return false;
    }
    return true;
}

bool EditChipDialog::check_address_width()
{
    if (get_address_width() <= 0)
    {
        QMessageBox::warning(this, "Add Chip", "Please give a valid address width!");
        return false;
    }
    return true;
}

bool EditChipDialog::check_register_width()
{
    if (get_register_width() <= 0)
    {
        QMessageBox::warning(this, "Add Chip", "Please give a valid register width!");
        return false;
    }
    return true;
}

bool EditChipDialog::sanity_check()
{
    return check_name() && check_address_width() && check_register_width();
}

void EditChipDialog::accept()
{
    if (sanity_check()) return QDialog::accept();
}

bool EditChipDialog::add_chip()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> >items;
    if (dbhandler.insert_item("chip_chip", {"chip_name", "owner", "register_width", "address_width", "msb_first"},
                                  {ui->lineEditChipName->text(),
                                  creater_id_,
                                  ui->lineEditRegWidth->text(),
                                  ui->lineEditAddrWidth->text(),
                                  ui->checkBoxMSBFirst->isChecked() ? "1" : "0"}) && \
                                dbhandler.show_items("chip_chip", {"chip_id"}, "chip_name", get_chip_name(), items))
    {
        chip_id_ = items[0][0];

        items.clear();
        dbhandler.show_items("def_project_role", {"project_role_id", "project_role"}, items, "", "order by project_role_id");
        assert(items.size() > 0);
        project_role_id_ = items[0][0], project_role_ = items[0][1];

        items.clear();
        dbhandler.insert_item("chip_designer", {"chip_id", "user_id", "project_role_id"}, {chip_id_, creater_id_, project_role_id_});
        dbhandler.show_items("chip_designer", {"chip_designer_id"}, {{"chip_id", chip_id_}, {"user_id", creater_id_}}, items);
        chip_designer_id_ = items[0][0];

        QMessageBox::information(this, "New Chip", "Chip successfully created!");
        return true;
    }
    else
    {
        QMessageBox::warning(this, "New Chip", QString("Chip creation failed\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}


QString EditChipDialog::get_chip_designer_id() const
{
    return chip_designer_id_;
}

QString EditChipDialog::get_project_role() const
{
    return project_role_;
}

QString EditChipDialog::get_project_role_id() const
{
    return project_role_id_;
}
