#include "edit_register_page_dialog.h"
#include "ui_edit_register_page_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include "data_utils.h"
#include <QtMath>
#include <QCompleter>
#include <QMessageBox>
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QDebug>

EditRegisterPageDialog::EditRegisterPageDialog(const QString& chip_id, QWidget *parent) :
    QDialog(parent),
    chip_id_(chip_id),
    mode_(DIALOG_MODE::ADD),
    ui(new Ui::EditRegisterPageDialog)
{
    ui->setupUi(this);
    setup_ui();
    setWindowTitle("Add Register Page");
}


EditRegisterPageDialog::EditRegisterPageDialog(const QString& chip_id, const QString& page_id, QWidget *parent):
    QDialog(parent),
    chip_id_(chip_id),
    page_id_(page_id),
    mode_(DIALOG_MODE::EDIT),
    ui(new Ui::EditRegisterPageDialog)
{
    ui->setupUi(this);
    setup_ui();
    setWindowTitle("Edit Register Page");

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;

    QVector<QString> fields = {"chip_register_page.page_name", "chip_register_page.ctrl_sig", "chip_register_page.page_count",
                               "signal_signal.sig_name", "signal_signal.width", "block_system_block.block_name",
                               "block_system_block.abbreviation", "signal_signal.add_port"};
    dbhandler.show_items_inner_join(fields,
            {{{"chip_register_page", "ctrl_sig"}, {"signal_signal", "sig_id"}}, {{"signal_signal", "block_id"}, {"block_system_block", "block_id"}}}, items, {{"chip_register_page.page_id", page_id_}});
    if (items.size() == 0)
    {
        QMessageBox::critical(this, "Edit Register Page", "Unable to open register page!");
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->comboBoxControlSignal->count() > 0);
        ui->tableWidgetAll->setEnabled(false);
        ui->tableWidgetSelected->setEnabled(false);
        return;
    }

    QString page_name = items[0][0], control_sig_id = items[0][1], page_count = items[0][2];
    qDebug() << page_count;
    int width = items[0][4].toInt();
    QString block_name = items[0][5], block_abbr = items[0][6];
    SIGNAL_NAMING.update_key("{BLOCK_NAME}", block_name);
    SIGNAL_NAMING.update_key("{BLOCK_ABBR}", block_abbr);
    QString control_sig_name = items[0][7]=="1" ? SIGNAL_NAMING.get_extended_name(items[0][3]) : items[0][3];

    original_control_sig_id_ = control_sig_id;
    original_page_name_ = page_name;
    sig_name2id_[control_sig_name] = control_sig_id;
    sig_name2width_[control_sig_name] = width;

    ui->comboBoxControlSignal->addItem(control_sig_name);
    ui->comboBoxControlSignal->setCurrentIndex(ui->comboBoxControlSignal->count() - 1);

    ui->lineEditName->setText(page_name);
    ui->lineEditPageCount->setText(page_count);

    items.clear();
    dbhandler.show_items("chip_register_page_content", {"reg_id"}, "page_id", page_id_, items);
    QSet<QString> selected_regs;
    for (const auto& item : items)
        selected_regs.insert(item[0]);
    int row = 0;
    while (row < ui->tableWidgetAll->rowCount())
    {
        QString reg_id = ui->tableWidgetAll->item(row, 0)->text();
        if (selected_regs.contains(reg_id))
        {
            ui->tableWidgetSelected->insertRow(ui->tableWidgetSelected->rowCount());
            ui->tableWidgetSelected->setItem(ui->tableWidgetSelected->rowCount()-1, 0, new QTableWidgetItem(reg_id));
            ui->tableWidgetSelected->setItem(ui->tableWidgetSelected->rowCount()-1, 1, new QTableWidgetItem(ui->tableWidgetAll->item(row, 1)->text()));
            ui->tableWidgetAll->removeRow(row);
        }
        else row++;
    }
}

EditRegisterPageDialog::~EditRegisterPageDialog()
{
    delete ui;
}

void EditRegisterPageDialog::setup_ui()
{
    ui->tableWidgetAll->setColumnHidden(0, true);
    ui->tableWidgetSelected->setColumnHidden(0, true);

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_system_block", {"block_id", "block_name", "abbreviation", "prev", "next"}, "chip_id", chip_id_, items);
    items = sort_doubly_linked_list(items);
    for (const auto& item : items)
    {
        REGISTER_NAMING.update_key("{BLOCK_NAME}", item[1]);
        REGISTER_NAMING.update_key("{BLOCK_ABBR}", item[2]);
        SIGNAL_NAMING.update_key("{BLOCK_NAME}", item[1]);
        SIGNAL_NAMING.update_key("{BLOCK_ABBR}", item[2]);

        QVector<QVector<QString> > reg_items;
        dbhandler.show_items("block_register", {"reg_id", "reg_name", "prev", "next"}, "block_id", item[0], reg_items);
        reg_items = sort_doubly_linked_list(reg_items);
        for (const auto& reg_item : reg_items)
        {
            int row = ui->tableWidgetAll->rowCount();
            ui->tableWidgetAll->insertRow(row);
            ui->tableWidgetAll->setItem(row, 0, new QTableWidgetItem(reg_item[0]));
            ui->tableWidgetAll->setItem(row, 1, new QTableWidgetItem(REGISTER_NAMING.get_extended_name(reg_item[1])));
        }

        QVector<QVector<QString> > sig_items;
        dbhandler.show_items("signal_signal", {"sig_id", "sig_name", "width", "add_port"}, {{"block_id", item[0]}, {"sig_type_id", "1"}}, sig_items);
        for (const auto& sig_item : sig_items)
        {
            QString sig_id = sig_item[0], sig_name = sig_item[3]=="1" ? SIGNAL_NAMING.get_extended_name(sig_item[1]) : sig_item[1];
            int width = sig_item[2].toInt();
            QVector<QVector<QString> > sig_part_items;
            QVector<QString> page_item;
            dbhandler.show_items_inner_join({"block_sig_reg_partition_mapping.sig_reg_part_mapping_id"},
                                            {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}}, {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}},
                                            sig_part_items, {{"signal_signal.sig_id", sig_id}});
            dbhandler.show_one_item("chip_register_page", page_item, {"page_id"}, "ctrl_sig", sig_id);
            if (sig_part_items.size() == 0 && page_item.size() == 0)
            {
                sig_name2id_[sig_name] = sig_id;
                sig_name2width_[sig_name] = width;
                ui->comboBoxControlSignal->addItem(sig_name);
            }
        }
        //QCompleter *completer = new QCompleter(sig_name2id_.keys(), this);
        //completer->setCaseSensitivity(Qt::CaseInsensitive);
        //ui->comboBoxControlSignal->setCompleter(completer);
    }

    ui->pushButton->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->comboBoxControlSignal->count() > 0);
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

void EditRegisterPageDialog::on_lineEditSearchRegister_editingFinished()
{
    QString pattern = ui->lineEditSearchRegister->text();
    for (int i = 0; i < ui->tableWidgetAll->rowCount(); i++)
    {
        ui->tableWidgetAll->setRowHidden(i, !ui->tableWidgetAll->item(i, 1)->text().contains(pattern));
        if (ui->tableWidgetAll->currentRow() == i)
        {
            ui->tableWidgetAll->setCurrentCell(-1, -1);
            ui->pushButton->setEnabled(false);
        }
    }
}

void EditRegisterPageDialog::on_lineEditSearchRegister_textChanged(const QString &pattern)
{
    for (int i = 0; i < ui->tableWidgetAll->rowCount(); i++)
    {
        ui->tableWidgetAll->setRowHidden(i, !ui->tableWidgetAll->item(i, 1)->text().contains(pattern));
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
        if (sig_name.contains(pattern)) ui->comboBoxControlSignal->addItem(sig_name);
    }
}

void EditRegisterPageDialog::accept()
{
    if (sanity_check()) QDialog::accept();
}

bool EditRegisterPageDialog::sanity_check()
{
    if (!check_name()) return false;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > sig_part_items;
    QString sig_id = sig_name2id_[ui->comboBoxControlSignal->currentText()];
    dbhandler.show_items_inner_join({"block_sig_reg_partition_mapping.sig_reg_part_mapping_id"},
                                    {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}}, {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}},
                                    sig_part_items, {{"signal_signal.sig_id", sig_id}});
    if (sig_part_items.size() > 0) return false;
    int width = sig_name2width_[ui->comboBoxControlSignal->currentText()];
    if (qRound(qPow(2, width)) < ui->lineEditPageCount->text().toInt())
    {
        QMessageBox::warning(this, "Create Register Page", "Register page count should range between 1 and " + QString::number(qRound(qPow(2, width))) + "!");
        return false;
    }
    return true;
}

bool EditRegisterPageDialog::check_name()
{
    QString title = mode_ == ADD ? "Create Register Page" : "Edit Register Page";
    QString page_name = ui->lineEditName->text();
    if (page_name == "")
    {
        QMessageBox::warning(this, title, "Register page name should not be empty!");
        return false;
    }
    if (mode_ == EDIT && get_register_page_name() == original_page_name_) return true;
    QVector<QString> item;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.show_one_item("chip_register_page", item, {"page_id"}, {{"chip_id", chip_id_}, {"page_name", page_name}});
    if (item.size() > 0)
    {
        QMessageBox::warning(this, title, "Register page " + page_name + " already exists!");
        return false;
    }
    return true;
}

void EditRegisterPageDialog::on_comboBoxControlSignal_currentIndexChanged(int index)
{
    int width = sig_name2width_[ui->comboBoxControlSignal->currentText()];
    ui->lineEditPageCount->setValidator(new QIntValidator(1, qRound(qPow(2, width)), this));
    ui->lineEditPageCount->setText(QString::number(qRound(qPow(2, width))));
}

bool EditRegisterPageDialog::add_register_page()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.insert_item("chip_register_page", {"page_name", "chip_id", "ctrl_sig", "page_count"}, {get_register_page_name(), chip_id_, get_control_signal_id(), get_page_count()}))
    {
        QVector<QString> item;
        dbhandler.show_one_item("chip_register_page", item, {"page_id"}, {{"page_name", get_register_page_name()}, {"chip_id", chip_id_}, {"ctrl_sig", get_control_signal_id()}});
        page_id_ = item[0];
        for (int i = 0; i < ui->tableWidgetSelected->rowCount(); i++)
        {
            QString reg_id = ui->tableWidgetSelected->item(i, 0)->text();
            dbhandler.insert_item("chip_register_page_content", {"page_id", "reg_id"}, {page_id_, reg_id});
        }
        return true;
    }
    else {
        QMessageBox::warning(this, "Create Register Page", "Unable to add register page!\nError massage: " + dbhandler.get_error_message());
        return false;
    }
}

bool EditRegisterPageDialog::edit_register_page()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.delete_items("chip_register_page_content", "page_id", page_id_);
    if (!dbhandler.update_items("chip_register_page", "page_id", page_id_, {{"page_name", get_register_page_name()}, {"ctrl_sig", get_control_signal_id()}, {"page_count", get_page_count()}}))
    {
        QMessageBox::warning(this, "Edit Register Page", "Unable to add edit page!\nError massage: " + dbhandler.get_error_message());
        return false;
    }
    for (int i = 0; i < ui->tableWidgetSelected->rowCount(); i++)
    {
        QString reg_id = ui->tableWidgetSelected->item(i, 0)->text();
        dbhandler.insert_item("chip_register_page_content", {"page_id", "reg_id"}, {page_id_, reg_id});
    }
    return true;
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
