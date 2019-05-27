#ifndef EDIT_DOC_DIALOG_H
#define EDIT_DOC_DIALOG_H

#include <QDialog>

namespace Ui {
class EditDocDialog;
}

class EditDocDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditDocDialog(QWidget *parent = nullptr);
    ~EditDocDialog();

private:
    Ui::EditDocDialog *ui;
};

#endif // EDIT_DOC_DIALOG_H
