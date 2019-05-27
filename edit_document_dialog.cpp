#include "edit_doc_dialog.h"
#include "ui_edit_doc_dialog.h"

EditDocDialog::EditDocDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDocDialog)
{
    ui->setupUi(this);
}

EditDocDialog::~EditDocDialog()
{
    delete ui;
}
