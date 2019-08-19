#include "edit_register_dialog.h"
#include "ui_edit_register_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QMessageBox>
#include <QtMath>
#include <QDebug>

EditRegisterDialog::EditRegisterDialog(const QString& chip_id, const QString& block_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditRegisterDialog),
    chip_id_(chip_id),
    block_id_(block_id),
    enabled_(true),
    mode_(DIALOG_MODE::ADD)
{
    bool success = setup_ui();
    setWindowTitle("Add Register");
    if (!success)
       QMessageBox::warning(this, "Add Register", "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}

EditRegisterDialog::EditRegisterDialog(const QString& chip_id, const QString& block_id, const QString& reg_id, bool enabled, QWidget* parent):
    QDialog (parent),
    ui(new Ui::EditRegisterDialog),
    chip_id_(chip_id),
    block_id_(block_id),
    reg_id_(reg_id),
    enabled_(enabled),
    mode_(DIALOG_MODE::EDIT)

{
     bool success = setup_ui();
     ui->comboBoxRegType->setEnabled(false);
     setWindowTitle("Edit Register");
     QVector<QVector<QString> > items;
     if (DataBaseHandler::show_items("block_register", {"reg_name"}, "reg_id", get_reg_id(), items) && items.size() == 1)
     {
         ui->lineEditRegName->setText(GLOBAL_REGISTER_NAMING.get_extended_name(items[0][0]));
         original_register_name_ = ui->lineEditRegName->text();
     }
     else success = false;
     if (!success)
        QMessageBox::warning(this, "Edit Register", "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}

bool EditRegisterDialog::setup_ui()
{
    ui->setupUi(this);
    ui->lineEditRegName->setValidator(new QRegExpValidator(QRegExp(GLOBAL_REGISTER_NAMING.get_extended_name("_?[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*_?"))));
    ui->lineEditRegName->setText(GLOBAL_REGISTER_NAMING.get_extended_name(""));
    ui->lineEditRegName->setCursorPosition(GLOBAL_REGISTER_NAMING.get_extended_name("***").indexOf("***"));

    QVector<QVector<QString> > items;
    bool success = DataBaseHandler::show_items("def_register_type", {"reg_type_id", "reg_type"}, items, "", "order by reg_type_id");
    for (const auto& item : items)
    {
        ui->comboBoxRegType->addItem(item[1]);
        reg_type_ids_.push_back(item[0]);
    }
    ui->comboBoxRegType->setEnabled(enabled_);
    ui->lineEditRegName->setEnabled(enabled_);
    return success;
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

QString EditRegisterDialog::get_address() const
{
    return address_;
}

void EditRegisterDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) return QDialog::accept();
}

bool EditRegisterDialog::sanity_check()
{
    return check_address() && check_name();
}

bool EditRegisterDialog::check_name()
{
    QRegularExpression re(GLOBAL_REGISTER_NAMING.get_extended_name("[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*"));
    QRegularExpressionMatch match = re.match(get_reg_name());
    if (!match.hasMatch())
    {
        QMessageBox::warning(this, windowTitle(), "Register name must match "+  GLOBAL_REGISTER_NAMING.get_extended_name("${NAME}") + " format!");
        return false;
    }
    if (mode_ == EDIT && get_reg_name() == original_register_name_) return true;

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("block_register", {"reg_name"}, {{"block_id", block_id_}, {"reg_name", get_reg_name()}}, items))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate register name due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    if (items.size() > 0)
    {
        QMessageBox::warning(this, windowTitle(), "Register " + get_reg_name() + " already exists!");
        return false;
    }
    // anything else?
    return true;
}

bool EditRegisterDialog::check_address()
{
    if (mode_ == DIALOG_MODE::EDIT) return true;

    QVector<QVector<QString> > items;

    if (!DataBaseHandler::show_items("block_system_block", {"block_id", "start_address", "block_name"}, "chip_id", chip_id_, items))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate register address due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toULongLong(nullptr, 16) < b[1].toULongLong(nullptr, 16);});

    int i = 0;
    while (i < items.size() && items[i][0] != block_id_) i++;

    quint64 num_regs = 0;
    QVector<QString> item;
    if (!DataBaseHandler::show_one_item("block_register", item, {"count(reg_id)"}, "block_id", block_id_))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate register address due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    num_regs = item[0].toULongLong();

    quint64 start_addr = items[i][1].toULongLong(nullptr, 16);
    quint64 next_start_addr;
    int address_width = 0;

    item.clear();
    if (!DataBaseHandler::show_one_item("chip_chip", item, {"address_width"}, "chip_id", chip_id_))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate register address due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    address_width = item[0].toInt();

    if (i == items.size() - 1)
        next_start_addr = quint64(qPow(2, address_width) + 0.5);
    else next_start_addr = items[i+1][1].toULongLong(nullptr, 16);

    if (num_regs+1 > next_start_addr - start_addr)
    {
        if (i == items.size() - 1) QMessageBox::warning(this, windowTitle(), "Too many registers!\nAddress exceeds address width " + QString::number(address_width) + "!");
        else QMessageBox::warning(this, windowTitle(), "Too many registers!\nAddress overlaps with block " + items[i+1][2] + "!");
        return false;
    }
    address_ = decimal2hex(num_regs + start_addr, address_width);
    return true;
}

bool EditRegisterDialog::add_register()
{

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("block_register", {"reg_id"}, {{"next", "-1"}, {"block_id", block_id_}}, items))
    {
        QMessageBox::warning(this, "Add Register", "Unable to add register due to database connection issue.\nPlease try again..\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    if (DataBaseHandler::get_next_auto_increment_id("block_register", "reg_id", reg_id_) &&
            DataBaseHandler::insert_item("block_register", {"reg_id", "reg_name", "block_id", "reg_type_id", "prev", "next"},
            {reg_id_, GLOBAL_REGISTER_NAMING.get_given_name(get_reg_name()), block_id_, get_reg_type_id(),  prev, "-1"}))
    {
        bool success = true;
        if (prev != "-1") success = success && DataBaseHandler::update_items("block_register", {{"reg_id", prev}}, {{"next", reg_id_}});
        if (success)
        {
            DataBaseHandler::commit();
            return true;
        }
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Add Register", "Unable to add register.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditRegisterDialog::edit_register()
{
    if (DataBaseHandler::update_items("block_register", {{"reg_id", get_reg_id()}}, {{"reg_name", GLOBAL_REGISTER_NAMING.get_given_name(get_reg_name())}}))
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Edit Register", "Unable to edit register.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}
