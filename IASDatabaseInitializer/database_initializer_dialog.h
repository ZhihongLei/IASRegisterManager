#ifndef DATABASE_INITIALIZER_DIALOG_H
#define DATABASE_INITIALIZER_DIALOG_H

#include <QDialog>

namespace Ui {
class DatabaseInitializerDialog;
}

class DatabaseInitializerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseInitializerDialog(QWidget *parent = nullptr);
    ~DatabaseInitializerDialog();
public slots:
    void on_logged_in();

private slots:
    void on_pushButtonAdd_clicked();
    bool init_database(const QString& database);

private:
    Ui::DatabaseInitializerDialog *ui;
};

#endif // DATABASE_INITIALIZER_DIALOG_H
