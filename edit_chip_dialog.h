#ifndef EDIT_CHIP_DIALOG_H
#define EDIT_CHIP_DIALOG_H

#include <QDialog>
#include "data_utils.h"

class Authenticator;
namespace Ui {
class EditChipDialog;
}

class EditChipDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditChipDialog(const QString& username, const QString& user_id, QWidget *parent = nullptr);
    explicit EditChipDialog(const QString& chip_id,
                            const QString& chip_name,
                            const QString& chip_owner,
                            int register_width,
                            int address_width,
                            bool msb_first,
                            QWidget *parent = nullptr);
    explicit EditChipDialog(const QString& username,
                            const QString& user_id,
                            const QString& chip_id,
                            const QString& chip_name,
                            int register_width,
                            int address_width,
                            bool msb_first,
                            QWidget *parent = nullptr);
    ~EditChipDialog();
    QString get_chip_name() const;
    QString get_chip_id() const;
    QString get_owner() const;
    QString get_owner_id() const;
    int get_register_width() const;
    int get_address_width() const;
    bool msb_first() const;
    QString get_project_role_id() const;
    QString get_project_role() const;
    bool add_chip();
    bool add_chip_from();
    bool edit_chip();

private:
    bool setup_ui();
    void accept();
    bool sanity_check();
    bool check_name();
    bool check_register_width();
    bool check_address_width();

    Ui::EditChipDialog *ui;
    QString chip_id_, project_role_id_, project_role_;
    QString original_chip_name_, original_chip_owner_;
    int original_register_width_, original_address_width_;
    QHash<QString, QString> username2id_;
    const DIALOG_MODE mode_;
};

#endif // EDIT_CHIP_DIALOG_H
