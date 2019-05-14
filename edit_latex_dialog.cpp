#include "edit_latex_dialog.h"
#include "ui_edit_latex_dialog.h"
#include <QUrl>
#include <iostream>

// src=\"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/MathJax.js?config=TeX-AMS-MML_HTMLorMML\">\n

QString html_template("\
<html>\n    \
<head>\n    \
    <script type=\"text/x-mathjax-config\">\n \
        MathJax.Hub.Config({tex2jax: {inlineMath: [['$','$'], ['\\(','\\)']]}});\n    \
    </script>\n   \
    <script type=\"text/javascript\"\n  \
        src=\"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/MathJax.js?config=TeX-AMS-MML_HTMLorMML\">\n   \
    </script>\n \
</head>\n   \
<body>\n    \
    CONTENT \
</body>\n   \
</html>");


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
    QString content = ui->plainTextEdit->toPlainText();
    QString html = html_template;
    std::cout << html.toUtf8().constData() << std::endl;
    html.replace("CONTENT", content);
    web->setHtml(html, QUrl("/Users/zhihonglei/tools/"));
    std::cout << web->url().path().toUtf8().constData() << std::endl;
}
