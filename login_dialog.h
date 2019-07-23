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
    void set_username(const QString& username);
    void set_password(const QString& password);
    void set_save_password(bool save=true);

    QString get_username() const;
    QString get_password() const;
    bool save_password() const;
    bool login();
    void clear();

signals:
    void logged_in(QString);

private:
    void accept();
    Ui::LoginDialog *ui;
};

#endif // LOGIN_DIALOG_H
