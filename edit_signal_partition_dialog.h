#ifndef EDIT_SIGNAL_PARTITION_DIALOG_H
#define EDIT_SIGNAL_PARTITION_DIALOG_H

#include <QDialog>
#include <QHash>
#include "edit_signal_partition_logic.h"
#include <QComboBox>

namespace Ui {
class EditSignalPartitionDialog;
}

class EditSignalPartitionDialog : public QDialog,  public EditSignalPartitionLogic
{
    Q_OBJECT

public:
    explicit EditSignalPartitionDialog(const QString& block_id,
                                       const QString& reg_sig_id,
                                       const QString& reg_type_id,
                                       int signal_width,
                                       int register_width,
                                       bool msb_first,
                                       QWidget *parent = nullptr);
    ~EditSignalPartitionDialog();

    QString get_signal_lsb() const;
    QString get_signal_msb() const;
    QString get_register_lsb() const;
    QString get_register_msb() const;
    QString get_register_name() const;
    QString get_register_id() const;
    QString get_reg_sig_part_id() const;
    bool add_signal_partition();

private:
    void make_occupied_signal_parts();
    int get_current_signal_lsb();
    int get_current_signal_msb();
    void display_available_register_parts();
    void accept();

private slots:
    void on_comboBoxSigLSB_currentIndexChanged(int index);
    void on_comboBoxSigMSB_currentIndexChanged(int index);
    void on_comboBoxReg_currentIndexChanged(int index);

private:
    Ui::EditSignalPartitionDialog *ui;
    const QString block_id_, reg_type_id_, reg_sig_id_;
    const int signal_width_;
    QString signal_lsb_, signal_msb_, reg_lsb_, reg_msb_, reg_name_, reg_id_, reg_sig_part_id_;
    QHash<QString, QString> reg_name2id_;
    QVector<int> available_sig_part_starts_, available_sig_part_ends_;
    QComboBox *comboBoxSigLSB_, *comboBoxSigMSB_;
};

#endif // EDIT_SIGNAL_PARTITION_DIALOG_H
