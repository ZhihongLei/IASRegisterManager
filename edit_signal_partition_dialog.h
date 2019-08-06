#ifndef EDIT_SIGNAL_PARTITION_DIALOG_H
#define EDIT_SIGNAL_PARTITION_DIALOG_H

#include <QDialog>
#include "edit_signal_partition_logic.h"

namespace Ui {
class EditSignalPartitionDialog;
}

class QComboBox;
class EditSignalPartitionDialog : public QDialog,  public EditSignalPartitionLogic
{
    Q_OBJECT

public:
    explicit EditSignalPartitionDialog(const QString& block_id,
                                       const QString& signal_id,
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
    QString get_sig_reg_part_mapping_id() const;
    bool add_signal_partition();

private slots:
    void on_comboBoxSigLSB_currentIndexChanged(int index);
    void on_comboBoxSigMSB_currentIndexChanged(int index);
    void on_comboBoxReg_currentIndexChanged(int index);

private:
    void make_occupied_signal_parts();
    int get_current_signal_lsb() const;
    int get_current_signal_msb() const;
    void display_available_register_parts();
    void accept();

    Ui::EditSignalPartitionDialog *ui;
    const QString block_id_, reg_type_id_, reg_sig_id_;
    const int signal_width_;
    QString signal_lsb_, signal_msb_, reg_lsb_, reg_msb_, reg_name_, reg_id_;
    QHash<QString, QString> reg_name2id_;
    QVector<int> available_sig_part_starts_, available_sig_part_ends_;
    QComboBox *comboBoxSigLSB_, *comboBoxSigMSB_;
};

#endif // EDIT_SIGNAL_PARTITION_DIALOG_H
