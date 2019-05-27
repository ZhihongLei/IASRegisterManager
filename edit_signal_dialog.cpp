#include "edit_signal_dialog.h"
#include "ui_edit_signal_dialog.h"
#include "database_handler.h"
#include "global_variables.h"
#include <QRegExpValidator>
#include "edit_signal_partition_dialog.h"
#include "edit_register_dialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QtMath>
#include "data_utils.h"


EditSignalDialog::EditSignalDialog(const QString& block_id, int register_width, bool msb_first, QWidget *parent) :
    QDialog(parent),
    EditSignalPartitionLogic(register_width, msb_first),
    block_id_(block_id),
    ui(new Ui::EditSignalDialog),
    mode_(MODE::NEW)
{
    setup_ui();
    setWindowTitle("New Signal");
}

EditSignalDialog::EditSignalDialog(const QString& block_id, const QString& sig_id, const QString& reg_sig_id, int register_width, bool msb_first, QWidget *parent) :
    QDialog(parent),
    EditSignalPartitionLogic(register_width, msb_first),
    block_id_(block_id),
    signal_id_(sig_id),
    reg_sig_id_(reg_sig_id),
    ui(new Ui::EditSignalDialog),
    mode_(MODE::EDIT)
{
    setup_ui();
    setWindowTitle("Edit Signal");

    std::cout << "Constructing" << std::endl;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("signal_signal", {"sig_name", "width", "sig_type_id"}, "sig_id", get_signal_id(), items);
    assert (items.size() == 1);

    ui->lineEditSignalName->setText(items[0][0]);
    ui->lineEditWidth->setText(items[0][1]);
    emit(ui->lineEditWidth->editingFinished());

    original_signal_name_ = items[0][0];
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
        dbhandler.show_items("signal_reg_signal", {"sig_id", "init_value", "reg_type_id"}, "reg_sig_id", get_reg_sig_id(), items);
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
        QVector<QString> ext_fields = {"signal_reg_sig_partition.reg_sig_part_id",
                                       "block_reg_partition.reg_part_id",
                                           "signal_reg_sig_partition.lsb",
                                           "signal_reg_sig_partition.msb",
                                           "block_register.reg_name",
                                           "block_reg_partition.lsb",
                                           "block_reg_partition.msb"};

        items.clear();
        dbhandler.show_items_inner_join(ext_fields, {{{"signal_reg_sig_partition", "reg_sig_part_id"}, {"block_reg_partition", "reg_sig_part_id"}},
                                                     {{"block_reg_partition", "reg_id"}, {"block_register", "reg_id"}}}, items, "signal_reg_sig_partition.reg_sig_id = \"" + reg_sig_id + "\"");
        if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[3] > b[3];});
        else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] < b[2];});
        for (const auto& item : items)
        {
            int row = ui->tableWidgetSigPart->rowCount();
            ui->tableWidgetSigPart->insertRow(row);
            QString reg_sig_part_id = item[0], reg_part_id = item[1], sig_lsb = item[2], sig_msb = item[3], reg_name = item[4], reg_lsb = item[5], reg_msb = item[6];
            QString sig_part, reg_part;
            if (msb_first_)
            {
                sig_part = "<" + sig_msb + ":"+ sig_lsb + ">";
                reg_part = reg_name + "<" + reg_msb + ":"+ reg_lsb +">";
            }
            else
            {
                sig_part = "<" + sig_lsb + ":"+ sig_msb + ">";
                reg_part = reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
            }
            ui->tableWidgetSigPart->setItem(row, 0, new QTableWidgetItem(sig_part));
            ui->tableWidgetSigPart->setItem(row, 1, new QTableWidgetItem(reg_part));
            original_sig_part_ids_.append(reg_sig_part_id);
            original_reg_part_ids_.append(reg_part_id);
            occupied_signal_parts_.push_back({sig_lsb.toInt(), sig_msb.toInt()});
            make_occupied_register_parts(reg_name2id_[reg_name]);
        }

        emit(ui->lineEditWidth->editingFinished());

        if (get_width().toInt() == 1 && ui->tableWidgetSigPart->rowCount() > 0)
        {
            ui->tableWidgetSigPart->setCurrentCell(0, 0);
            QString reg_name = ui->tableWidgetSigPart->item(0, 1)->text().split("<")[0];
            QString reg_lsb = ui->tableWidgetSigPart->item(0, 1)->text().replace(">", "").split(":")[1];
            //emit(ui->pushButtonRemoveSigPart->clicked());
            on_pushButtonRemoveSigPart_clicked();
            //emit(ui->lineEditWidth->editingFinished());
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



    //ui->lineEditWidth->setEnabled(false);
    //ui->comboBoxSigType->setEnabled(false);
    //ui->comboBoxRegType->setEnabled(false);

}

void EditSignalDialog::setup_ui()
{
    ui->setupUi(this);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("def_signal_type", {"sig_type_id", "sig_type"}, items, "", "order by sig_type_id");
    for (const auto& item : items)
    {
        QString sig_type = item[1], id = item[0];
        sig_type2id_[sig_type] = id;
        sig_type_id2type_[id] = sig_type;
        ui->comboBoxSigType->addItem(sig_type);

    }
    items.clear();
    dbhandler.show_items("def_register_type", {"reg_type_id", "reg_type"}, items, "", "order by reg_type_id");
    for (const auto& item : items)
    {
        QString reg_type = item[1], id = item[0];
        reg_type2id_[reg_type] = id;
        reg_type_id2type_[id] = reg_type;
        ui->comboBoxRegType->addItem(reg_type);
    }

    items.clear();
    dbhandler.show_items("def_sig_reg_type_mapping", {"sig_type_id", "reg_type_id"}, items, "", "order by mapping_id");
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

    if (msb_first_)
    {
        QRect lsb_geo = ui->comboBoxSigLSB->geometry();
        QRect msb_geo = ui->comboBoxSigMSB->geometry();
        ui->comboBoxSigLSB->setGeometry(msb_geo);
        ui->comboBoxSigMSB->setGeometry(lsb_geo);
    }
    emit(ui->lineEditWidth->editingFinished());
    emit(ui->comboBoxSigType->currentIndexChanged(ui->comboBoxSigType->currentIndex()));
    emit(ui->tableWidgetSigPart->currentCellChanged(-1, -1, -1, -1));
}

EditSignalDialog::~EditSignalDialog()
{
    delete ui;
}

QString EditSignalDialog::get_signal_name() const
{
    return ui->lineEditSignalName->text();
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
    return ui->lineEditValue->text();
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

bool EditSignalDialog::is_register_signal() const
{
    return sig_type2reg_types_.find(get_signal_type()) != sig_type2reg_types_.end();
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

int EditSignalDialog::get_current_signal_lsb()
{
    if (ui->comboBoxSigLSB->currentIndex() >= 0) return ui->comboBoxSigLSB->currentText().toInt();
    else return -1;
}

int EditSignalDialog::get_current_signal_msb()
{
    if (ui->comboBoxSigMSB->currentIndex() >= 0) return ui->comboBoxSigMSB->currentText().toInt();
    else return -1;
}


QString EditSignalDialog::get_register_name()
{
    return ui->comboBoxReg->currentText();
}

QString EditSignalDialog::get_register_id()
{
    return reg_name2id_[get_register_name()];
}

void EditSignalDialog::make_occupied_register_parts(const QString &reg_id)
{
    EditSignalPartitionLogic::make_occupied_register_parts(reg_id);
    /*
    for (int row = 0; row < ui->tableWidgetSigPart->rowCount(); row++)
    {
        QString reg_name = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[0];
        if (get_register_name() == reg_name)
        {
            QString part = ui->tableWidgetSigPart->item(row, 1)->text().split("<")[1].replace(">", "");
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
            occupied_register_parts_.push_back({lsb, msb});
        }
    }
    */

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
            ui->comboBoxRegPart->addItem("<" + QString::number(segment.second) + ":" + QString::number(segment.first) + ">");
        else
            ui->comboBoxRegPart->addItem("<" + QString::number(segment.first) + ":" + QString::number(segment.second) + ">");
    }
    ui->comboBoxReg->setEnabled(ui->comboBoxSigLSB->count() > 0 || ui->comboBoxSigMSB->count() > 0);
}


void EditSignalDialog::on_lineEditValue_editingFinished()
{

}

void EditSignalDialog::on_lineEditWidth_editingFinished()
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
            emit(ui->pushButtonRemoveSigPart->clicked());
            row_count = ui->tableWidgetSigPart->rowCount();
        }
        ui->lineEditValue->setText("0x");
    }
    last_width_ = get_width().toInt();
    make_available_signal_parts(get_width().toInt());
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

    edit_partitions_ = !edit_partitions_;
    emit(ui->pushButtonEditSigParts->clicked());

}


void EditSignalDialog::on_comboBoxSigType_currentIndexChanged(int index)
{
    ui->lineEditValue->setEnabled(index>=0 && is_register_signal());
    if (!is_register_signal()) ui->lineEditValue->setText("");
    if (is_register_signal() && ui->lineEditValue->text() == "") ui->lineEditValue->setText("0x");
    ui->comboBoxSigType->setEnabled(index>=0);
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
    std::cout << "hah regtype" << std::endl;
    ui->comboBoxRegType->setEnabled(index>=0);
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
        DataBaseHandler dbhandler(gDBHost, gDatabase);
        QVector<QVector<QString> > items;

        dbhandler.show_items("block_register", {"reg_name", "reg_id"}, {{"block_id", block_id_}, {"reg_type_id", get_register_type_id()}}, items);
        for (const auto& item : items)
        {
            std::cout << item[0].toUtf8().constData() <<std::endl;
            reg_name2id_[item[0]] = item[1];
            ui->comboBoxReg->addItem(item[0]);
            std::cout << "reg id: " << item[1].toUtf8().constData() << std::endl;
        }
    }
    ui->comboBoxReg->blockSignals(false);
    emit(ui->comboBoxReg->currentIndexChanged(ui->comboBoxReg->currentIndex()));
}

void EditSignalDialog::on_comboBoxReg_currentIndexChanged(int index)
{
    ui->comboBoxReg->setEnabled(index>=0);
    ui->pushButtonAddSigPart->setEnabled(index != -1);
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
    emit(ui->lineEditWidth->editingFinished());
}


void EditSignalDialog::on_comboBoxSigLSB_currentIndexChanged(int index)
{
    ui->pushButtonAddSigPart->setEnabled(index != -1);
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

void EditSignalDialog::on_comboBoxSigMSB_currentIndexChanged(int index)
{
    ui->pushButtonAddSigPart->setEnabled(index != -1);
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


void EditSignalDialog::on_tableWidgetSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveSigPart->setEnabled(currentRow >= 0);
}


void EditSignalDialog::on_pushButtonAddReg_clicked()
{
    EditRegisterDialog new_reg(block_id_);
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
    QString sig_lsb = ui->comboBoxSigLSB->currentText();
    QString sig_msb = ui->comboBoxSigMSB->currentText();
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

bool EditSignalDialog::sanity_check()
{
    if (!check_name()) return false;
    if (is_register_signal()) return check_value() && check_partitions();
    return true;
}

bool EditSignalDialog::check_name()
{
    QString warning_title = mode_ == MODE::NEW ? "Add Signal" : "Edit Signal";
    if (get_signal_name() == "")
    {
        QMessageBox::warning(this, warning_title, "Signal name must not be empty!");
        return false;
    }
    if (mode_ == EDIT && get_signal_name() == original_signal_name_) return true;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("signal_signal", {"sig_name"}, {{"block_id", block_id_}, {"sig_name", get_signal_name()}}, items);
    if (items.size() > 0)
    {
        QMessageBox::warning(this, warning_title, "Signal " + get_signal_name() + " already exists!");
        return false;
    }
    // anything else?
    return true;
}

bool EditSignalDialog::check_value()
{
    QString warning_title = mode_ == MODE::NEW ? "Add Signal" : "Edit Signal";
    // temporary solution
    if (get_value() == "")
    {
        QMessageBox::warning(this, warning_title, "Value must not be empty!");
        return false;
    }
    int width = get_width().toInt();
    bool ok;
    uint64_t value = get_value().toULongLong(&ok, 16);
    if (!ok)
    {
        QMessageBox::warning(this, warning_title, "Invalid signal value " + get_value());
        return false;
    }
    uint64_t max_value = static_cast<uint64_t>(qPow(2, width));
    if (value >= max_value)
    {
        QMessageBox::warning(this, warning_title, "Signal value is too large!");
        return false;
    }
    return true;
}

bool EditSignalDialog::check_partitions()
{
    // TODO
    QString warning_title = mode_ == MODE::NEW ? "Add Signal" : "Edit Signal";
    return true;
}

void EditSignalDialog::accept()
{
    if (sanity_check()) return QDialog::accept();
}

bool EditSignalDialog::add_signal()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    if (dbhandler.insert_item("signal_signal", {"sig_name", "block_id", "width", "sig_type_id"}, {get_signal_name(), block_id_, get_width(), get_signal_type_id()}) && \
            dbhandler.show_items("signal_signal", {"sig_id"}, {{"sig_name", get_signal_name()}, {"block_id", block_id_}}, items))
    {
        signal_id_ = items[0][0];
        if (is_register_signal())
        {
            items.clear();
            if (dbhandler.insert_item("signal_reg_signal", {"sig_id", "init_value", "reg_type_id"}, {get_signal_id(), get_value(), get_register_type_id()}) && \
                    dbhandler.show_items("signal_reg_signal", {"reg_sig_id"}, "sig_id", signal_id_, items))
            {
                reg_sig_id_ = items[0][0];
                if (get_width().toInt() == 1 && ui->comboBoxSigLSB->count() > 0 \
                        && ui->comboBoxSigMSB->count() > 0 && ui->comboBoxReg->count() > 0 \
                        && ui->comboBoxRegPart->count() >0 && edit_partitions_ && ui->pushButtonAddSigPart->isEnabled())
                {
                    emit(ui->pushButtonAddSigPart->clicked());
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
                    add_signal_partition(sig_lsb, sig_msb, reg_lsb, reg_msb, reg_name2id_[reg_name], reg_sig_id_);
                }
                return true;
            }
            else return false;
        }
        else return true;
    }
    return false;
}


bool EditSignalDialog::edit_signal()
{
    // TODO
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (!dbhandler.update_items("signal_signal", "sig_id", get_signal_id(),
                                {{"sig_name", get_signal_name()}, {"width", get_width()}, {"sig_type_id", get_signal_type_id()}})) return false;


    bool originally_register_signal = (original_reg_type_id_ != "");

    if (!originally_register_signal && !is_register_signal()) return true;

    for (const QString& reg_part_id : original_reg_part_ids_)
        dbhandler.delete_items("block_reg_partition", "reg_part_id", reg_part_id);
    for (const QString& sig_part_id : original_sig_part_ids_)
        dbhandler.delete_items("signal_reg_sig_partition", "reg_sig_part_id", sig_part_id);

    if (originally_register_signal && !is_register_signal())
    {
        dbhandler.delete_items("signal_reg_signal", "reg_sig_id", get_reg_sig_id());
        return true;
    }

    if (is_register_signal())
    {
        if (originally_register_signal)
            dbhandler.update_items("signal_reg_signal", "reg_sig_id", get_reg_sig_id(), {{"init_value", get_value()}, {"reg_type_id", get_register_type_id()}});
        else
        {
            QVector<QVector<QString> > items;
            dbhandler.insert_item("signal_reg_signal", {"sig_id", "init_value", "reg_type_id"}, {get_signal_id(), get_value(), get_register_type_id()});
            dbhandler.show_items("signal_reg_signal", {"reg_sig_id"}, "sig_id", signal_id_, items);
            reg_sig_id_ = items[0][0];
        }
        if (get_width().toInt() == 1 && ui->comboBoxSigLSB->count() > 0 \
                && ui->comboBoxSigMSB->count() > 0 && ui->comboBoxReg->count() > 0 \
                && ui->comboBoxRegPart->count() >0 && edit_partitions_ && ui->pushButtonAddSigPart->isEnabled())
        {
            emit(ui->pushButtonAddSigPart->clicked());
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
            add_signal_partition(sig_lsb, sig_msb, reg_lsb, reg_msb, reg_name2id_[reg_name], reg_sig_id_);
        }
    }
    return true;
}


void EditSignalDialog::on_pushButtonEditSigParts_clicked()
{
    edit_partitions_ = !edit_partitions_;

    if (edit_partitions_)
    {
        ui->comboBoxReg->show();
        ui->comboBoxRegPart->show();
        ui->pushButtonAddReg->show();

        if (get_width().toInt() == 1)
        {
            ui->comboBoxSigLSB->hide();
            ui->comboBoxSigMSB->hide();
            ui->tableWidgetSigPart->hide();
            ui->pushButtonAddSigPart->hide();
            ui->pushButtonRemoveSigPart->hide();

            ui->comboBoxReg->move(40, ui->comboBoxReg->y());
            ui->comboBoxRegPart->move(280, ui->comboBoxReg->y());
            ui->pushButtonEditSigParts->move(ui->pushButtonEditSigParts->x(), 250);
            ui->buttonBox->move(ui->buttonBox->x(), 250);
            resize(550, 310);


            if (ui->tableWidgetSigPart->rowCount() > 0 )
            {
                assert (ui->tableWidgetSigPart->rowCount() == 1);


            }

        }
        else {
            ui->comboBoxSigLSB->show();
            ui->comboBoxSigMSB->show();
            ui->tableWidgetSigPart->show();
            ui->pushButtonAddSigPart->show();
            ui->pushButtonRemoveSigPart->show();

            ui->comboBoxReg->move(180, ui->comboBoxReg->y());
            ui->comboBoxRegPart->move(420, ui->comboBoxReg->y());
            ui->pushButtonEditSigParts->move(ui->pushButtonEditSigParts->x(), 500);
            ui->buttonBox->move(ui->buttonBox->x(), 500);
            resize(550, 560);
        }
        ui->pushButtonEditSigParts->setText("Hide");
    }
    else
    {
        ui->comboBoxSigLSB->hide();
        ui->comboBoxSigMSB->hide();
        ui->comboBoxReg->hide();
        ui->comboBoxRegPart->hide();
        ui->tableWidgetSigPart->hide();
        ui->pushButtonAddReg->hide();
        ui->pushButtonAddSigPart->hide();
        ui->pushButtonRemoveSigPart->hide();
        ui->pushButtonEditSigParts->setText("Edit Partitions");
        ui->pushButtonEditSigParts->move(ui->pushButtonEditSigParts->x(), 200);
        ui->buttonBox->move(ui->buttonBox->x(), 200);
        resize(550, 260);
    }

}