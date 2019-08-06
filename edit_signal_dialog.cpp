#include "edit_signal_dialog.h"
#include "ui_edit_signal_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include "data_utils.h"
#include "edit_signal_partition_dialog.h"
#include "edit_register_dialog.h"
#include <QMessageBox>
#include <QtMath>
#include <QRegExpValidator>
#include <QDebug>


EditSignalDialog::EditSignalDialog(const QString& chip_id, const QString& block_id, int register_width, bool msb_first, QWidget *parent) :
    QDialog(parent),
    EditSignalPartitionLogic(register_width, msb_first),
    ui(new Ui::EditSignalDialog),
    chip_id_(chip_id),
    block_id_(block_id),
    mode_(DIALOG_MODE::ADD),
    enabled_(true)

{
    bool success = setup_ui();
    setWindowTitle("Add Signal");
    if (!success)
        QMessageBox::warning(this, windowTitle(), "Unable to initialize signal editor due to database connection issue.\nPlease try again!");
}

EditSignalDialog::EditSignalDialog(const QString& chip_id, const QString& block_id, const QString& sig_id, const QString& reg_sig_id, int register_width, bool msb_first, bool enabled, QWidget *parent) :
    QDialog(parent),
    EditSignalPartitionLogic(register_width, msb_first),
    ui(new Ui::EditSignalDialog),
    chip_id_(chip_id),
    block_id_(block_id),
    signal_id_(sig_id),
    reg_sig_id_(reg_sig_id),
    mode_(DIALOG_MODE::EDIT),
    enabled_(enabled)
{
    bool success = setup_ui();
    setWindowTitle("Edit Signal");

    QVector<QVector<QString> > items;
    success = success && DataBaseHandler::show_items("signal_signal", {"sig_name", "width", "sig_type_id", "add_port"}, "sig_id", get_signal_id(), items);
    assert (items.size() == 1);

    ui->checkBoxAddPort->setChecked(items[0][3] == "1");
    on_checkBoxAddPort_clicked(ui->checkBoxAddPort->isChecked());
    QString sig_name = ui->checkBoxAddPort->isChecked() ? GLOBAL_SIGNAL_NAMING.get_extended_name(items[0][0]) : items[0][0];
    ui->lineEditSigName->setText(sig_name);
    ui->lineEditWidth->setText(items[0][1]);
    emit(ui->lineEditWidth->editingFinished());

    original_signal_given_name_ = items[0][0];
    original_width_ = items[0][1];
    original_sig_type_id_ = items[0][2];

    for (int i = 0; i < ui->comboBoxSigType->count(); i++)
    {
        QString sig_type = ui->comboBoxSigType->itemText(i);
        if (sig_type2id_[sig_type] == original_sig_type_id_)
        {
            ui->comboBoxSigType->setCurrentIndex(i);
            break;
        }
    }

    if (is_register_signal())
    {
        items.clear();
        success = success && DataBaseHandler::show_items("signal_reg_signal", {"sig_id", "init_value", "reg_type_id"}, "reg_sig_id", get_reg_sig_id(), items);
        assert (items.size() == 1 && items[0][0] == get_signal_id());
        ui->lineEditValue->setText(items[0][1]);
        original_value_ = items[0][1];
        original_reg_type_id_ = items[0][2];
        for (int i = 0; i < ui->comboBoxRegType->count(); i++)
        {
            QString reg_type = ui->comboBoxRegType->itemText(i);
            if (reg_type2id_[reg_type] == original_reg_type_id_)
            {
                ui->comboBoxRegType->setCurrentIndex(i);
                break;
            }
        }

        ui->tableWidgetSigPart->setRowCount(0);
        QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                       "block_register.reg_name",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb" };

        items.clear();
        success = success && DataBaseHandler::show_items_inner_join(extended_fields, {{{"block_sig_reg_partition_mapping", "reg_id"}, {"block_register", "reg_id"}}}, items, "block_sig_reg_partition_mapping.reg_sig_id = \"" + reg_sig_id + "\"");
        if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[3].toInt() > b[3].toInt();});
        else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() < b[2].toInt();});
        for (const auto& item : items)
        {
            int row = ui->tableWidgetSigPart->rowCount();
            ui->tableWidgetSigPart->insertRow(row);
            QString sig_reg_part_mapping_id = item[0], sig_lsb = item[1], sig_msb = item[2], reg_name = item[3], reg_lsb = item[4], reg_msb = item[5];
            reg_name = GLOBAL_REGISTER_NAMING.get_extended_name(reg_name);
            QString sig_part, reg_part;
            sig_part = msb_first_ ? "<" + sig_msb + ":"+ sig_lsb + ">" : "<" + sig_lsb + ":"+ sig_msb + ">";
            reg_part = msb_first_ ? reg_name + "<" + reg_msb + ":"+ reg_lsb +">" : reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
            ui->tableWidgetSigPart->setItem(row, 0, new QTableWidgetItem(sig_part));
            ui->tableWidgetSigPart->setItem(row, 1, new QTableWidgetItem(reg_part));
            original_sig_reg_mapping_part_ids_.push_back(sig_reg_part_mapping_id);
            occupied_signal_parts_.push_back({sig_lsb.toInt(), sig_msb.toInt()});
            make_occupied_register_parts(reg_name2id_[reg_name]);
        }

        emit(ui->lineEditWidth->editingFinished());

        if (get_width().toInt() == 1 && ui->tableWidgetSigPart->rowCount() > 0)
        {
            ui->tableWidgetSigPart->setCurrentCell(0, 0);
            QString reg_name = ui->tableWidgetSigPart->item(0, 1)->text().split("<")[0];
            QString reg_lsb = ui->tableWidgetSigPart->item(0, 1)->text().replace(">", "").split(":")[1];
            on_pushButtonRemoveSigPart_clicked();
            for (int i = 0; i < ui->comboBoxReg->count(); i++)
            {
                if (ui->comboBoxReg->itemText(i) == reg_name)
                {
                    ui->comboBoxReg->setCurrentIndex(i);
                    break;
                }
            }
            for (int i = 0; i < ui->comboBoxRegPart->count(); i++)
            {
                if (ui->comboBoxRegPart->itemText(i).replace("<", "").replace(">", "").split(":")[0] == reg_lsb)
                {
                    ui->comboBoxRegPart->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    if (!success)
        QMessageBox::warning(this, windowTitle(), "Unable to initialize due to database connection issue.\nPlease try again!");
}

bool EditSignalDialog::setup_ui()
{
    ui->setupUi(this);
    if (ui->checkBoxAddPort->isChecked())
    {
        ui->lineEditSigName->setValidator(new QRegExpValidator(QRegExp(GLOBAL_SIGNAL_NAMING.get_extended_name("_?[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*_?"))));
        ui->lineEditSigName->setText(GLOBAL_SIGNAL_NAMING.get_extended_name(""));
        ui->lineEditSigName->setCursorPosition(GLOBAL_SIGNAL_NAMING.get_extended_name("***").indexOf("***"));
    }

    comboBoxSigLSB_ = msb_first_ ? ui->comboBoxSigRight : ui->comboBoxSigLeft;
    comboBoxSigMSB_ = msb_first_ ? ui->comboBoxSigLeft : ui->comboBoxSigRight;
    connect(comboBoxSigLSB_, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxSigLSB_currentIndexChanged(int)));
    connect(comboBoxSigMSB_, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxSigMSB_currentIndexChanged(int)));

    QVector<QVector<QString> > items;
    bool success = true;
    success = success && DataBaseHandler::show_items("def_signal_type", {"sig_type_id", "sig_type"}, items, "", "order by sig_type_id");
    for (const auto& item : items)
    {
        QString sig_type = item[1], id = item[0];
        sig_type2id_[sig_type] = id;
        sig_type_id2type_[id] = sig_type;
        ui->comboBoxSigType->addItem(sig_type);
    }


    if (mode_ == DIALOG_MODE::EDIT)
    {
        items.clear();
        success = success && DataBaseHandler::show_items("chip_register_page", {"page_id"}, "ctrl_sig", signal_id_, items);
        for (const auto& item : items)
        {
            QString page_id = item[0];
            QVector<QVector<QString> > regs;
            success = success && DataBaseHandler::show_items("chip_register_page_content", {"reg_id"}, "page_id", page_id, regs);
            for (const auto& reg : regs)
                registers_in_page_.insert(reg[0]);
        }
    }

    items.clear();
    success = success && DataBaseHandler::show_items("def_register_type", {"reg_type_id", "reg_type", "writable"}, items, "", "order by reg_type_id");
    for (const auto& item : items)
    {
        QString reg_type = item[1], id = item[0];
        reg_type2id_[reg_type] = id;
        reg_type_id2type_[id] = reg_type;
        ui->comboBoxRegType->addItem(reg_type);
        if (item[2] == "1") writable_reg_types_.insert(reg_type);
    }

    items.clear();
    success = success && DataBaseHandler::show_items("def_sig_reg_type_mapping", {"sig_type_id", "reg_type_id"}, items, "", "order by mapping_id");
    for (const auto& item: items)
    {
        QString sig_type = sig_type_id2type_[item[0]];
        QString reg_type = reg_type_id2type_[item[1]];
        if (sig_type2reg_types_.find(sig_type) == sig_type2reg_types_.end())
            sig_type2reg_types_[sig_type] = QVector<QString>();
        sig_type2reg_types_[sig_type].push_back(reg_type);
    }

    ui->lineEditWidth->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*")));
    ui->lineEditWidth->setText("1");
    if (is_register_signal() && ui->lineEditValue->text() == "") ui->lineEditValue->setText("0x");

    comboBoxSigLSB_ = msb_first_ ? ui->comboBoxSigRight : ui->comboBoxSigLeft;
    comboBoxSigMSB_ = msb_first_ ? ui->comboBoxSigLeft : ui->comboBoxSigRight;

    emit(ui->lineEditWidth->editingFinished());
    emit(ui->comboBoxSigType->currentIndexChanged(ui->comboBoxSigType->currentIndex()));
    emit(ui->tableWidgetSigPart->currentCellChanged(-1, -1, -1, -1));

    QVector<QWidget*> widgets = {ui->lineEditSigName, ui->lineEditWidth, ui->comboBoxSigType, ui->lineEditValue,
                                ui->comboBoxRegType, ui->comboBoxReg, ui->comboBoxSigLeft, ui->comboBoxSigRight, ui->checkBoxAddPort, ui->pushButtonAddReg,
                                ui->comboBoxRegPart, ui->pushButtonAddSigPart, ui->tableWidgetSigPart};
    for (QWidget* w : widgets) w->setEnabled(enabled_);
    ui->pushButtonRemoveSigPart->setEnabled(enabled_ && ui->tableWidgetSigPart->currentRow() >= 0);
    ui->tableWidgetSigPart->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    return success;
}

EditSignalDialog::~EditSignalDialog()
{
    delete ui;
}

QString EditSignalDialog::get_signal_name() const
{
    return ui->lineEditSigName->text();
}

QString EditSignalDialog::get_signal_id() const
{
    return signal_id_;
}

QString EditSignalDialog::get_reg_sig_id() const
{
    return reg_sig_id_;
}

QString EditSignalDialog::get_width() const
{
    return ui->lineEditWidth->text();
}

QString EditSignalDialog::get_value() const
{
    return normalize_hex(ui->lineEditValue->text(), get_width().toInt());
}

QString EditSignalDialog::get_signal_type() const
{
    return ui->comboBoxSigType->currentText();
}

QString EditSignalDialog::get_signal_type_id() const
{
    if (sig_type2id_.find(get_signal_type()) != sig_type2id_.end()) return sig_type2id_[get_signal_type()];
    return "";
}

QString EditSignalDialog::get_register_type() const
{
    return ui->comboBoxRegType->currentText();
}

QString EditSignalDialog::get_register_type_id() const
{
    if (is_register_signal()) return reg_type2id_[get_register_type()];
    else return "";
}

bool EditSignalDialog::is_register_signal() const
{
    return sig_type2reg_types_.find(get_signal_type()) != sig_type2reg_types_.end();
}

bool EditSignalDialog::add_port() const
{
    return ui->checkBoxAddPort->isChecked();
}

int EditSignalDialog::get_current_signal_lsb() const
{
    if (comboBoxSigLSB_->currentIndex() >= 0) return comboBoxSigLSB_->currentText().toInt();
    else return -1;
}

int EditSignalDialog::get_current_signal_msb() const
{
    if (comboBoxSigMSB_->currentIndex() >= 0) return comboBoxSigMSB_->currentText().toInt();
    else return -1;
}

void EditSignalDialog::make_occupied_signal_parts()
{
    EditSignalPartitionLogic::make_occupied_signal_parts();
    for (int row = 0; row < ui->tableWidgetSigPart->rowCount(); row++)
    {
        QString part = ui->tableWidgetSigPart->item(row, 0)->text().replace("<", "").replace(">", "");
        int lsb, msb;
        if (msb_first_)
        {
            msb = part.split(":")[0].toInt();
            lsb = part.split(":")[1].toInt();
        }
        else {
            lsb = part.split(":")[0].toInt();
            msb = part.split(":")[1].toInt();
        }
        occupied_signal_parts_.push_back({lsb, msb});
    }
}


void EditSignalDialog::display_available_register_parts()
{
    const partition_list& parts = get_available_register_parts_by_length(get_partition_length());
    ui->comboBoxRegPart->clear();
    for (const auto& segment : parts)
    {
        if (msb_first_)
            ui->comboBoxRegPart->insertItem(0, "<" + QString::number(segment.second) + ":" + QString::number(segment.first) + ">");
        else
            ui->comboBoxRegPart->addItem("<" + QString::number(segment.first) + ":" + QString::number(segment.second) + ">");
    }
    if (ui->comboBoxRegPart->count() > 0 && msb_first_) ui->comboBoxRegPart->setCurrentIndex(0);
    ui->comboBoxReg->setEnabled(enabled_ && (comboBoxSigLSB_->count() > 0 || comboBoxSigMSB_->count() > 0));
}

void EditSignalDialog::on_lineEditWidth_editingFinished()
{
    on_lineEditWidth_textChanged(ui->lineEditWidth->text());
    if (ui->lineEditWidth->text() != "") ui->lineEditWidth->clearFocus();
}

void EditSignalDialog::on_lineEditWidth_textChanged(const QString &arg1)
{
    ui->lineEditValue->setValidator(new HexValueValidator(get_width().toInt()));
    if (!is_register_signal()) {
        last_width_ = get_width().toInt();
        ui->lineEditValue->setText("");
        return;
    }
    if (get_width().toInt() != last_width_)
    {
        int row_count = ui->tableWidgetSigPart->rowCount();
        while (row_count > 0)
        {
            ui->tableWidgetSigPart->setCurrentCell(0, 0);
            on_pushButtonRemoveSigPart_clicked();
            row_count = ui->tableWidgetSigPart->rowCount();
        }
        ui->lineEditValue->setText("0x");
    }
    last_width_ = get_width().toInt();
    make_available_signal_parts(get_width().toInt());
    if (!msb_first_)
    {
        make_available_signal_starts();
        comboBoxSigLSB_->blockSignals(true);
        comboBoxSigLSB_->clear();
        for (int i : available_signal_starts_) comboBoxSigLSB_->addItem(QString::number(i));
        comboBoxSigLSB_->setCurrentIndex(comboBoxSigLSB_->count() > 0 ? 0: -1);
        comboBoxSigLSB_->blockSignals(false);
        emit(comboBoxSigLSB_->currentIndexChanged(comboBoxSigLSB_->currentIndex()));

    }
    else {
        make_available_signal_ends();
        comboBoxSigMSB_->blockSignals(true);
        comboBoxSigMSB_->clear();
        for (int i : available_signal_ends_) comboBoxSigMSB_->addItem(QString::number(i));
        comboBoxSigMSB_->setCurrentIndex((comboBoxSigMSB_->count()-1));
        comboBoxSigMSB_->blockSignals(false);
        emit(comboBoxSigMSB_->currentIndexChanged(comboBoxSigMSB_->currentIndex()));
    }

    int w = width();
    comboBoxSigLSB_->setVisible(get_width().toInt() != 1);
    comboBoxSigMSB_->setVisible(get_width().toInt() != 1);
    ui->tableWidgetSigPart->setVisible(get_width().toInt() != 1);
    ui->pushButtonAddSigPart->setVisible(get_width().toInt() != 1);
    ui->pushButtonRemoveSigPart->setVisible(get_width().toInt() != 1);

    if (get_width().toInt() == 1) resize(w, 290);
    else resize(w, 560);
}

void EditSignalDialog::on_comboBoxSigType_currentIndexChanged(int index)
{
    ui->comboBoxSigType->setEnabled(enabled_ && index>=0);
    ui->comboBoxRegType->blockSignals(true);
    ui->comboBoxRegType->clear();
    if (is_register_signal())
    {
        for (const QString & reg_type : sig_type2reg_types_[get_signal_type()])
            ui->comboBoxRegType->addItem(reg_type);
    }
    ui->comboBoxRegType->blockSignals(false);
    emit(ui->comboBoxRegType->currentIndexChanged(ui->comboBoxRegType->currentIndex()));
}

void EditSignalDialog::on_comboBoxRegType_currentIndexChanged(int index)
{
    ui->comboBoxRegType->setEnabled(enabled_ && index>=0);
    ui->lineEditValue->setEnabled(enabled_ && index>=0 && writable_reg_types_.contains(ui->comboBoxRegType->currentText()));
    if (!writable_reg_types_.contains(ui->comboBoxRegType->currentText())) ui->lineEditValue->setText("");
    if (writable_reg_types_.contains(ui->comboBoxRegType->currentText()) && ui->lineEditValue->text() == "") ui->lineEditValue->setText("0x");

    int row_count = ui->tableWidgetSigPart->rowCount();
    while (row_count > 0)
    {
        ui->tableWidgetSigPart->setCurrentCell(0, 0);
        emit(ui->pushButtonRemoveSigPart->clicked());
        row_count = ui->tableWidgetSigPart->rowCount();
    }

    if (index < 0)
    {
        ui->comboBoxReg->clear();
        return;
    }
    ui->comboBoxReg->blockSignals(true);
    ui->comboBoxReg->clear();
    if (index >= 0)
    {

        QVector<QVector<QString> > items;

        if (!DataBaseHandler::show_items("block_register", {"reg_name", "reg_id"}, {{"block_id", block_id_}, {"reg_type_id", get_register_type_id()}}, items))
            QMessageBox::warning(this, windowTitle(), "Unable to read register types from database.\nPlease try again.!");
        for (const auto& item : items)
        {
            if (registers_in_page_.contains(item[1])) continue;
            reg_name2id_[GLOBAL_REGISTER_NAMING.get_extended_name(item[0])] = item[1];
            ui->comboBoxReg->addItem(GLOBAL_REGISTER_NAMING.get_extended_name(item[0]));
        }
    }
    ui->comboBoxReg->blockSignals(false);
    emit(ui->comboBoxReg->currentIndexChanged(ui->comboBoxReg->currentIndex()));
}

void EditSignalDialog::on_comboBoxReg_currentIndexChanged(int index)
{
    ui->comboBoxReg->setEnabled(enabled_ && index>=0);
    ui->pushButtonAddSigPart->setEnabled(enabled_ && index >= 0);
    comboBoxSigLSB_->setEnabled(enabled_ && index>=0);
    comboBoxSigMSB_->setEnabled(enabled_ && index>=0);
    ui->comboBoxRegPart->setEnabled(enabled_ && index>=0);
    if (index < 0)
    {
        comboBoxSigLSB_->clear();
        comboBoxSigMSB_->clear();
        ui->comboBoxRegPart->clear();
        if (index < 0) return;
    }

    make_occupied_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts_by_length();
    emit(ui->lineEditWidth->editingFinished());
}


void EditSignalDialog::on_comboBoxSigLSB_currentIndexChanged(int index)
{
    ui->pushButtonAddSigPart->setEnabled(enabled_ && index != -1);
    comboBoxSigLSB_->setEnabled(enabled_ && index != -1);
    //comboBoxSigMSB_->setEnabled(enabled_ && index != -1);
    ui->comboBoxRegPart->setEnabled(enabled_ && index != -1);
    if (msb_first_)
    {
        display_available_register_parts();
        return;
    }
    comboBoxSigMSB_->blockSignals(true);
    comboBoxSigMSB_->clear();
    make_available_signal_ends();
    for (int i : available_signal_ends_) comboBoxSigMSB_->addItem(QString::number(i));
    comboBoxSigMSB_->setCurrentIndex(comboBoxSigMSB_->count()-1);
    comboBoxSigMSB_->blockSignals(false);
    emit(comboBoxSigMSB_->currentIndexChanged(comboBoxSigMSB_->currentIndex()));
}

void EditSignalDialog::on_comboBoxSigMSB_currentIndexChanged(int index)
{
    ui->pushButtonAddSigPart->setEnabled(enabled_ && index != -1);
    //comboBoxSigLSB_->setEnabled(enabled_ && index != -1);
    comboBoxSigMSB_->setEnabled(enabled_ && index != -1);
    ui->comboBoxRegPart->setEnabled(enabled_ && index != -1);
    if (!msb_first_)
    {
        display_available_register_parts();
        return;
    }
    comboBoxSigLSB_->blockSignals(true);
    comboBoxSigLSB_->clear();
    make_available_signal_starts();
    for (int i : available_signal_starts_) comboBoxSigLSB_->addItem(QString::number(i));
    comboBoxSigLSB_->setCurrentIndex(comboBoxSigLSB_->count() > 0 ? 0 : -1);
    comboBoxSigLSB_->blockSignals(false);
    emit(comboBoxSigLSB_->currentIndexChanged(comboBoxSigLSB_->currentIndex()));
}


void EditSignalDialog::on_pushButtonAddReg_clicked()
{
    EditRegisterDialog new_reg(chip_id_, block_id_);
    if (new_reg.exec() == QDialog::Accepted)
    {
        new_reg.add_register();
        QString reg_name = new_reg.get_reg_name();
        QString reg_id = new_reg.get_reg_id();
        reg_name2id_[reg_name] = reg_id;
        if (new_reg.get_reg_type() != get_register_type()) return;
        ui->comboBoxReg->addItem(reg_name);
        ui->comboBoxReg->setCurrentIndex(ui->comboBoxReg->count()-1);
    }
}

void EditSignalDialog::on_pushButtonAddSigPart_clicked()
{
    QString sig_lsb = comboBoxSigLSB_->currentText();
    QString sig_msb = comboBoxSigMSB_->currentText();
    QString reg_part = ui->comboBoxRegPart->currentText().replace("<", "").replace(">", "");
    QString reg_lsb, reg_msb;
    QString reg_name = ui->comboBoxReg->currentText();
    if (msb_first_)
    {
        reg_lsb = reg_part.split(":")[1];
        reg_msb = reg_part.split(":")[0];
    }
    else {
        reg_lsb = reg_part.split(":")[0];
        reg_msb = reg_part.split(":")[1];

    }
    int row = ui->tableWidgetSigPart->rowCount();
    ui->tableWidgetSigPart->insertRow(row);
    if (msb_first_)
    {
        ui->tableWidgetSigPart->setItem(row, 0, new QTableWidgetItem("<" + sig_msb + ":" + sig_lsb + ">"));
        ui->tableWidgetSigPart->setItem(row, 1, new QTableWidgetItem(reg_name + "<" + reg_msb + ":" + reg_lsb + ">"));
    }
    else {
        ui->tableWidgetSigPart->setItem(row, 0, new QTableWidgetItem("<" + sig_lsb + ":" + sig_msb + ">"));
        ui->tableWidgetSigPart->setItem(row, 1, new QTableWidgetItem(reg_name + "<" + reg_lsb + ":" + reg_msb + ">"));
    }
    occupied_signal_parts_.push_back({sig_lsb.toInt(), sig_msb.toInt()});
    QString reg_id = reg_name2id_[ui->comboBoxReg->currentText()];
    reg_id2occupied_register_parts_[reg_id].push_back({reg_lsb.toInt(), reg_msb.toInt()});
    make_available_register_parts(reg_id);
    make_available_register_parts_by_length();
    emit(ui->lineEditWidth->editingFinished());
    //emit(ui->comboBoxSigType->currentIndexChanged(ui->comboBoxSigType->currentIndex()));
}

void EditSignalDialog::on_pushButtonRemoveSigPart_clicked()
{
    int row = ui->tableWidgetSigPart->currentRow();
    if (row == -1) return;
    int reg_lsb, reg_msb, sig_lsb, sig_msb;
    QString sig_part = ui->tableWidgetSigPart->item(row, 0)->text().replace("<", "").replace(">", "");
    QString reg_name = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[0];
    QString reg_part = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[1].replace(">", "");
    if (msb_first_)
    {
        sig_lsb = sig_part.split(":")[1].toInt();
        sig_msb = sig_part.split(":")[0].toInt();
        reg_lsb = reg_part.split(":")[1].toInt();
        reg_msb = reg_part.split(":")[0].toInt();
    }
    else {
        sig_lsb = sig_part.split(":")[0].toInt();
        sig_msb = sig_part.split(":")[1].toInt();
        reg_lsb = reg_part.split(":")[0].toInt();
        reg_msb = reg_part.split(":")[1].toInt();
    }
    ui->tableWidgetSigPart->removeRow(row);
    occupied_signal_parts_.removeAll({sig_lsb, sig_msb});
    QString reg_id = reg_name2id_[reg_name];
    reg_id2occupied_register_parts_[reg_id].removeAll({reg_lsb, reg_msb});
    make_available_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts_by_length();
    emit(ui->lineEditWidth->editingFinished());
}

void EditSignalDialog::on_tableWidgetSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveSigPart->setEnabled(enabled_ && currentRow >= 0);
}

void EditSignalDialog::on_checkBoxAddPort_clicked(bool checked)
{
    if (checked)
    {
        ui->lineEditSigName->setValidator(new QRegExpValidator(QRegExp(GLOBAL_SIGNAL_NAMING.get_extended_name("_?[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*_?"))));
        ui->lineEditSigName->setText(GLOBAL_SIGNAL_NAMING.get_extended_name(""));
        ui->lineEditSigName->setCursorPosition(GLOBAL_SIGNAL_NAMING.get_extended_name("***").indexOf("***"));
    }
    else
    {
        delete ui->lineEditSigName->validator();
        ui->lineEditSigName->setText("");
    }
}

void EditSignalDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) return QDialog::accept();
}

bool EditSignalDialog::sanity_check()
{
    if (!check_name()) return false;
    if (is_register_signal()) return check_value() && check_partitions();
    return true;
}

bool EditSignalDialog::check_name()
{
    if (ui->checkBoxAddPort->isChecked())
    {
        QRegularExpression re(GLOBAL_SIGNAL_NAMING.get_extended_name("[a-zA-Z0-9]+(_[a-zA-Z0-9]+)*"));
        QRegularExpressionMatch match = re.match(get_signal_name());
        if (!match.hasMatch())
        {
            QMessageBox::warning(this, windowTitle(), "Signal name must match "+  GLOBAL_SIGNAL_NAMING.get_extended_name("${NAME}") + " format!");
            return false;
        }
    }
    if (mode_ == EDIT )
    {
        QString sig_given_name = ui->checkBoxAddPort->isChecked() ? GLOBAL_SIGNAL_NAMING.get_given_name(get_signal_name()) : get_signal_name();
        if (sig_given_name == original_signal_given_name_) return true;
    }

    QVector<QVector<QString> > items;
    QString sig_given_name = ui->checkBoxAddPort->isChecked() ? GLOBAL_SIGNAL_NAMING.get_given_name(get_signal_name()) : get_signal_name();
    if (!DataBaseHandler::show_items("signal_signal", {"sig_name"}, {{"block_id", block_id_}, {"sig_name", sig_given_name}}, items))
        QMessageBox::warning(this, "Signal Editor", "Unable to validate signal name due to database access issue.\nPlease try agai.!");
    if (items.size() > 0)
    {
        QMessageBox::warning(this, windowTitle(), "Signal " + get_signal_name() + " already exists!");
        return false;
    }
    // anything else?
    return true;
}

bool EditSignalDialog::check_value()
{
    // temporary solution
    if (get_value() == "")
    {
        QMessageBox::warning(this, windowTitle(), "Value must not be empty!");
        return false;
    }
    int width = get_width().toInt();
    quint64 value = get_value().toULongLong(nullptr, 16);
    quint64 max_value = static_cast<quint64>(qPow(2, width) + 0.5);
    if (value >= max_value)
    {
        QMessageBox::warning(this, windowTitle(), "Signal value is too large!");
        return false;
    }
    return true;
}

bool EditSignalDialog::check_partitions()
{
    // TODO
    return true;
}

bool EditSignalDialog::add_signal()
{
    QString sig_given_name = ui->checkBoxAddPort->isChecked() ? GLOBAL_SIGNAL_NAMING.get_given_name(get_signal_name()) : get_signal_name();

    bool success = false;
    if (DataBaseHandler::get_next_auto_increment_id("signal_signal", "sig_id", signal_id_) &&
            DataBaseHandler::insert_item("signal_signal", {"sig_id", "sig_name", "block_id", "width", "sig_type_id", "add_port"},
                                         {signal_id_, sig_given_name, block_id_, get_width(), get_signal_type_id(), add_port() ? "1" : "0"}) )
    {
        success = true;
        if (is_register_signal())
        {
            if (DataBaseHandler::get_next_auto_increment_id("signal_reg_signal", "reg_sig_id", reg_sig_id_) &&
                DataBaseHandler::insert_item("signal_reg_signal", {"sig_id", "init_value", "reg_type_id"},
                                    {get_signal_id(), get_value(), get_register_type_id()}))
            {
                if (get_width().toInt() == 1 && comboBoxSigLSB_->count() > 0 \
                        && comboBoxSigMSB_->count() > 0 && ui->comboBoxReg->count() > 0 \
                        && ui->comboBoxRegPart->count() >0 && ui->pushButtonAddSigPart->isEnabled())
                {
                    on_pushButtonAddSigPart_clicked();
                }

                for (int row = 0; row < ui->tableWidgetSigPart->rowCount(); row++)
                {
                    QString reg_lsb, reg_msb, sig_lsb, sig_msb;
                    QString sig_part = ui->tableWidgetSigPart->item(row, 0)->text().replace("<", "").replace(">", "");
                    QString reg_name = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[0];
                    QString reg_part = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[1].replace(">", "");
                    if (msb_first_)
                    {
                        sig_lsb = sig_part.split(":")[1];
                        sig_msb = sig_part.split(":")[0];
                        reg_lsb = reg_part.split(":")[1];
                        reg_msb = reg_part.split(":")[0];
                    }
                    else {
                        sig_lsb = sig_part.split(":")[0];
                        sig_msb = sig_part.split(":")[1];
                        reg_lsb = reg_part.split(":")[0];
                        reg_msb = reg_part.split(":")[1];
                    }
                    success = success && add_signal_partition(sig_lsb, sig_msb, reg_lsb, reg_msb, reg_name2id_[reg_name], reg_sig_id_);
                }
            }
            else success = false;
        }
    }
    if (success)
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, windowTitle(), "Unable to add signal.\nError massage: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditSignalDialog::edit_signal()
{
    QString sig_given_name = ui->checkBoxAddPort->isChecked() ? GLOBAL_SIGNAL_NAMING.get_given_name(get_signal_name()) : get_signal_name();
    if (!DataBaseHandler::update_items("signal_signal", "sig_id", get_signal_id(),
                                {{"sig_name", sig_given_name},
                                {"width", get_width()}, {"sig_type_id", get_signal_type_id()}, {"add_port", add_port() ? "1" : "0"}}))
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, windowTitle(), "Unable to edit signal.\nError massage: " + DataBaseHandler::get_error_message());
        return false;
    }

    bool originally_register_signal = (original_reg_type_id_ != "");
    bool success = true;

    if (originally_register_signal)
    {
        for (const QString& sig_reg_part_mapping_id : original_sig_reg_mapping_part_ids_)
            success = success && DataBaseHandler::delete_items("block_sig_reg_partition_mapping", "sig_reg_part_mapping_id", sig_reg_part_mapping_id);
    }

    if (!is_register_signal() && originally_register_signal)
        success = success && DataBaseHandler::delete_items("signal_reg_signal", "reg_sig_id", get_reg_sig_id());
    else if (is_register_signal())
    {
        if (originally_register_signal)
            success = success && DataBaseHandler::update_items("signal_reg_signal", "reg_sig_id", get_reg_sig_id(), {{"init_value", get_value()}, {"reg_type_id", get_register_type_id()}});
        else
        {
            QVector<QVector<QString> > items;
            success = success && DataBaseHandler::insert_item("signal_reg_signal", {"sig_id", "init_value", "reg_type_id"}, {get_signal_id(), get_value(), get_register_type_id()});
            success = success && DataBaseHandler::show_items("signal_reg_signal", {"reg_sig_id"}, "sig_id", signal_id_, items);
            reg_sig_id_ = items[0][0];
        }
        if (get_width().toInt() == 1 && comboBoxSigLSB_->count() > 0 \
                && comboBoxSigMSB_->count() > 0 && ui->comboBoxReg->count() > 0 \
                && ui->comboBoxRegPart->count() >0 && ui->pushButtonAddSigPart->isEnabled())
        {
            on_pushButtonAddSigPart_clicked();
        }
        for (int row = 0; row < ui->tableWidgetSigPart->rowCount(); row++)
        {
            QString reg_lsb, reg_msb, sig_lsb, sig_msb;
            QString sig_part = ui->tableWidgetSigPart->item(row, 0)->text().replace("<", "").replace(">", "");
            QString reg_name = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[0];
            QString reg_part = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[1].replace(">", "");
            if (msb_first_)
            {
                sig_lsb = sig_part.split(":")[1];
                sig_msb = sig_part.split(":")[0];
                reg_lsb = reg_part.split(":")[1];
                reg_msb = reg_part.split(":")[0];
            }
            else {
                sig_lsb = sig_part.split(":")[0];
                sig_msb = sig_part.split(":")[1];
                reg_lsb = reg_part.split(":")[0];
                reg_msb = reg_part.split(":")[1];
            }
            success = success && add_signal_partition(sig_lsb, sig_msb, reg_lsb, reg_msb, reg_name2id_[reg_name], reg_sig_id_);
        }
    }
    if (success)
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, windowTitle(), "Unable to edit signal.\nError massage: " + DataBaseHandler::get_error_message());
    return false;
}
