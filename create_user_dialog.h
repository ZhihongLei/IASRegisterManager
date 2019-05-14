#ifndef CREATE_USER_DIALOG_H
#define CREATE_USER_DIALOG_H

#include <QDialog>

namespace Ui {
class CreateUserDialog;
}

class CreateUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateUserDialog(QWidget *parent = nullptr);
    ~CreateUserDialog();
    QString get_username();
    QString get_user_id();
    QString get_db_role();
    QString get_db_role_id();
    QString get_password();
    bool create_user();

private:
    void accept();
    bool sanity_check();
    bool check_name();
    bool check_password();
    Ui::CreateUserDialog *ui;
    QString user_id_;
    QVector<QString> role_ids_;
};

#endif // CREATE_USER_DIALOG_H
