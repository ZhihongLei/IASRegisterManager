#include "document_editor.h"
#include "ui_document_editor.h"
#include <QFileDialog>
#include <QRegExpValidator>
#include <QRegExp>
#include "global_variables.h"
#include "database_handler.h"
#include <QPushButton>
#include <QMessageBox>

DocumentEditor::DocumentEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DocumentEditor)
{
    ui->setupUi(this);
    level_ = DOCUMENT_LEVEL::BLOCK;
    mode_ = DIALOG_MODE::ADD;
    setup_ui();
    on_comboBoxDocType_currentIndexChanged(0);
}

DocumentEditor::~DocumentEditor()
{
    delete ui;
}

void DocumentEditor::setup_ui()
{
    show_preview_ = true;
    ui->stackedWidget->setCurrentIndex(0);
    ui->framePreview->setVisible(show_preview_);
    ui->pushButtonPreview->setVisible(true);
    ui->pushButtonPreview->setText("Show Preview");
    /*
    QVector<QWidget*> widgets = {ui->comboBoxDocType, ui->textEditText,
                                ui->lineEditImagePath, ui->lineEditImageCaption, ui->pushButtonSelectImage,
                                ui->tableWidget, ui->lineEditTableRow, ui->lineEditTableColumn, ui->pushButtonPreview, ui->lineEditTableTitle, ui->webEngineView};
    for (QWidget* w : widgets) w->setEnabled(enabled_);
    */

    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
    ui->lineEditTableRow->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*"), this));
    ui->lineEditTableColumn->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*"), this));

    ui->lineEditTableRow->setText("3");
    ui->lineEditTableColumn->setText("3");
    on_lineEditTableRow_editingFinished();
    on_lineEditTableColumn_editingFinished();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("def_doc_type", {"doc_type_id", "doc_type"}, items, "", "order by doc_type_id");

    for (const auto& item : items)
    {
        ui->comboBoxDocType->addItem(item[1]);
        document_types_.push_back(item[1]);
        document_type2id_[item[1]] = item[0];
    }
    //ui->comboBoxDocType->setCurrentIndex(0);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(ui->comboBoxDocType->count() > 0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}


void DocumentEditor::set_content(const QString& doc_id, const QString& doc_type, const QString& content)
{
    doc_id_ = doc_id;
    if (doc_type == ui->comboBoxDocType->itemText(0)) // text
    {
        ui->comboBoxDocType->setCurrentIndex(0);
        ui->textEditText->insertPlainText(content);
    }
    else if (doc_type == ui->comboBoxDocType->itemText(1)) // image
    {
        ui->comboBoxDocType->setCurrentIndex(1);
        QStringList ss = content.split(DOC_DELIMITER);
        ui->lineEditImageCaption->setText(ss[0]);
        ui->lineEditImagePath->setText(ss[1]);
        on_lineEditImageCaption_editingFinished();
    }
    else if (doc_type == ui->comboBoxDocType->itemText(2)) // table
    {
        ui->comboBoxDocType->setCurrentIndex(2);
        QStringList ss = content.split(DOC_DELIMITER);
        QString title = ss[0], rows = ss[1], cols = ss[2];
        ui->lineEditTableCaption->setText(title);
        ui->lineEditTableRow->setText(rows);
        ui->lineEditTableColumn->setText(cols);
        on_lineEditTableRow_editingFinished();
        on_lineEditTableColumn_editingFinished();
        for (int i = 0; i < rows.toInt(); i++)
            for (int j = 0; j < cols.toInt(); j++)
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(ss[i*cols.toInt() + j + 3]));
        //ui->tableWidget->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
        ui->tableWidget->resizeRowsToContents();
    }
}

void DocumentEditor::set_level(const DOCUMENT_LEVEL &level)
{
    level_ = level;
}

void DocumentEditor::set_block_id(const QString &block_id)
{
    block_id_ = block_id;
}

void DocumentEditor::set_register_id(const QString &reg_id)
{
    register_id_ = reg_id;
}

void DocumentEditor::set_signal_id(const QString &sig_id)
{
    signal_id_ = sig_id;
}

void DocumentEditor::set_mode(const DIALOG_MODE& mode)
{
    mode_ = mode;
}

void DocumentEditor::on_comboBoxDocType_currentIndexChanged(int index)
{
    if (index < 0) return;
    ui->splitter->setOrientation(index == 1 ? Qt::Vertical : Qt::Horizontal);
    ui->stackedWidget->setCurrentIndex(index);
    ui->webEngineView->setHtml("", QUrl("file://"));
}

void DocumentEditor::on_pushButtonSelectImage_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Select an image", "", "JPEG file (*.jpg *.jpeg);;PNG file (*.png);;PDF file (*.pdf);; All Files (*.* *)");
    if (file == "") return;
    ui->lineEditImagePath->setText(file);
    QString image = html_image_template;
    QString html = html_template;
    image.replace("{CAPTION}", ui->lineEditImageCaption->text()).replace("{IMAGE}", file);
    html.replace("{CONTENT}", image).replace("{MATHJAX_ROOT}", mathjax_root);
    ui->webEngineView->setHtml(html, QUrl("file://"));
}

void DocumentEditor::on_lineEditImageCaption_editingFinished()
{
    QString file = ui->lineEditImagePath->text();
    QString image = html_image_template;
    QString html = html_template;
    image.replace("{CAPTION}", ui->lineEditImageCaption->text()).replace("{IMAGE}", file);
    html.replace("{CONTENT}", image).replace("{MATHJAX_ROOT}", mathjax_root);
    ui->webEngineView->setHtml(html, QUrl("file://"));
}

void DocumentEditor::on_lineEditTableRow_editingFinished()
{
    ui->tableWidget->setRowCount(ui->lineEditTableRow->text().toInt());
    if (show_preview_) on_tableWidget_cellChanged(0, 0);
}

void DocumentEditor::on_lineEditTableColumn_editingFinished()
{
    ui->tableWidget->setColumnCount(ui->lineEditTableColumn->text().toInt());
    QStringList labels;
    for (int i = 0; i < ui->tableWidget->columnCount(); i++) labels << "";
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    if (show_preview_) on_tableWidget_cellChanged(0, 0);
}

bool DocumentEditor::sanity_check()
{
    // TODO
    return true;
}


void DocumentEditor::accept()
{
    //if (!enabled_) return QDialog::reject();
    //if (sanity_check()) QDialog::accept();
}


bool DocumentEditor::add_document()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    QString doc_id_field, obj_field, table, obj_id;
    if (level_ == DOCUMENT_LEVEL::BLOCK)
    {
        table = "doc_block";
        doc_id_field = "block_doc_id";
        obj_field = "block_id";
        obj_id = block_id_;

    }
    else if (level_ == DOCUMENT_LEVEL::REGISTER)
    {
        table = "doc_register";
        doc_id_field = "register_doc_id";
        obj_field = "reg_id";
        obj_id = register_id_;
    }
    else if (level_ == DOCUMENT_LEVEL::SIGNAL)
    {
        table = "doc_signal";
        doc_id_field = "signal_doc_id";
        obj_field = "sig_id";
        obj_id = signal_id_;
    }
    else return false;

    dbhandler.show_items(table, {doc_id_field}, {{"next", "-1"}, {obj_field, obj_id}}, items);
    assert (items.size() <= 1);
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    QString content = get_content();
    content.replace("\\", "\\\\");
    QVector<QString> fields = {obj_field, "doc_type_id", "content", "prev", "next"};
    QVector<QString> values = {obj_id, get_document_type_id(), content, prev, "-2"};

    if (dbhandler.insert_item(table, fields, values) && \
        dbhandler.show_items(table, {doc_id_field}, {{obj_field, obj_id}, {"next", "-2"}}, items))
    {
        doc_id_ = items[0][0];
        dbhandler.update_items(table, {{doc_id_field, doc_id_}}, {{"next", "-1"}});
        if (prev != "-1") dbhandler.update_items(table, {{doc_id_field, prev}}, {{"next", doc_id_}});
        return true;
    }
    else
    {
        QMessageBox::warning(this, "Add Document", QString("Adding document failed\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}


bool DocumentEditor::edit_document()
{
    QString doc_id_field, obj_field, table;
    if (level_ == DOCUMENT_LEVEL::BLOCK)
    {
        table = "doc_block";
        doc_id_field = "block_doc_id";
    }
    else if (level_ == DOCUMENT_LEVEL::REGISTER)
    {
        table = "doc_register";
        doc_id_field = "register_doc_id";
    }
    else if (level_ == DOCUMENT_LEVEL::SIGNAL)
    {
        table = "doc_signal";
        doc_id_field = "signal_doc_id";
    }
    else return false;

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QString content = get_content();
    content.replace("\\", "\\\\");
    if (dbhandler.update_items(table, {{doc_id_field, doc_id_}}, {{"doc_type_id", get_document_type_id()}, {"content", content}}))
        return true;
    else
    {
        QMessageBox::warning(this, "Edit Document", QString("Editting document failed\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}


QString DocumentEditor::get_content() const
{
    if (ui->stackedWidget->currentIndex() == 0) return ui->textEditText->toPlainText();
    if (ui->stackedWidget->currentIndex() == 1) return ui->lineEditImageCaption->text() + DOC_DELIMITER + ui->lineEditImagePath->text();
    if (ui->stackedWidget->currentIndex() == 2)
    {
        QString content =  ui->lineEditTableCaption->text() + DOC_DELIMITER + ui->lineEditTableRow->text() + DOC_DELIMITER +   ui->lineEditTableColumn->text() + DOC_DELIMITER;
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
            for (int j = 0; j < ui->tableWidget->columnCount(); j++)
            {
                QString text = ui->tableWidget->item(i, j) ? ui->tableWidget->item(i, j)->text() : "";
                content += (text + DOC_DELIMITER);
            }
         return content;
    }
    return "";
}

QString DocumentEditor::get_document_type() const
{
    return document_types_[ui->stackedWidget->currentIndex()];
}

QString DocumentEditor::get_document_type_id() const
{
    return document_type2id_[get_document_type()];
}

QString DocumentEditor::get_doc_id() const
{
    return doc_id_;
}

void DocumentEditor::on_pushButtonPreview_clicked()
{
    show_preview_ = !show_preview_;
    if (show_preview_) ui->pushButtonPreview->setText("Hide Preview");
    else ui->pushButtonPreview->setText("Show Preview");
    ui->framePreview->setVisible(show_preview_);
    //resize(width(), height() + (2 * (int)show_preview_ - 1) * 120);
}

void DocumentEditor::on_tableWidget_cellClicked(int row, int column)
{

}


void DocumentEditor::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* current = ui->tableWidget->item(row, column);
    current_text_ = current ? current->text() : "";
}

void DocumentEditor::on_tableWidget_cellChanged(int row, int column)
{
    if (row < 0 || column < 0) return;
    ui->webEngineView->setHtml(generate_html_table(), QUrl("file://"));
    //ui->tableWidget->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableWidget->resizeRowsToContents();
}

QString DocumentEditor::generate_html_table()
{
    QString html = html_template;;
    QString table = html_table_template;
    QString table_content;
    int rows = ui->lineEditTableRow->text().toInt();
    int cols = ui->lineEditTableColumn->text().toInt();
    if (rows < 1 or cols < 1) return "";

    // header
    if (ui->lineEditTableCaption->text() != "")
        table_content += ("<caption>" + ui->lineEditTableCaption->text() + "</caption>");

    table_content += "<tr>";
    for (int j = 0; j < cols; j++)
        table_content += ("<th>" + (ui->tableWidget->item(0, j) ? ui->tableWidget->item(0, j)->text() : "") + "</th>");
    table_content += "</tr>";
    if (rows > 1)
    {
        for (int i = 1; i < rows; i++)
        {
            table_content += "<tr>";
            for (int j = 0; j < cols; j ++)
                table_content += ("<td>" + (ui->tableWidget->item(i, j) ? ui->tableWidget->item(i, j)->text() : "") + "</td>");
            table_content += "</tr>";
        }
    }
    table.replace("{TABLE}", table_content);
    html.replace("{CONTENT}", table).replace("{MATHJAX_ROOT}", mathjax_root);
    return html;
}

bool DocumentEditor::validate_table_cell_text(const QString& text)
{
    return !text.contains(DOC_DELIMITER);
}


void DocumentEditor::on_textEditText_textChanged()
{
    QString content = ui->textEditText->toPlainText();
    QString html = html_template;
    html.replace("{CONTENT}", content).replace("{MATHJAX_ROOT}", mathjax_root);
    ui->webEngineView->setHtml(html, QUrl("file://"));
}


void DocumentEditor::on_buttonBox_rejected()
{
    clear();
    setVisible(false);
}

void DocumentEditor::on_buttonBox_accepted()
{
    if (sanity_check())
    {
        if ((mode_ == DIALOG_MODE::ADD && add_document()) || (mode_ == DIALOG_MODE::EDIT && edit_document()))
        {
            setVisible(false);
            if (mode_ == DIALOG_MODE::ADD) emit(document_added());
            else if (mode_ == DIALOG_MODE::EDIT) emit(document_edited());
        }
    }
}

void DocumentEditor::clear()
{
    ui->comboBoxDocType->setCurrentIndex(0);
    for (QString* pid : {&doc_id_, &block_id_, &register_id_, &signal_id_}) pid->clear();

    ui->textEditText->clear();
    for (QLineEdit* edit: {ui->lineEditImagePath, ui->lineEditImageCaption, ui->lineEditTableCaption})
        edit->clear();

    ui->tableWidget->setRowCount(0);
    ui->lineEditTableRow->setText("3");
    ui->lineEditTableColumn->setText("3");
    on_lineEditTableRow_editingFinished();
    on_lineEditTableColumn_editingFinished();
    ui->webEngineView->setHtml("", QUrl("file://"));
}


void DocumentEditor::on_lineEditTableCaption_editingFinished()
{
    on_tableWidget_cellChanged(0, 0);
}


void DocumentEditor::setEnabled(bool enabled)
{
    QVector<QWidget*> widgets = {ui->comboBoxDocType, ui->textEditText,
                                ui->lineEditImagePath, ui->lineEditImageCaption, ui->pushButtonSelectImage,
                                ui->tableWidget, ui->lineEditTableRow, ui->lineEditTableColumn, ui->lineEditTableCaption};
    for (QWidget* w : widgets) w->setEnabled(enabled);
}
