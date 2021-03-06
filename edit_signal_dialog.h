#ifndef EDIT_SIGNAL_DIALOG_H
#define EDIT_SIGNAL_DIALOG_H

#include <QDialog>
#include "edit_signal_partition_logic.h"
#include "data_utils.h"
#include <QSet>

class QComboBox;
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
    void on_lineEditWidth_editingFinished();
    void on_lineEditWidth_textChanged(const QString &arg1);

    void on_comboBoxSigType_currentIndexChanged(int index);
    void on_comboBoxRegType_currentIndexChanged(int index);
    void on_comboBoxReg_currentIndexChanged(int index);
    void sigLSB_currentIndexChanged(int index);
    void sigMSB_currentIndexChanged(int index);
    void on_checkBoxAddPartition_stateChanged(int arg1);

    void on_pushButtonAddReg_clicked();
    void on_pushButtonAddSigPart_clicked();
    void on_pushButtonRemoveSigPart_clicked();

    void on_tableWidgetSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_checkBoxAddPort_clicked(bool);


private:
    bool setup_ui();
    int get_current_signal_lsb() const;
    int get_current_signal_msb() const;
    void make_occupied_signal_parts();
    void display_available_register_parts();

    bool sanity_check();
    bool check_name();
    bool check_width();
    bool check_value();
    bool check_partitions();
    void accept();

private:
    Ui::EditSignalDialog *ui;
    QHash<QString, QString> sig_type2id_, sig_type_id2type_, reg_name2id_;
    QHash<QString, QString> reg_type2id_, reg_type_id2type_;
    QSet<QString> writable_reg_types_;
    QHash<QString, QVector<QString>> sig_type2reg_types_;
    const QString chip_id_, block_id_;
    QString signal_id_, reg_sig_id_;
    const DIALOG_MODE mode_;
    QVector<QString> original_sig_reg_mapping_part_ids_;
    QString original_signal_given_name_, original_sig_type_id_, original_width_, original_reg_type_id_, original_value_;
    int last_width_ = -1;
    QComboBox* comboBoxSigLSB_, *comboBoxSigMSB_;
    const bool enabled_;
    QSet<QString> registers_in_page_;
};



#endif // EDIT_SIGNAL_DIALOG_H
