#ifndef RESOURCES_BASE_DIR_DIALOG_H
#define RESOURCES_BASE_DIR_DIALOG_H

#include <QDialog>

namespace Ui {
class ResourcesBaseDirDialog;
}

class ResourcesBaseDirDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResourcesBaseDirDialog(const QString& basedir, QWidget *parent = nullptr);
    ~ResourcesBaseDirDialog();
    QString get_base_dir() const;

private slots:
    void on_pushButton_clicked();

private:
    Ui::ResourcesBaseDirDialog *ui;
};

#endif // RESOURCES_BASE_DIR_DIALOG_H
