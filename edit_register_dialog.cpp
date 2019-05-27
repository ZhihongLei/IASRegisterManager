#include "edit_register_dialog.h"
#include "ui_edit_register_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QMessageBox>

EditRegisterDialog::EditRegisterDialog(const QString& block_id, QWidget *parent) :
    QDialog(parent),
    block_id_(block_id),
    ui(new Ui::EditRegisterDialog),
    mode_(DIALOG_MODE::ADD),
    enabled_(true)
{
    setup_ui();
    setWindowTitle("Add Register");
}

EditRegisterDialog::EditRegisterDialog(const QString& block_id, const QString& reg_id, bool enabled, QWidget* parent):
    QDialog (parent),
    block_id_(block_id),
    reg_id_(reg_id),
    ui(new Ui::EditRegisterDialog),
    mode_(DIALOG_MODE::EDIT),
    enabled_(enabled)
{
     setup_ui();
     ui->comboBoxRegType->setEnabled(false);
     setWindowTitle("Edit Register");

     DataBaseHandler dbhandler(gDBHost, gDatabase);
     QVector<QVector<QString> > items;
     dbhandler.show_items("block_register", {"reg_name"}, "reg_id", get_reg_id(), items);
     assert (items.size() == 1);
     ui->lineEditRegName->setText(items[0][0]);
     original_register_name_ = items[0][0];
}

void EditRegisterDialog::setup_ui()
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("def_register_type", {"reg_type_id", "reg_type"}, items, "", "order by reg_type_id");
    for (const auto& item : items)
    {
        ui->comboBoxRegType->addItem(item[1]);
        reg_type_ids_.push_back(item[0]);
    }
    ui->comboBoxRegType->setEnabled(enabled_);
    ui->lineEditRegName->setEnabled(enabled_);
}

EditRegisterDialog::~EditRegisterDialog()
{
    delete ui;
}

QString EditRegisterDialog::get_reg_name() const
{
    return ui->lineEditRegName->text();
}

QString EditRegisterDialog::get_reg_id() const
{
    return reg_id_;
}

QString EditRegisterDialog::get_reg_type() const
{
    return ui->comboBoxRegType->currentText();
}

QString EditRegisterDialog::get_reg_type_id() const
{
    return reg_type_ids_[ui->comboBoxRegType->currentIndex()];
}


bool EditRegisterDialog::sanity_check()
{

    QString warning_title = mode_ == DIALOG_MODE::ADD ? "Add Register" : "Edit Register";
    if (get_reg_name() == "")
    {
        QMessageBox::warning(this, warning_title, "Register name must not be empty!");
        return false;
    }
    if (mode_ == EDIT && get_reg_name() == original_register_name_) return true;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_register", {"reg_name"}, {{"block_id", block_id_}, {"reg_name", get_reg_name()}}, items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, warning_title, "Register " + get_reg_name() + " already exists!");
        return false;
    }
    // anything else?
    return true;
}

bool EditRegisterDialog::add_register()
{

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_register", {"reg_id"}, {{"next", "-1"}, {"block_id", block_id_}}, items);
    assert (items.size() <= 1);
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    QVector<QString> fields = {"reg_name", "block_id", "reg_type_id", "prev", "next"};
    QVector<QString> values = {get_reg_name(), block_id_, get_reg_type_id(),  prev, "-1"};

    if (dbhandler.insert_item("block_register", fields, values) && \
        dbhandler.show_items("block_register", {"reg_id"}, {{"block_id", block_id_}, {"reg_name", get_reg_name()}}, items))
    {
        reg_id_ = items[0][0];
        if (prev != "-1") dbhandler.update_items("block_register", {{"reg_id", prev}}, {{"next", reg_id_}});
        return true;
    }
    else
    {
        QMessageBox::warning(this, "Create Register", QString("Creating register failed\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}

bool EditRegisterDialog::edit_register()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_register", {"reg_name"}, "reg_id", get_reg_id(), items);
    assert (items.size() == 1);
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    QVector<QString> fields = {"reg_name"};
    return dbhandler.update_items("block_register", {{"reg_id", get_reg_id()}}, {{"reg_name",get_reg_name()}});
}

void EditRegisterDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) return QDialog::accept();
}
