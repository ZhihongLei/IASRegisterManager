#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
signals:
    void logged_in();

private:
    void accept();
    Ui::LoginDialog *ui;
};

#endif // LOGIN_DIALOG_H
