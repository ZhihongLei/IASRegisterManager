#include "edit_latex_dialog.h"
#include "ui_edit_latex_dialog.h"
#include <QUrl>
#include "global_variables.h"


EditLaTeXDialog::EditLaTeXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditLaTeXDialog)
{
    ui->setupUi(this);
    web = new QWebEngineView(this);
    web->setGeometry(ui->webframe->geometry());
    web->show();
}

EditLaTeXDialog::~EditLaTeXDialog()
{
    delete ui;
    delete web;
}


void EditLaTeXDialog::on_pushButton_clicked()
{
}

void EditLaTeXDialog::on_plainTextEdit_textChanged()
{
    QString content = ui->plainTextEdit->toPlainText();
    QString html = html_template;
    html.replace("{CONTENT}", content);
    html.replace("{MATHJAX_ROOT}", mathjax_root);

    QString filename = "/Users/zhihonglei/test.html";
    QFile file(filename);
    if ( file.open(QIODevice::ReadWrite | QIODevice::Truncate) )
    {
        QTextStream out( &file );
        out << html << endl;
    }
    web->setHtml(html, QUrl("file://"));
}
