#ifndef CHANGE_PASSWORD_DIALOG_H
#define CHANGE_PASSWORD_DIALOG_H

#include <QDialog>

namespace Ui {
class ChangePasswordDialog;
}

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(const QString& username, QWidget *parent = nullptr);
    ~ChangePasswordDialog();
    bool change_password();

private:
    void accept();
    bool sanity_check();
    bool check_password();
    Ui::ChangePasswordDialog *ui;
    const QString username_;

};

#endif // CHANGE_PASSWORD_DIALOG_H
