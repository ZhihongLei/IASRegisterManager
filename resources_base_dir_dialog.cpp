#include "resources_base_dir_dialog.h"
#include "ui_resources_base_dir_dialog.h"
#include <QFileDialog>
#include <QDebug>

ResourcesBaseDirDialog::ResourcesBaseDirDialog(const QString& basedir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResourcesBaseDirDialog)
{
    ui->setupUi(this);
    setWindowTitle("Resource Base Dir");
    ui->lineEdit->setText(basedir);
}

ResourcesBaseDirDialog::~ResourcesBaseDirDialog()
{
    delete ui;
}

void ResourcesBaseDirDialog::on_pushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Base Dir");
    if (dir != "") ui->lineEdit->setText(dir);
}

QString ResourcesBaseDirDialog::get_base_dir() const
{
    return ui->lineEdit->text();
}
