#include "edit_signal_partition_dialog.h"
#include "ui_edit_signal_partition_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include <QIntValidator>

EditSignalPartitionDialog::EditSignalPartitionDialog(const QString& block_id,
                                                     const QString& reg_sig_id,
                                                     const QString& reg_type_id,
                                                     int signal_width,
                                                     int register_width,
                                                     bool msb_first,
                                                     QWidget *parent) :
    QDialog(parent),
    EditSignalPartitionLogic(register_width, msb_first),
    block_id_(block_id),
    reg_type_id_(reg_type_id),
    reg_sig_id_(reg_sig_id),
    signal_width_(signal_width),
    ui(new Ui::EditSignalPartitionDialog)
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("block_register", {"reg_name", "reg_id"}, {{"block_id", block_id_}, {"reg_type_id", reg_type_id_}}, items);
    for (const auto& item : items)
    {
        ui->comboBoxReg->addItem(item[0]);
        reg_name2id_[item[0]] = item[1];
    }

    if (msb_first_)
    {
        QRect lsb_geo = ui->comboBoxSigLSB->geometry();
        QRect msb_geo = ui->comboBoxSigMSB->geometry();
        ui->comboBoxSigLSB->setGeometry(msb_geo);
        ui->comboBoxSigMSB->setGeometry(lsb_geo);
        QRect lsb_label_geo = ui->label_lsb->geometry();
        QRect msb_label_geo = ui->label_msb->geometry();
        ui->label_lsb->setGeometry(msb_label_geo);
        ui->label_msb->setGeometry(lsb_label_geo);
    }

    make_occupied_signal_parts();
    make_available_signal_parts(signal_width);
    emit(ui->comboBoxReg->currentIndexChanged(ui->comboBoxReg->currentIndex()));
}

EditSignalPartitionDialog::~EditSignalPartitionDialog()
{
    delete ui;
}

void EditSignalPartitionDialog::make_occupied_signal_parts()
{
    QVector<QVector<QString> > items;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.show_items("signal_reg_sig_partition", {"lsb", "msb"}, "reg_sig_id", reg_sig_id_, items);
    for (const auto& item : items)
    {
        occupied_signal_parts_.push_back({item[0].toInt(), item[1].toInt()});
    }
}

QString EditSignalPartitionDialog::get_signal_lsb() const
{
    return ui->comboBoxSigLSB->currentText();
}

QString EditSignalPartitionDialog::get_signal_msb() const
{
    return ui->comboBoxSigMSB->currentText();
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

QString EditSignalPartitionDialog::get_reg_sig_part_id() const
{
    return reg_sig_part_id_;
}

void EditSignalPartitionDialog::accept()
{
    // TODO: check validity
    QDialog::accept();
}


void EditSignalPartitionDialog::on_comboBoxSigLSB_currentIndexChanged(int index)
{
    ui->comboBoxSigLSB->setEnabled(index != -1);
    //ui->comboBoxSigMSB->setEnabled(index != -1);
    ui->comboBoxRegPart->setEnabled(index != -1);
    if (msb_first_)
    {
        display_available_register_parts();
        return;
    }
    ui->comboBoxSigMSB->blockSignals(true);
    ui->comboBoxSigMSB->clear();
    make_available_signal_ends();
    for (int i : available_signal_ends_) ui->comboBoxSigMSB->addItem(QString::number(i));
    ui->comboBoxSigMSB->setCurrentIndex(ui->comboBoxSigMSB->count()-1);
    ui->comboBoxSigMSB->blockSignals(false);
    emit(ui->comboBoxSigMSB->currentIndexChanged(ui->comboBoxSigMSB->currentIndex()));
}

void EditSignalPartitionDialog::on_comboBoxSigMSB_currentIndexChanged(int index)
{
    //ui->comboBoxSigLSB->setEnabled(index != -1);
    ui->comboBoxSigMSB->setEnabled(index != -1);
    ui->comboBoxRegPart->setEnabled(index != -1);
    if (!msb_first_)
    {
        display_available_register_parts();
        return;
    }
    ui->comboBoxSigLSB->blockSignals(true);
    ui->comboBoxSigLSB->clear();
    make_available_signal_starts();
    for (int i : available_signal_starts_) ui->comboBoxSigLSB->addItem(QString::number(i));
    ui->comboBoxSigLSB->setCurrentIndex(ui->comboBoxSigLSB->count() > 0 ? 0 : -1);
    ui->comboBoxSigLSB->blockSignals(false);
    emit(ui->comboBoxSigLSB->currentIndexChanged(ui->comboBoxSigLSB->currentIndex()));
}


void EditSignalPartitionDialog::on_comboBoxReg_currentIndexChanged(int index)
{
    ui->comboBoxReg->setEnabled(index>=0);
    ui->comboBoxSigLSB->setEnabled(index>=0);
    ui->comboBoxSigMSB->setEnabled(index>=0);
    ui->comboBoxRegPart->setEnabled(index>=0);
    if (index < 0)
    {
        ui->comboBoxSigLSB->clear();
        ui->comboBoxSigMSB->clear();
        ui->comboBoxRegPart->clear();
        if (index < 0) return;
    }

    make_occupied_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts(reg_name2id_[ui->comboBoxReg->currentText()]);
    make_available_register_parts_by_length();

    if (!msb_first_)
    {
        make_available_signal_starts();
        ui->comboBoxSigLSB->blockSignals(true);
        ui->comboBoxSigLSB->clear();
        for (int i : available_signal_starts_) ui->comboBoxSigLSB->addItem(QString::number(i));
        ui->comboBoxSigLSB->setCurrentIndex(ui->comboBoxSigLSB->count() > 0 ? 0: -1);
        ui->comboBoxSigLSB->blockSignals(false);
        emit(ui->comboBoxSigLSB->currentIndexChanged(ui->comboBoxSigLSB->currentIndex()));

    }
    else {
        make_available_signal_ends();
        ui->comboBoxSigMSB->blockSignals(true);
        ui->comboBoxSigMSB->clear();
        for (int i : available_signal_ends_) ui->comboBoxSigMSB->addItem(QString::number(i));
        ui->comboBoxSigMSB->setCurrentIndex((ui->comboBoxSigMSB->count()-1));
        ui->comboBoxSigMSB->blockSignals(false);
        emit(ui->comboBoxSigMSB->currentIndexChanged(ui->comboBoxSigMSB->currentIndex()));
    }

}



bool EditSignalPartitionDialog::add_signal_partition()
{
    std::cout << "haha 0" << std::endl;
    QString sig_lsb = ui->comboBoxSigLSB->currentText();
    QString sig_msb = ui->comboBoxSigMSB->currentText();
    std::cout << "haha 1" << std::endl;
    QString reg_lsb = get_register_lsb();
    QString reg_msb = get_register_msb();
    std::cout << "haha 2" << std::endl;
    return EditSignalPartitionLogic::add_signal_partition(sig_lsb, sig_msb, reg_lsb, reg_msb, get_register_id(), reg_sig_id_);

}

int EditSignalPartitionDialog::get_current_signal_lsb()
{
    if (ui->comboBoxSigLSB->currentIndex() < 0) return -1;
    return ui->comboBoxSigLSB->currentText().toInt();
}

int EditSignalPartitionDialog::get_current_signal_msb()
{
    if (ui->comboBoxSigMSB->currentIndex() < 0) return -1;
    return ui->comboBoxSigMSB->currentText().toInt();
}

void EditSignalPartitionDialog::display_available_register_parts()
{
    std::cout << "signal partition length: " << get_partition_length() << std::endl;
    const partition_list& parts = get_available_register_parts_by_length(get_partition_length());
    ui->comboBoxRegPart->clear();
    for (const auto& segment : parts)
    {
        if (msb_first_)
            ui->comboBoxRegPart->addItem("<" + QString::number(segment.second) + ":" + QString::number(segment.first) + ">");
        else
            ui->comboBoxRegPart->addItem("<" + QString::number(segment.first) + ":" + QString::number(segment.second) + ">");
    }
    ui->comboBoxReg->setEnabled(ui->comboBoxSigLSB->count() > 0 || ui->comboBoxSigMSB->count() > 0);
}
