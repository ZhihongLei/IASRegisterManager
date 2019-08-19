#include "edit_system_block_dialog.h"
#include "ui_edit_system_block_dialog.h"
#include "database_handler.h"
#include "data_utils.h"
#include "edit_chip_designer_dialog.h"
#include "global_variables.h"
#include <QMessageBox>
#include <QtMath>
#include <QDebug>

EditSystemBlockDialog::EditSystemBlockDialog(const QString& chip_id, int address_width, bool can_add_designer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditSystemBlockDialog),
    chip_id_(chip_id),
    address_width_(address_width),
    mode_(DIALOG_MODE::ADD),
    enabled_(true)
{
    bool success = setup_ui();
    ui->pushButtonAddDesigner->setEnabled(enabled_ && can_add_designer);
    setWindowTitle("New System Block");
    if (!success) if (!success) QMessageBox::warning(this, windowTitle(), "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}

EditSystemBlockDialog::EditSystemBlockDialog(const QString& chip_id, const QString& block_id, int address_width, bool can_add_designer, bool enabled, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditSystemBlockDialog),
    chip_id_(chip_id),
    block_id_(block_id),
    address_width_(address_width),
    mode_(DIALOG_MODE::EDIT),
    enabled_(enabled)
{
    bool success = setup_ui();
    setWindowTitle("Edit System Block");
    ui->pushButtonAddDesigner->setEnabled(enabled_ && can_add_designer);
    QVector<QVector<QString> > items;
    
    success = success && DataBaseHandler::show_items("block_system_block", {"block_name", "abbreviation", "start_address", "responsible"}, "block_id", get_block_id(), items);
    assert (items.size() == 1);
    ui->lineEditName->setText(items[0][0]);
    ui->lineEditAbbr->setText(items[0][1]);
    ui->lineEditStartAddr->setText(items[0][2]);
    original_abbr_ = items[0][1];
    original_block_name_ = items[0][0];
    for (int i = 0; i < ui->comboBoxResponsible->count(); i++)
    {
        QString user = ui->comboBoxResponsible->itemText(i);
        if (responsible2user_id_[user] == items[0][3])
        {
            ui->comboBoxResponsible->setCurrentIndex(i);
            break;
        }
    }
    if (!success) if (!success) QMessageBox::warning(this, windowTitle(), "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}

bool EditSystemBlockDialog::setup_ui()
{
    ui->setupUi(this);
    bool success = true;
    
    QVector<QVector<QString> > items;
    success = success && DataBaseHandler::show_items_inner_join({"global_user.username", "global_user.user_id"}, {{{"global_user", "user_id"}, {"chip_designer", "user_id"}}},
                                    items,
                                    {{"chip_designer.chip_id", chip_id_}},
                                    "order by chip_designer.chip_designer_id");
    for (const auto& item : items)
    {
        ui->comboBoxResponsible->addItem(item[0]);
        responsible2user_id_[item[0]] = item[1];
    }

    if (ui->comboBoxResponsible->count() == 0)
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

    ui->lineEditStartAddr->setValidator(new HexValueValidator(address_width_));
    ui->lineEditStartAddr->setText("0x");

    QVector<QWidget*> widgets = {ui->lineEditName, ui->lineEditAbbr, ui->lineEditStartAddr, ui->comboBoxResponsible, ui->pushButtonAddDesigner};
    for (QWidget* w : widgets) w->setEnabled(enabled_);
    return success;
}

EditSystemBlockDialog::~EditSystemBlockDialog()
{
    delete ui;
}

QString EditSystemBlockDialog::get_block_name() const
{
    return ui->lineEditName->text();
}

QString EditSystemBlockDialog::get_block_id() const
{
    return block_id_;
}

QString EditSystemBlockDialog::get_block_abbr() const
{
    return ui->lineEditAbbr->text();
}

QString EditSystemBlockDialog::get_start_addr() const
{
    return normalize_hex(ui->lineEditStartAddr->text(), address_width_);
}

QString EditSystemBlockDialog::get_responsible() const
{
    return ui->comboBoxResponsible->currentText();
}

QString EditSystemBlockDialog::get_responsible_id() const
{
    return responsible2user_id_[get_responsible()];
}

bool EditSystemBlockDialog::designer_added() const
{
    return designer_added_;
}

void EditSystemBlockDialog::on_pushButtonAddDesigner_clicked()
{
    EditChipDesignerDialog add_designer(chip_id_, this);
    if (add_designer.exec() == QDialog::Accepted && add_designer.add_designer())
    {
        ui->comboBoxResponsible->addItem(add_designer.get_username());
        ui->comboBoxResponsible->setCurrentIndex(ui->comboBoxResponsible->count() - 1);
        responsible2user_id_[add_designer.get_username()] = add_designer.get_user_id();
        designer_added_ = true;
    }
}

void EditSystemBlockDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) return QDialog::accept();
}

bool EditSystemBlockDialog::sanity_check()
{
    return check_block_name() && check_block_abbreviation() && check_start_address();
}

bool EditSystemBlockDialog::check_block_name()
{
    if (get_block_name() == "")
    {
        QMessageBox::warning(this, windowTitle(), "System block name must not be empty!");
        return false;
    }
    if (mode_ == DIALOG_MODE::EDIT && get_block_name() == original_block_name_) return true;

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("block_system_block", {"block_id"}, {{"chip_id", chip_id_}, {"block_name", get_block_name()}}, items))
    {
        QMessageBox::warning(this, "System Block Editor", "Unable to validate block name due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    if (items.size() > 0)
    {
        QMessageBox::warning(this, windowTitle(), "System block " + get_block_name() + " already exists!");
        return false;
    }
    return true;
}

bool EditSystemBlockDialog::check_block_abbreviation()
{
    if (get_block_abbr() == "")
    {
        QMessageBox::warning(this, windowTitle(), "System block abbreviation must not be empty!");
        return false;
    }
    if (mode_ == DIALOG_MODE::EDIT && get_block_abbr() == original_abbr_) return true;

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("block_system_block", {"block_id"}, {{"chip_id", chip_id_}, {"abbreviation", get_block_abbr()}}, items))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate block abbreviation due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    if (items.size() > 0)
    {
        QMessageBox::warning(this, windowTitle(), "System block abbreviation " + get_block_abbr() + " already exists!");
        return false;
    }
    return true;
}

bool EditSystemBlockDialog::check_start_address()
{
    if (ui->lineEditStartAddr->text() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Please give a valid start address!");
        return false;
    }

    QVector<QVector<QString> > items;

    bool success = true;
    success = success && DataBaseHandler::show_items("block_system_block", {"block_id", "start_address", "block_name"}, "chip_id", chip_id_, items);

    if (mode_ == DIALOG_MODE::ADD) items.append({block_id_, ui->lineEditStartAddr->text(), ui->lineEditName->text()});

    int i = 0;
    while (i < items.size() && items[i][0] != block_id_) i++;
    items[i][1] = ui->lineEditStartAddr->text();
    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toULongLong(nullptr, 16) < b[1].toULongLong(nullptr, 16);});

    i = 0;
    while (i < items.size() && items[i][0] != block_id_) i++;
    qulonglong num_regs = 0, num_prev_regs = 0;
    qulonglong start_addr = ui->lineEditStartAddr->text().toULongLong(nullptr, 16),
               prev_start_addr = 0,
               next_start_addr = (i == items.size() -1) ?  quint64(qPow(2, address_width_)+0.5) : items[i+1][1].toULongLong(nullptr, 16);

    if (mode_ == DIALOG_MODE::EDIT)
    {
        QVector<QString> item;
        success = success && DataBaseHandler::show_one_item("block_register", item, {"count(reg_id)"}, "block_id", block_id_);
        num_regs = item[0].toULongLong();
    }
    if (i != 0)
    {
        QVector<QString> item;
        success = success && DataBaseHandler::show_one_item("block_register", item, {"count(reg_id)"}, "block_id", items[i-1][0]);
        num_prev_regs = item[0].toULongLong();
        prev_start_addr = items[i-1][1].toULongLong(nullptr, 16);
    }

    if (!success)
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate start address due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }

    if (num_prev_regs > start_addr - prev_start_addr)
    {
        if (i == 0) QMessageBox::warning(this, windowTitle(), "Start address must be a positive number!");
        else QMessageBox::warning(this, windowTitle(), "Register address overlaps with block " + items[i-1][2] + "!");
        return false;
    }
    if (num_regs > next_start_addr - start_addr)
    {
        if ( i == items.size() -1) QMessageBox::warning(this, windowTitle(), "Register address exceeds address width " + QString::number(address_width_) + "!");
        else QMessageBox::warning(this, windowTitle(), "Register address overlaps with block " + items[i+1][2] + "!");
        return false;
    }
    return true;
}

bool EditSystemBlockDialog::add_system_block()
{

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items("block_system_block", {"block_id"}, {{"next", "-1"}, {"chip_id", chip_id_}}, items))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to add system block due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    assert (items.size() <= 1);
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    if (DataBaseHandler::get_next_auto_increment_id("block_system_block", "block_id", block_id_) &&
            DataBaseHandler::insert_item("block_system_block",
                        {"block_id", "block_name", "abbreviation", "chip_id", "responsible", "start_address", "prev", "next"},
                        {block_id_, get_block_name(), get_block_abbr(), chip_id_, get_responsible_id(), get_start_addr(), prev, "-1"}) )
    {
        bool success = true;
        if (prev != "-1") success = success && DataBaseHandler::update_items("block_system_block", {{"block_id", prev}}, {{"next", block_id_}});
        if (success)
        {
            DataBaseHandler::commit();
            return true;
        }
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, windowTitle(), "Unable to create system block.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditSystemBlockDialog::edit_system_block()
{

    if(DataBaseHandler::update_items("block_system_block",
                               {{"block_id", get_block_id()}},
                                {{"block_name", get_block_name()}, {"abbreviation", get_block_abbr()}, {"responsible", get_responsible_id()}, {"start_address", get_start_addr()}}))
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, windowTitle(), "Unable to edit system block.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}
