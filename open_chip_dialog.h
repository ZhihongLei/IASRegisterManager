#ifndef OPEN_CHIP_DIALOG_H
#define OPEN_CHIP_DIALOG_H

#include <QDialog>

namespace Ui {
class OpenChipDialog;
}

class OpenChipDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenChipDialog(const QString& user_id, QWidget *parent = nullptr);
    ~OpenChipDialog();
    QString get_chip_id() const;
    QString get_chip_name() const;
    QString get_owner() const;
    QString get_owner_id() const;
    QString get_project_role_id() const;
    int get_register_width() const;
    int get_address_width() const;
    bool is_msb_first() const;

private:
    void accept();
    bool sanity_check();
    bool check_project_role();
    Ui::OpenChipDialog *ui;
    const QString user_id_;
    QString project_role_id_ = "-1";
    QVector<QString> chip_ids_;
    int address_width_, register_width_;
};

#endif // OPEN_CHIP_DIALOG_H
