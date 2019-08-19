#include "edit_register_page_dialog.h"
#include "ui_edit_register_page_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include "data_utils.h"
#include <QtMath>
#include <QMessageBox>
#include <QIntValidator>
#include <QDebug>

EditRegisterPageDialog::EditRegisterPageDialog(const QString& chip_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditRegisterPageDialog),
    chip_id_(chip_id),
    mode_(DIALOG_MODE::ADD)
{
    ui->setupUi(this);
    bool success = setup_ui();
    setWindowTitle("Add Register Page");
    if (!success) QMessageBox::warning(this, "Add Register Page", "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}


EditRegisterPageDialog::EditRegisterPageDialog(const QString& chip_id, const QString& page_id, QWidget *parent):
    QDialog(parent),
    ui(new Ui::EditRegisterPageDialog),
    chip_id_(chip_id),
    page_id_(page_id),
    mode_(DIALOG_MODE::EDIT)

{
    ui->setupUi(this);
    bool success = setup_ui();
    setWindowTitle("Edit Register Page");


    QVector<QVector<QString> > items;

    QVector<QString> fields = {"chip_register_page.page_name", "chip_register_page.ctrl_sig", "chip_register_page.page_count",
                               "signal_signal.sig_name", "signal_signal.width", "block_system_block.block_name",
                               "block_system_block.abbreviation", "signal_signal.add_port"};
    success = success && DataBaseHandler::show_items_inner_join(fields,
                                    {{{"chip_register_page", "ctrl_sig"}, {"signal_signal", "sig_id"}}, {{"signal_signal", "block_id"}, {"block_system_block", "block_id"}}},
                                    items, {{"chip_register_page.page_id", page_id_}});

    if (items.size() == 0)
    {
        QMessageBox::warning(this, "Edit Register Page", "Unable to open register pages.\nError message: " + DataBaseHandler::get_error_message());
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->comboBoxControlSignal->count() > 0);
        ui->tableWidgetAll->setEnabled(false);
        ui->tableWidgetSelected->setEnabled(false);
        return;
    }

    QString page_name = items[0][0], control_sig_id = items[0][1], page_count = items[0][2];
    int width = items[0][4].toInt();
    QString block_name = items[0][5], block_abbr = items[0][6];

    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING,
            register_naming = GLOBAL_REGISTER_NAMING;
    signal_naming.update_key("${BLOCK_NAME}", block_name);
    signal_naming.update_key("${BLOCK_ABBR}", block_abbr);
    QString control_sig_name = items[0][7]=="1" ? signal_naming.get_extended_name(items[0][3]) : items[0][3];

    original_control_sig_id_ = control_sig_id;
    original_page_name_ = page_name;
    sig_name2id_[control_sig_name] = control_sig_id;
    sig_name2width_[control_sig_name] = width;

    ui->comboBoxControlSignal->addItem(control_sig_name);
    ui->comboBoxControlSignal->setCurrentIndex(ui->comboBoxControlSignal->count() - 1);

    ui->lineEditName->setText(page_name);
    ui->lineEditPageCount->setText(page_count);

    items.clear();
    success = success && DataBaseHandler::show_items_inner_join({"block_register.reg_id", "block_register.reg_name", "block_system_block.block_name", "block_system_block.abbreviation"},
                                    {{{"chip_register_page_content", "reg_id"}, {"block_register", "reg_id"}}, {{"block_register", "block_id"}, {"block_system_block", "block_id"}}},
                                    items, {{"chip_register_page_content.page_id", page_id}});
    QSet<QString> selected_regs;
    for (const auto& item : items)
    {
        QString block_name = item[2], block_abbr = item[3];
        register_naming.update_key("${BLOCK_NAME}", block_name);
        register_naming.update_key("${BLOCK_ABBR}", block_abbr);
        int row = ui->tableWidgetSelected->rowCount();
        ui->tableWidgetSelected->insertRow(row);
        ui->tableWidgetSelected->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableWidgetSelected->setItem(row, 1, new QTableWidgetItem(register_naming.get_extended_name(item[1])));
    }
    if (!success) QMessageBox::warning(this, "Add Register Page", "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
}

EditRegisterPageDialog::~EditRegisterPageDialog()
{
    delete ui;
}

bool EditRegisterPageDialog::setup_ui()
{
    ui->tableWidgetAll->setColumnHidden(0, true);
    ui->tableWidgetSelected->setColumnHidden(0, true);
    QVector<QVector<QString> > items;

    if (!DataBaseHandler::show_items("block_system_block", {"block_id", "block_name", "abbreviation", "prev", "next"}, "chip_id", chip_id_, items))
        return false;
    bool success = true;
    items = sort_doubly_linked_list(items);
    QSet<QString> available_reg_types = get_available_paged_register_types();
    QSet<QString> available_sig_types = get_available_control_signal_types();
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING,
            register_naming = GLOBAL_REGISTER_NAMING;
    for (const auto& item : items)
    {
        register_naming.update_key("${BLOCK_NAME}", item[1]);
        register_naming.update_key("${BLOCK_ABBR}", item[2]);
        signal_naming.update_key("${BLOCK_NAME}", item[1]);
        signal_naming.update_key("${BLOCK_ABBR}", item[2]);

        QVector<QVector<QString> > reg_items;
        success = success && DataBaseHandler::show_items_inner_join({"chip_register_page_content.reg_id"},
                                        {{{"chip_register_page_content", "page_id"}, {"chip_register_page", "page_id"}}},
                                        reg_items, {{"chip_register_page.chip_id", chip_id_}});
        QSet<QString> regs_in_pages;
        for (const auto& reg_item : reg_items) regs_in_pages.insert(reg_item[0]);
        reg_items.clear();

        success = success && DataBaseHandler::show_items("block_register", {"reg_id", "reg_name", "reg_type_id", "prev", "next"}, "block_id", item[0], reg_items);
        reg_items = sort_doubly_linked_list(reg_items);
        for (const auto& reg_item : reg_items)
        {
            if (regs_in_pages.contains(reg_item[0])) continue;
            if (!available_reg_types.contains(reg_item[2])) continue;
            reg_id2name_[reg_item[0]] = register_naming.get_extended_name(reg_item[1]);
        }

        QVector<QVector<QString> > sig_items;
        success = success && DataBaseHandler::show_items("signal_signal", {"sig_id", "sig_name", "width", "add_port", "sig_type_id"}, "block_id", item[0], sig_items);
        for (const auto& sig_item : sig_items)
        {
            QString sig_type_id = sig_item[4];
            if (!available_sig_types.contains(sig_type_id)) continue;
            QString sig_id = sig_item[0],
                    sig_name = sig_item[3]=="1" ? signal_naming.get_extended_name(sig_item[1]) : sig_item[1];
            int width = sig_item[2].toInt();
            QVector<QString> page_item;
            success = success && DataBaseHandler::show_one_item("chip_register_page", page_item, {"page_id"}, "ctrl_sig", sig_id);
            if (page_item.size() == 0)
            {
                if (sig_name2id_.contains(sig_name)) sig_name = item[2] + ":" + sig_name;
                sig_name2id_[sig_name] = sig_id;
                sig_name2width_[sig_name] = width;
                ui->comboBoxControlSignal->addItem(sig_name);
            }
        }
    }

    ui->pushButton->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->comboBoxControlSignal->count() > 0);
    emit(ui->comboBoxControlSignal->currentIndexChanged(ui->comboBoxControlSignal->currentIndex()));
    return success;
}

QString EditRegisterPageDialog::get_register_page_id() const
{
    return page_id_;
}

QString EditRegisterPageDialog::get_register_page_name() const
{
    return ui->lineEditName->text();
}

QString EditRegisterPageDialog::get_control_signal_name() const
{
    return ui->comboBoxControlSignal->currentText();
}

QString EditRegisterPageDialog::get_control_signal_id() const
{
    return sig_name2id_[get_control_signal_name()];
}

QString EditRegisterPageDialog::get_page_count() const
{
    return ui->lineEditPageCount->text();
}

void EditRegisterPageDialog::on_comboBoxControlSignal_currentIndexChanged(int index)
{
    ui->tableWidgetSelected->setRowCount(0);
    int width = sig_name2width_[ui->comboBoxControlSignal->currentText()];
    ui->lineEditPageCount->setValidator(new QIntValidator(1, qRound(qPow(2, width)), this));
    ui->lineEditPageCount->setText(QString::number(qRound(qPow(2, width))));

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"block_sig_reg_partition_mapping.reg_id"},
                                    {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}}, {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}},
                                    items, {{"signal_signal.sig_id", get_control_signal_id()}}))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to filter out ineligible registers due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    QSet<QString> regs;
    for (const auto &item : items)
        regs.insert(item[0]);

    ui->tableWidgetAll->setRowCount(0);
    for (const QString& reg_id: reg_id2name_.keys())
    {
        if (!regs.contains(reg_id))
        {
            int row = ui->tableWidgetAll->rowCount();
            ui->tableWidgetAll->insertRow(row);
            ui->tableWidgetAll->setItem(row, 0, new QTableWidgetItem(reg_id));
            ui->tableWidgetAll->setItem(row, 1, new QTableWidgetItem(reg_id2name_[reg_id]));
        }
    }
}

void EditRegisterPageDialog::on_tableWidgetAll_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    on_pushButton_clicked();
}

void EditRegisterPageDialog::on_tableWidgetSelected_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    on_pushButton_clicked();
}

void EditRegisterPageDialog::on_tableWidgetAll_cellClicked(int row, int column)
{
    if (row < 0) return;
    to_add_register_ = true;
    ui->pushButton->setText(">>");
    ui->pushButton->setEnabled(true);
}

void EditRegisterPageDialog::on_tableWidgetSelected_cellClicked(int row, int column)
{
    if (row < 0) return;
    to_add_register_ = false;
    ui->pushButton->setText("<<");
    ui->pushButton->setEnabled(true);
}

void EditRegisterPageDialog::on_pushButton_clicked()
{
    if (to_add_register_)
    {
        int row = ui->tableWidgetAll->currentRow();
        if (row < 0) return;
        QString reg_id = ui->tableWidgetAll->item(row, 0)->text(),
                reg_name = ui->tableWidgetAll->item(row, 1)->text();
        ui->tableWidgetAll->removeRow(row);
        row = ui->tableWidgetSelected->rowCount();
        ui->tableWidgetSelected->insertRow(row);
        ui->tableWidgetSelected->setItem(row, 0, new QTableWidgetItem(reg_id));
        ui->tableWidgetSelected->setItem(row, 1, new QTableWidgetItem(reg_name));
        ui->pushButton->setEnabled(ui->tableWidgetAll->rowCount() > 0);
    }
    else {
        int row = ui->tableWidgetSelected->currentRow();
        if (row < 0) return;
        QString reg_id = ui->tableWidgetSelected->item(row, 0)->text(),
                reg_name = ui->tableWidgetSelected->item(row, 1)->text();
        ui->tableWidgetSelected->removeRow(row);
        row = ui->tableWidgetAll->rowCount();
        ui->tableWidgetAll->insertRow(row);
        ui->tableWidgetAll->setItem(row, 0, new QTableWidgetItem(reg_id));
        ui->tableWidgetAll->setItem(row, 1, new QTableWidgetItem(reg_name));
        ui->pushButton->setEnabled(ui->tableWidgetSelected->rowCount() > 0);
    }
}

void EditRegisterPageDialog::on_lineEditSearchRegister_textChanged(const QString &pattern)
{
    for (int i = 0; i < ui->tableWidgetAll->rowCount(); i++)
    {
        ui->tableWidgetAll->setRowHidden(i, !ui->tableWidgetAll->item(i, 1)->text().toLower().contains(pattern.toLower()));
        if (ui->tableWidgetAll->currentRow() == i)
        {
            ui->tableWidgetAll->setCurrentCell(-1, -1);
            ui->pushButton->setEnabled(false);
        }
    }
}

void EditRegisterPageDialog::on_lineEditSearchControlSignal_textChanged(const QString &pattern)
{
    ui->comboBoxControlSignal->clear();
    for (const QString& sig_name : sig_name2id_.keys())
    {
        if (QString(sig_name).toLower().contains(pattern.toLower())) ui->comboBoxControlSignal->addItem(sig_name);
    }
}

void EditRegisterPageDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool EditRegisterPageDialog::sanity_check()
{
    if (!check_name()) return false;
    QString sig_id = sig_name2id_[ui->comboBoxControlSignal->currentText()];

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"block_sig_reg_partition_mapping.reg_id"},
                                    {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}}, {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}},
                                    items, {{"signal_signal.sig_id", get_control_signal_id()}}))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate control signal due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    QSet<QString> regs;
    for (const auto &item : items)
        regs.insert(item[0]);

    for (int i = 0; i < ui->tableWidgetSelected->rowCount(); i++)
    {
        QString reg_id = ui->tableWidgetSelected->item(i, 0)->text();
        if (regs.contains(reg_id))
        {
            QMessageBox::warning(this,  windowTitle(), "The control signal cannot be mapped to selected registers!");
            return false;
        }
    }

    int width = sig_name2width_[ui->comboBoxControlSignal->currentText()];
    if (quint64(qPow(2, width) + 0.5) < ui->lineEditPageCount->text().toULongLong())
    {
        QMessageBox::warning(this, windowTitle(), "Register page count should range between 1 and " + QString::number(quint64(qPow(2, width) + 0.5)) + "!");
        return false;
    }
    return true;
}

bool EditRegisterPageDialog::check_name()
{
    QString page_name = ui->lineEditName->text();
    if (page_name == "")
    {
        QMessageBox::warning(this, windowTitle(), "Register page name must not be empty!");
        return false;
    }
    if (mode_ == EDIT && get_register_page_name() == original_page_name_) return true;
    QVector<QString> item;

    if (!DataBaseHandler::show_one_item("chip_register_page", item, {"page_id"}, {{"chip_id", chip_id_}, {"page_name", page_name}}))
    {
        QMessageBox::warning(this, windowTitle(), "Unable to validate register page name due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return false;
    }
    if (item.size() > 0)
    {
        QMessageBox::warning(this, windowTitle(), "Register page " + page_name + " already exists!");
        return false;
    }
    return true;
}

QSet<QString> EditRegisterPageDialog::get_available_control_signal_types() const
{
    QVector<QVector<QString> > items;
    DataBaseHandler::show_items("def_register_type", {"reg_type_id"}, "writable", "1", items);
    QSet<QString> writable_reg_types;
    for (const auto& item : items)
        writable_reg_types.insert(item[0]);

    items.clear();
    DataBaseHandler::show_items("def_sig_reg_type_mapping", {"sig_type_id", "reg_type_id"}, items);
    QSet<QString> sig_types;
    for (const auto& item : items)
        if (writable_reg_types.contains(item[1])) sig_types.insert(item[0]);
    return sig_types;
}

QSet<QString> EditRegisterPageDialog::get_available_paged_register_types() const
{
    QVector<QVector<QString> > items;
    DataBaseHandler::show_items("def_register_type", {"reg_type_id"}, {{"writable", "1"}, {"clear", "0"}}, items);
    QSet<QString> reg_types;
    for (const auto& item : items)
        reg_types.insert(item[0]);
    return reg_types;
}

bool EditRegisterPageDialog::add_register_page()
{
    if (DataBaseHandler::get_next_auto_increment_id("chip_register_page", "page_id", page_id_) &&
        DataBaseHandler::insert_item("chip_register_page", {"page_id", "page_name", "chip_id", "ctrl_sig", "page_count"},
        {page_id_, get_register_page_name(), chip_id_, get_control_signal_id(), get_page_count()}))
    {
        bool success = true;
        for (int i = 0; i < ui->tableWidgetSelected->rowCount(); i++)
        {
            QString reg_id = ui->tableWidgetSelected->item(i, 0)->text();
            success = success && DataBaseHandler::insert_item("chip_register_page_content", {"page_id", "reg_id"}, {page_id_, reg_id});
        }
        if (success)
        {
            DataBaseHandler::commit();
            return true;
        }
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Add Register Page", "Unable to add register page.\nError massage: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditRegisterPageDialog::edit_register_page()
{

    bool success = true;
    success = success && DataBaseHandler::delete_items("chip_register_page_content", "page_id", page_id_);
    success = success && DataBaseHandler::update_items("chip_register_page", "page_id", page_id_, {{"page_name", get_register_page_name()}, {"ctrl_sig", get_control_signal_id()}, {"page_count", get_page_count()}});
    for (int i = 0; i < ui->tableWidgetSelected->rowCount(); i++)
    {
        QString reg_id = ui->tableWidgetSelected->item(i, 0)->text();
        success = success && DataBaseHandler::insert_item("chip_register_page_content", {"page_id", "reg_id"}, {page_id_, reg_id});
    }
    if (success)
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Edit Register Page", "Unable to edit register page.\nError massage: " + DataBaseHandler::get_error_message());
    return false;
}
