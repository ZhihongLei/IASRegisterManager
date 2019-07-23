#ifndef EDIT_REGISTER_DIALOG_H
#define EDIT_REGISTER_DIALOG_H

#include <QDialog>
#include "data_utils.h"

namespace Ui {
class EditRegisterDialog;
}

class EditRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRegisterDialog(const QString& chip_id, const QString& block_id, QWidget *parent = nullptr);
    explicit EditRegisterDialog(const QString& chip_id, const QString& block_id, const QString& reg_id, bool enabled = true, QWidget *parent = nullptr);
    ~EditRegisterDialog();
    QString get_reg_name() const;
    QString get_reg_id() const;
    QString get_reg_type() const;
    QString get_reg_type_id() const;
    QString get_address() const;
    bool add_register();
    bool edit_register();

private:
    bool setup_ui();
    void accept();
    bool sanity_check();
    bool check_name();
    bool check_address();

    Ui::EditRegisterDialog *ui;
    const QString chip_id_, block_id_;
    QString reg_id_, address_;
    QString original_register_name_;
    QVector<QString> reg_type_ids_;
    const bool enabled_;
    const DIALOG_MODE mode_;


};

#endif // EDIT_REGISTER_DIALOG_H
