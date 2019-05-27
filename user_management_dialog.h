#ifndef USER_MANAGEMENT_DIALOG_H
#define USER_MANAGEMENT_DIALOG_H

#include <QDialog>
namespace Ui {
class UserManagementDialog;
}

class UserManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserManagementDialog(const QString& myself, QWidget *parent = nullptr);
    ~UserManagementDialog();

private slots:
    void on_pushButtonAddUser_clicked();
    void on_pushButtonRemoveUser_clicked();

    void on_tableWidgetUser_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    Ui::UserManagementDialog *ui;
    const QString myself_;
};

#endif // USER_MANAGEMENT_DIALOG_H
