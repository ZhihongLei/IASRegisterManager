#ifndef EDIT_REGISTER_PAGE_DIALOG_H
#define EDIT_REGISTER_PAGE_DIALOG_H

#include <QDialog>
#include "data_utils.h"

namespace Ui {
class EditRegisterPageDialog;
}

class EditRegisterPageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRegisterPageDialog(const QString& chip_id, QWidget *parent = nullptr);
    explicit EditRegisterPageDialog(const QString& chip_id, const QString& page_id, QWidget *parent = nullptr);
    ~EditRegisterPageDialog();
    QString get_register_page_id() const;
    QString get_register_page_name() const;
    QString get_control_signal_name() const;
    QString get_control_signal_id() const;
    QString get_page_count() const;
    bool add_register_page();
    bool edit_register_page();

private slots:
    void on_comboBoxControlSignal_currentIndexChanged(int index);
    void on_tableWidgetAll_cellDoubleClicked(int row, int column);
    void on_tableWidgetSelected_cellDoubleClicked(int row, int column);
    void on_tableWidgetAll_cellClicked(int row, int column);
    void on_tableWidgetSelected_cellClicked(int row, int column);
    void on_pushButton_clicked();
    void on_lineEditSearchRegister_textChanged(const QString &arg1);
    void on_lineEditSearchControlSignal_textChanged(const QString &arg1);


private:
    bool setup_ui();
    QSet<QString> get_available_control_signal_types() const;
    QSet<QString> get_available_paged_register_types() const;
    void accept();
    bool sanity_check();
    bool check_name();

    Ui::EditRegisterPageDialog *ui;
    const QString chip_id_;
    bool to_add_register_;
    QHash<QString, QString> sig_name2id_;
    QHash<QString, int> sig_name2width_;
    QHash<QString, QString> reg_id2name_;
    QString page_id_;
    QString original_control_sig_id_, original_page_name_;
    const DIALOG_MODE mode_;
};

#endif // EDIT_REGISTER_PAGE_DIALOG_H
