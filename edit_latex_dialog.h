#ifndef EDIT_LATEX_DIALOG_H
#define EDIT_LATEX_DIALOG_H

#include <QDialog>
#include <QtWebEngineWidgets>

namespace Ui {
class EditLaTeXDialog;
}

class EditLaTeXDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditLaTeXDialog(QWidget *parent = nullptr);
    ~EditLaTeXDialog();
    void show_latex();


private slots:
    void on_pushButton_clicked();

    void on_plainTextEdit_textChanged();

private:
    Ui::EditLaTeXDialog *ui;
    QWebEngineView *web;
};

#endif // EDIT_LATEX_DIALOG_H
