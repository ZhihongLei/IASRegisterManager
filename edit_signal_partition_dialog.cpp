#include "edit_signal_partition_dialog.h"
#include "ui_edit_signal_partition_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QPushButton>
#include "data_utils.h"
#include <QDebug>
#include <QMessageBox>

EditSignalPartitionDialog::EditSignalPartitionDialog(const QString& block_id,
                                                     const QString& signal_id,
                                                     const QString& reg_sig_id,
                                                     const QString& reg_type_id,
                                                     int signal_width,
                                                     int register_width,
                                                     bool msb_first,
                                                     QWidget *parent) :
    QDialog(parent),
    EditSignalPartitionLogic(register_width, msb_first),
    ui(new Ui::EditSignalPartitionDialog),
    block_id_(block_id),
    reg_type_id_(reg_type_id),
    reg_sig_id_(reg_sig_id),
    signal_width_(signal_width)
{
    ui->setupUi(this);
    setWindowTitle("Add Signal Partition");
    comboBoxSigLSB_ = msb_first_ ? ui->comboBoxSigRight : ui->comboBoxSigLeft;
    comboBoxSigMSB_ = msb_first_ ? ui->comboBoxSigLeft : ui->comboBoxSigRight;
    connect(comboBoxSigLSB_, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxSigLSB_currentIndexChanged(int)));
    connect(comboBoxSigMSB_, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxSigMSB_currentIndexChanged(int)));

    QVector<QVector<QString> > items;
    bool success = true;

    QSet<QString> registers_in_page;
    success = success && DataBaseHandler::show_items("chip_register_page", {"page_id"}, "ctrl_sig", signal_id, items);
    for (const auto& item : items)
    {
        QString page_id = item[0];
        QVector<QVector<QString> > regs;
        success = success && DataBaseHandler::show_items("chip_register_page_content", {"reg_id"}, "page_id", page_id, regs);
        for (const auto& reg : regs)
            registers_in_page.insert(reg[0]);
    }

    items.clear();
    success = success && DataBaseHandler::show_items("block_register", {"reg_name", "reg_id"}, {{"block_id", block_id_}, {"reg_type_id", reg_type_id_}}, items);
    qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[0] < b[0];});
    for (const auto& item : items)
    {
        QVector<QVector<QString> > partitions;
        DataBaseHandler::show_items("block_sig_reg_partition_mapping", {"reg_lsb", "reg_msb"}, "reg_id", item[1], partitions);
        int occupied_bits = 0;
        for (const auto& part : partitions) occupied_bits += part[1].toInt() - part[0].toInt() + 1;

        if (registers_in_page.contains(item[1])) continue;
        reg_name2id_[GLOBAL_REGISTER_NAMING.get_extended_name(item[0])] = item[1];
        if (occupied_bits < register_width) ui->comboBoxReg->addItem(GLOBAL_REGISTER_NAMING.get_extended_name(item[0]));
    }
    for (int i = 0; i < ui->comboBoxReg->count(); i++)
    {
        if (reg_name2id_[ui->comboBoxReg->itemText(i)] == RECENT_REGISTER_ID)
        {
            ui->comboBoxReg->setCurrentIndex(i);
            break;
        }
    }
    if (!success) QMessageBox::warning(this, windowTitle(), "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());

    make_occupied_signal_parts();
    make_available_signal_parts(signal_width);
    emit(ui->comboBoxReg->currentIndexChanged(ui->comboBoxReg->currentIndex()));
}

EditSignalPartitionDialog::~EditSignalPartitionDialog()
{
    delete ui;
}

QString EditSignalPartitionDialog::get_signal_lsb() const
{
    return comboBoxSigLSB_->currentText();
}

QString EditSignalPartitionDialog::get_signal_msb() const
{
    return comboBoxSigMSB_->currentText();
}

QString EditSignalPartitionDialog::get_register_lsb() const
{
    QString part = ui->comboBoxRegPart->currentText().replace("<", "").replace(">", "");
    if (msb_first_) return part.split(":")[1];
    else return part.split(":")[0];
}

QString EditSignalPartitionDialog::get_register_msb() const
{
    QString part = ui->comboBoxRegPart->currentText().replace("<", "").replace(">", "");
    if (msb_first_) return part.split(":")[0];
    else return part.split(":")[1];
}

QString EditSignalPartitionDialog::get_register_name() const
{
    return ui->comboBoxReg->currentText();
}

QString EditSignalPartitionDialog::get_register_id() const
{
    return reg_name2id_[get_register_name()];
}

QString EditSignalPartitionDialog::get_sig_reg_part_mapping_id() const
{
    return sig_reg_part_mapping_id_;
}

int EditSignalPartitionDialog::get_current_signal_lsb() const
{
    if (comboBoxSigLSB_->currentIndex() < 0) return -1;
    return comboBoxSigLSB_->currentText().toInt();
}

int EditSignalPartitionDialog::get_current_signal_msb() const
{
    if (comboBoxSigMSB_->currentIndex() < 0) return -1;
    return comboBoxSigMSB_->currentText().toInt();
}

void EditSignalPartitionDialog::make_occupied_signal_parts()
{
    QVector<QVector<QString> > items;

    if (!DataBaseHandler::show_items("block_sig_reg_partition_mapping", {"sig_lsb", "sig_msb"}, "reg_sig_id", reg_sig_id_, items))
        QMessageBox::warning(this, windowTitle(), "Unable to read existing signal partitions from database.\nExceptions might happen.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
    for (const auto& item : items)
    {
        occupied_signal_parts_.push_back({item[0].toInt(), item[1].toInt()});
    }
}

void EditSignalPartitionDialog::display_available_register_parts()
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
    if (msb_first_ && ui->comboBoxRegPart->count() > 0) ui->comboBoxRegPart->setCurrentIndex(0);
    ui->comboBoxReg->setEnabled(comboBoxSigLSB_->count() > 0 || comboBoxSigMSB_->count() > 0);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(comboBoxSigLSB_->count() > 0 || comboBoxSigMSB_->count() > 0);
}

void EditSignalPartitionDialog::accept()
{
    // TODO: check validity
    QDialog::accept();
}

void EditSignalPartitionDialog::on_comboBoxSigLSB_currentIndexChanged(int index)
{
    comboBoxSigLSB_->setEnabled(index != -1);
    //comboBoxSigMSB_->setEnabled(index != -1);
    ui->comboBoxRegPart->setEnabled(index != -1);
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

void EditSignalPartitionDialog::on_comboBoxSigMSB_currentIndexChanged(int index)
{
    //comboBoxSigLSB_->setEnabled(index != -1);
    comboBoxSigMSB_->setEnabled(index != -1);
    ui->comboBoxRegPart->setEnabled(index != -1);
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


void EditSignalPartitionDialog::on_comboBoxReg_currentIndexChanged(int index)
{
    ui->comboBoxReg->setEnabled(index>=0);
    comboBoxSigLSB_->setEnabled(index>=0);
    comboBoxSigMSB_->setEnabled(index>=0);
    ui->comboBoxRegPart->setEnabled(index>=0);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(index>=0);
    if (index < 0)
    {
        comboBoxSigLSB_->clear();
        comboBoxSigMSB_->clear();
        ui->comboBoxRegPart->clear();
        return;
    }

    make_occupied_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts_by_length();

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
}

bool EditSignalPartitionDialog::add_signal_partition()
{
    QString sig_lsb = comboBoxSigLSB_->currentText(),
            sig_msb = comboBoxSigMSB_->currentText();
    QString reg_lsb = get_register_lsb(),
            reg_msb = get_register_msb();
    if (EditSignalPartitionLogic::add_signal_partition(sig_lsb, sig_msb, reg_lsb, reg_msb, get_register_id(), reg_sig_id_))
    {
        RECENT_REGISTER_ID = get_register_id();
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, windowTitle(), "Unable to add signal partition.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}
