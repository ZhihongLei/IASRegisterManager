#ifndef EDIT_SIGNAL_DIALOG_H
#define EDIT_SIGNAL_DIALOG_H

#include <QDialog>
#include <QHash>
#include "edit_signal_partition_logic.h"
#include <QValidator>
#include <QRegExpValidator>
#include "data_utils.h"
#include <QComboBox>

namespace Ui {
class EditSignalDialog;
}

class EditSignalDialog : public QDialog, public EditSignalPartitionLogic
{
    Q_OBJECT

public:
    explicit EditSignalDialog(const QString& chip_id, const QString& block_id, int register_width, bool msb_first=true, QWidget *parent = nullptr);
    explicit EditSignalDialog(const QString& chip_id, const QString& block_id, const QString& sig_id, const QString& reg_sig_id, int register_width, bool msb_first=true, bool enabled=true, QWidget *parent = nullptr);
    ~EditSignalDialog();
    QString get_signal_name() const;
    QString get_signal_id() const;
    QString get_reg_sig_id() const;
    QString get_value() const;
    QString get_width() const;
    QString get_signal_type() const;
    QString get_signal_type_id() const;
    QString get_register_type() const;
    QString get_register_type_id() const;
    bool is_register_signal() const;
    bool add_port() const;
    bool add_signal();
    bool edit_signal();



private slots:
    void on_lineEditValue_editingFinished();
    void on_lineEditWidth_editingFinished();

    void on_comboBoxSigType_currentIndexChanged(int index);
    void on_comboBoxRegType_currentIndexChanged(int index);
    void on_comboBoxReg_currentIndexChanged(int index);
    void on_comboBoxSigLSB_currentIndexChanged(int index);
    void on_comboBoxSigMSB_currentIndexChanged(int index);
    void on_tableWidgetSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_pushButtonAddSigPart_clicked();
    void on_pushButtonAddReg_clicked();
    void on_pushButtonRemoveSigPart_clicked();

    void on_pushButtonEditSigParts_clicked();


    void on_checkBoxAddPort_clicked(bool);

private:
    void make_occupied_signal_parts();
    void make_occupied_register_parts(const QString& reg_id);
    int get_current_signal_lsb();
    int get_current_signal_msb();

    void setup_ui();

    void display_available_register_parts();

    QString get_register_name();
    QString get_register_id();

    bool sanity_check();
    bool check_name();
    bool check_value();
    bool check_partitions();
    void accept();


private:
    Ui::EditSignalDialog *ui;
    QHash<QString, QString> sig_type2id_, sig_type_id2type_, reg_name2id_;
    QHash<QString, QString> reg_type2id_, reg_type_id2type_;
    QHash<QString, QVector<QString>> sig_type2reg_types_;
    const QString chip_id_, block_id_;
    QString signal_id_, reg_sig_id_;
    const DIALOG_MODE mode_;
    QVector<QString> original_sig_reg_mapping_part_ids_;
    QString original_shortened_signal_name_, original_sig_type_id_, original_width_, original_reg_type_id_, original_value_;
    int last_width_ = -1;
    bool edit_partitions_ = true;
    QComboBox* comboBoxSigLSB_, *comboBoxSigMSB_;
    const bool enabled_;
    bool is_register_page_control_signal_ = false;
};



#endif // EDIT_SIGNAL_DIALOG_H
