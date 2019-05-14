#ifndef EDIT_REGISTER_DIALOG_H
#define EDIT_REGISTER_DIALOG_H

#include <QDialog>

namespace Ui {
class EditRegisterDialog;
}

class EditRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRegisterDialog(const QString& block_id, QWidget *parent = nullptr);
    explicit EditRegisterDialog(const QString& block_id, const QString& reg_id, QWidget *parent = nullptr);
    ~EditRegisterDialog();
    QString get_reg_name() const;
    QString get_reg_id() const;
    QString get_reg_type() const;
    QString get_reg_type_id() const;
    bool add_register();
    bool edit_register();

private:
    enum MODE {NEW, EDIT};
    Ui::EditRegisterDialog *ui;
    QVector<QString> reg_type_ids_;
    const QString block_id_;
    QString reg_id_;
    QString original_register_name_;
    const MODE mode_;
    void accept();
    void setup_ui();
    bool sanity_check();
};

#endif // EDIT_REGISTER_DIALOG_H
