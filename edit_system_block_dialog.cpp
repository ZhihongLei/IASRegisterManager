#include "edit_system_block_dialog.h"
#include "ui_edit_system_block_dialog.h"
#include "database_handler.h"
#include "data_utils.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QMessageBox>

EditSystemBlockDialog::EditSystemBlockDialog(const QString& chip_id, int address_width, QWidget *parent) :
    QDialog(parent),
    chip_id_(chip_id),
    ui(new Ui::EditSystemBlockDialog),
    address_width_(address_width),
    mode_(MODE::NEW)
{
    setup_ui();
}

EditSystemBlockDialog::EditSystemBlockDialog(const QString& chip_id, const QString& block_id, int address_width, QWidget *parent) :
    QDialog(parent),
    chip_id_(chip_id),
    ui(new Ui::EditSystemBlockDialog),
    address_width_(address_width),
    mode_(MODE::EDIT),
    block_id_(block_id)
{
    setup_ui();
    QVector<QVector<QString> > items;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.show_items("block_system_block", {"block_name", "abbreviation", "start_address", "responsible"}, "block_id", get_block_id(), items);
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
}

void EditSystemBlockDialog::setup_ui()
{

    ui->setupUi(this);

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"global_user.username", "global_user.user_id"}, {{{"global_user", "user_id"}, {"chip_designer", "user_id"}}},
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
}

EditSystemBlockDialog::~EditSystemBlockDialog()
{
    delete ui;
}

void EditSystemBlockDialog::accept()
{
    if (sanity_check()) return QDialog::accept();
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
    return ui->lineEditStartAddr->text();
}

QString EditSystemBlockDialog::get_responsible() const
{
    return ui->comboBoxResponsible->currentText();
}

QString EditSystemBlockDialog::get_responsible_id() const
{
    return responsible2user_id_[get_responsible()];
}

bool EditSystemBlockDialog::check_block_name()
{
    QString title = mode_ == MODE::NEW ? "Create System Block" : "Edit System Block";
    if (get_block_name() == "")
    {
        QMessageBox::warning(this, title, "System block name must not be empty!");
        return false;
    }
    if (mode_ == MODE::EDIT && get_block_name() == original_block_name_) return true;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_system_block", {"block_id"}, {{"chip_id", chip_id_}, {"block_name", get_block_name()}}, items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, title, "System block " + get_block_name() + " already exists!");
        return false;
    }
    return true;
}


bool EditSystemBlockDialog::check_block_abbreviation()
{
    QString title = mode_ == MODE::NEW ? "Create System Block" : "Edit System Block";
    if (get_block_abbr() == "")
    {
        QMessageBox::warning(this, title, "System block abbreviation must not be empty!");
        return false;
    }
    if (mode_ == MODE::EDIT && get_block_abbr() == original_abbr_) return true;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_system_block", {"block_id"}, {{"chip_id", chip_id_}, {"abbreviation", get_block_abbr()}}, items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, title, "System block abbreviation " + get_block_abbr() + " already exists!");
        return false;
    }
    return true;
}

bool EditSystemBlockDialog::check_start_address()
{
    // TODO
    QString title = mode_ == MODE::NEW ? "Create System Block" : "Edit System Block";
    if (ui->lineEditStartAddr->text() == "")
    {
        QMessageBox::warning(this, title, "Please give a valid start address!");
        return false;
    }
    return true;
}

bool EditSystemBlockDialog::sanity_check()
{
    return check_block_name() && check_block_abbreviation() && check_start_address();
}


bool EditSystemBlockDialog::add_system_block()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_system_block", {"block_id"}, {{"next", "-1"}, {"chip_id", chip_id_}}, items);
    assert (items.size() <= 1);
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    QVector<QString> fields = {"block_name", "abbreviation", "chip_id", "responsible", "start_address", "prev", "next"};
    QVector<QString> values = {get_block_name(), get_block_abbr(), chip_id_,
                                       get_responsible_id(), get_start_addr(), prev, "-1"};

    if (dbhandler.insert_item("block_system_block", fields, values) && \
            dbhandler.show_items("block_system_block", {"block_id"}, {{"block_name", get_block_name()}, {"chip_id", chip_id_}}, items))
    {
        block_id_ = items[0][0];
        if (prev != "-1") dbhandler.update_items("block_system_block", {{"block_id", prev}}, {{"next", block_id_}});
        return true;
    }
    else
    {
        QMessageBox::warning(this, "New System Block", QString("System block not created\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}

bool EditSystemBlockDialog::edit_system_block()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    return dbhandler.update_items("block_system_block", {{"block_id", get_block_id()}}, {{"block_name", get_block_name()}, {"abbreviation", get_block_abbr()}, {"responsible", get_responsible_id()}, {"start_address", get_start_addr()}});
}
