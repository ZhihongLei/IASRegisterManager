#include "edit_register_dialog.h"
#include "ui_edit_register_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QMessageBox>
#include <QtMath>
#include <QDebug>

EditRegisterDialog::EditRegisterDialog(const QString& chip_id, const QString& block_id, QWidget *parent) :
    QDialog(parent),
    block_id_(block_id),
    ui(new Ui::EditRegisterDialog),
    mode_(DIALOG_MODE::ADD),
    enabled_(true),
    chip_id_(chip_id)
{
    setup_ui();
    setWindowTitle("Add Register");
}

EditRegisterDialog::EditRegisterDialog(const QString& chip_id, const QString& block_id, const QString& reg_id, bool enabled, QWidget* parent):
    QDialog (parent),
    block_id_(block_id),
    reg_id_(reg_id),
    ui(new Ui::EditRegisterDialog),
    mode_(DIALOG_MODE::EDIT),
    enabled_(enabled),
    chip_id_(chip_id)
{
     setup_ui();
     ui->comboBoxRegType->setEnabled(false);
     setWindowTitle("Edit Register");

     DataBaseHandler dbhandler(gDBHost, gDatabase);
     QVector<QVector<QString> > items;
     dbhandler.show_items("block_register", {"reg_name"}, "reg_id", get_reg_id(), items);
     assert (items.size() == 1);
     ui->lineEditRegName->setText(REGISTER_NAMING.get_extended_name(items[0][0]));
     original_register_name_ = ui->lineEditRegName->text();
}

void EditRegisterDialog::setup_ui()
{
    ui->setupUi(this);
    ui->lineEditRegName->setValidator(new QRegExpValidator(QRegExp(REGISTER_NAMING.get_extended_name("_?[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*_?"))));
    ui->lineEditRegName->setText(REGISTER_NAMING.get_extended_name(""));
    ui->lineEditRegName->setCursorPosition(REGISTER_NAMING.get_extended_name("***").indexOf("***"));
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

bool EditRegisterDialog::check_name()
{
    QString warning_title = mode_ == DIALOG_MODE::ADD ? "Add Register" : "Edit Register";
    QRegularExpression re(REGISTER_NAMING.get_extended_name("[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*"));
    QRegularExpressionMatch match = re.match(get_reg_name());
    if (!match.hasMatch())
    {
        QMessageBox::warning(this, warning_title, "Register name must match "+  REGISTER_NAMING.get_extended_name("${NAME}") + " format!");
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

bool EditRegisterDialog::check_address()
{
    if (mode_ == DIALOG_MODE::EDIT) return true;
    QString warning_title = "Add Register";

    QVector<QVector<QString> > items;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.show_items("block_system_block", {"block_id", "start_address", "block_name"}, "chip_id", chip_id_, items);
    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toLongLong(nullptr, 16) < b[1].toLongLong(nullptr, 16);});

    int i = 0;
    while (i < items.size() && items[i][0] != block_id_) i++;

    int num_regs = 0;
    QVector<QString> item;
    dbhandler.show_one_item("block_register", item, {"count(reg_id)"}, "block_id", block_id_);
    num_regs = item[0].toInt();

    qlonglong start_addr = items[i][1].toLongLong(nullptr, 16),
                next_start_addr,
                address_width = 0;

    if (i == items.size() - 1)
    {
        item.clear();
        dbhandler.show_one_item("chip_chip", item, {"address_width"}, "chip_id", chip_id_);
        address_width = item[0].toInt();
        next_start_addr = qRound64(qPow(2, address_width));
    }
    else next_start_addr = items[i+1][1].toLongLong(nullptr, 16);

    if (num_regs+1 > next_start_addr - start_addr)
    {
        if (i == items.size() - 1) QMessageBox::warning(this, warning_title, "Too many registers!\nAddress exceeds address width " + QString::number(address_width) + "!");
        else QMessageBox::warning(this, warning_title, "Too many registers!\nAddress overlaps with block " + items[i+1][2] + "!");
        return false;
    }
    return true;
}


bool EditRegisterDialog::sanity_check()
{
    return check_address() && check_name();
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
    QVector<QString> values = {REGISTER_NAMING.get_shortened_name(get_reg_name()), block_id_, get_reg_type_id(),  prev, "-1"};

    if (dbhandler.insert_item("block_register", fields, values) && \
        dbhandler.show_items("block_register", {"reg_id"}, {{"block_id", block_id_}, {"reg_name", REGISTER_NAMING.get_shortened_name(get_reg_name())}}, items))
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
    return dbhandler.update_items("block_register", {{"reg_id", get_reg_id()}}, {{"reg_name", REGISTER_NAMING.get_shortened_name(get_reg_name())}});
}

void EditRegisterDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) return QDialog::accept();
}
