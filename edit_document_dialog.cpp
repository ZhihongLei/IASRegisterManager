#include "edit_document_dialog.h"
#include "ui_edit_document_dialog.h"
#include "global_variables.h"
#include "database_handler.h"
#include "document_generator.h"
#include <QFileDialog>
#include <QRegExpValidator>
#include <QMessageBox>

EditDocumentDialog::EditDocumentDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditDocumentDialog)
{
    ui->setupUi(this);
    ui->tableWidget->setItemDelegate(new DelegateLineEdit());
    ui->lineEditImageWidth->setValidator(new QDoubleValidator(0., 1., 2, this));
    level_ = LEVEL::BLOCK;
    mode_ = DIALOG_MODE::ADD;
    bool success = setup_ui();
    on_comboBoxDocType_currentIndexChanged(0);
    if (!success) QMessageBox::warning(this, "Document Editor", "Unable to initialize due to database access issue.\nPlease try again.");
}

EditDocumentDialog::~EditDocumentDialog()
{
    delete ui;
}

bool EditDocumentDialog::setup_ui()
{
    show_preview_ = true;
    ui->stackedWidget->setCurrentIndex(0);
    ui->framePreview->setVisible(show_preview_);
    ui->pushButtonPreview->setVisible(true);
    ui->pushButtonPreview->setText("Show Preview");

    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
    int w = ui->framePreview->width() + ui->stackedWidget->width();
    ui->splitter->setSizes({INT_MAX, INT_MAX});
    ui->lineEditTableRow->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*"), this));
    ui->lineEditTableColumn->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*"), this));

    ui->lineEditTableRow->setText("3");
    ui->lineEditTableColumn->setText("3");
    on_lineEditTableRow_editingFinished();
    on_lineEditTableColumn_editingFinished();

    QVector<QVector<QString> > items;
    bool success = DataBaseHandler::show_items("def_doc_type", {"doc_type_id", "doc_type"}, items, "", "order by doc_type_id");
    for (const auto& item : items)
    {
        ui->comboBoxDocType->addItem(item[1]);
        document_types_.push_back(item[1]);
        document_type2id_[item[1]] = item[0];
        DocumentGenerator::add_doc_type(item[1]);
    }
    //ui->comboBoxDocType->setCurrentIndex(0);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(ui->comboBoxDocType->count() > 0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->verticalSpacer->changeSize(ui->verticalSpacer->sizeHint().width(), ui->verticalSpacer->sizeHint().height(), QSizePolicy::Fixed, QSizePolicy::Fixed);
    return success;
}

void EditDocumentDialog::set_mode(const DIALOG_MODE& mode)
{
    mode_ = mode;
}

void EditDocumentDialog::set_doc_level(const LEVEL &level)
{
    level_ = level;
}

void EditDocumentDialog::set_completer(QCompleter *c)
{
    ui->textEditText->setCompleter(c);
    ui->lineEditTableCaption->setCompleter(c);
    ui->lineEditImageCaption->setCompleter(c);
    static_cast<DelegateLineEdit*>(ui->tableWidget->itemDelegate())->setCompleter(c);
}

void EditDocumentDialog::setEnabled(bool enabled)
{
    QVector<QWidget*> widgets = {ui->comboBoxDocType, ui->textEditText,
                                ui->lineEditImagePath, ui->lineEditImageCaption, ui->pushButtonSelectImage,
                                ui->tableWidget, ui->lineEditTableRow, ui->lineEditTableColumn, ui->lineEditTableCaption};
    for (QWidget* w : widgets) w->setEnabled(enabled);
    enabled_ = enabled;
}

void EditDocumentDialog::set_chip_id(const QString& chip_id)
{
    chip_id_ = chip_id;
}

void EditDocumentDialog::set_block_id(const QString &block_id)
{
    block_id_ = block_id;
}

void EditDocumentDialog::set_register_id(const QString &reg_id)
{
    register_id_ = reg_id;
}

void EditDocumentDialog::set_signal_id(const QString &sig_id)
{
    signal_id_ = sig_id;
}

void EditDocumentDialog::set_content(const QString& doc_id, const QString& doc_type, const QString& content)
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
        ui->lineEditImageWidth->setText(ss[1]);
        ui->lineEditImagePath->setText(ss[2]);
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
        ui->tableWidget->resizeRowsToContents();
    }
}

void EditDocumentDialog::clear_content()
{
    ui->comboBoxDocType->setCurrentIndex(0);

    ui->textEditText->clear();
    ui->lineEditImagePath->clear();
    ui->lineEditImageCaption->clear();
    ui->lineEditTableCaption->clear();

    ui->tableWidget->setRowCount(0);
    ui->lineEditTableRow->setText("3");
    ui->lineEditTableColumn->setText("3");
    on_lineEditTableRow_editingFinished();
    on_lineEditTableColumn_editingFinished();
    ui->webEngineView->setHtml("", QUrl("file://"));
}

void EditDocumentDialog::clear()
{
    clear_content();
    for (QString* pid: {&chip_id_, &block_id_, &register_id_, &signal_id_})
        pid->clear();
    level_ = LEVEL::CHIP;
}

QString EditDocumentDialog::get_document_type() const
{
    return document_types_[ui->stackedWidget->currentIndex()];
}

QString EditDocumentDialog::get_document_type_id() const
{
    return document_type2id_[get_document_type()];
}

QString EditDocumentDialog::get_doc_id() const
{
    return doc_id_;
}

QString EditDocumentDialog::get_content() const
{
    if (ui->stackedWidget->currentIndex() == 0) return ui->textEditText->toPlainText();
    if (ui->stackedWidget->currentIndex() == 1)
        return ui->lineEditImageCaption->text() + DOC_DELIMITER + ui->lineEditImageWidth->text() + DOC_DELIMITER + ui->lineEditImagePath->text();
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

void EditDocumentDialog::on_comboBoxDocType_currentIndexChanged(int index)
{
    if (index < 0) return;
    ui->splitter->setOrientation(index == 1 ? Qt::Vertical : Qt::Horizontal);
    ui->stackedWidget->setMaximumHeight(index == 1 ? 120 : 80000);
    ui->verticalSpacer->changeSize(ui->verticalSpacer->sizeHint().width(),
                                   ui->verticalSpacer->sizeHint().height(),
                                   QSizePolicy::Fixed, !show_preview_ && index == 1 ? QSizePolicy::Expanding: QSizePolicy::Fixed);
    ui->stackedWidget->setCurrentIndex(index);
    ui->webEngineView->setHtml("", QUrl("file://"));
}

void EditDocumentDialog::on_pushButtonPreview_clicked()
{
    show_preview_ = !show_preview_;
    if (show_preview_) ui->pushButtonPreview->setText("Hide Preview");
    else ui->pushButtonPreview->setText("Show Preview");
    ui->framePreview->setVisible(show_preview_);
    int index = ui->comboBoxDocType->currentIndex();
    ui->verticalSpacer->changeSize(ui->verticalSpacer->sizeHint().width(),
                                   ui->verticalSpacer->sizeHint().height(),
                                   QSizePolicy::Fixed, !show_preview_ && index == 1 ? QSizePolicy::Expanding: QSizePolicy::Fixed);
}

void EditDocumentDialog::on_textEditText_textChanged()
{
    QString html = HTML_TEMPLATE;
    QString text = DocumentGenerator::generate_text_html(ui->textEditText->toPlainText());
    html.replace("${HTML}", text).replace("${MATHJAX_ROOT}", MATHJAX_ROOT);
    ui->webEngineView->setHtml(html, QUrl("file://"));
}

void EditDocumentDialog::on_pushButtonSelectImage_clicked()
{
    //DocumentGenerator generator(chip_id_, chip_)
    QString path = QFileDialog::getOpenFileName(this, "Select an image", "", "Image file (*.jpg *.jpeg *.png *.pdf);; All Files (*.* *)");
    if (path == "") return;
    QString caption = ui->lineEditImageCaption->text(),
            width = ui->lineEditImageWidth->text();
    ui->lineEditImagePath->setText(path);
    QString image = DocumentGenerator::generate_image_html(caption, width, path);
    QString html = HTML_TEMPLATE;
    html.replace("${HTML}", image).replace("${MATHJAX_ROOT}", MATHJAX_ROOT);
    ui->webEngineView->setHtml(html, QUrl("file://"));
}

void EditDocumentDialog::on_lineEditImageCaption_editingFinished()
{
    QString caption = ui->lineEditImageCaption->text(),
            width = ui->lineEditImageWidth->text(),
            path = ui->lineEditImagePath->text();
    if (!validate_delimiter(caption))
    {
        ui->lineEditImageCaption->clear();
        QMessageBox::warning(this, "Document Editor", "Caption must not contain delimiter " + DOC_DELIMITER + "!");
        return;
    }
    QString image = DocumentGenerator::generate_image_html(caption, width, path);
    QString html = HTML_TEMPLATE;
    html.replace("${HTML}", image).replace("${MATHJAX_ROOT}", MATHJAX_ROOT);
    ui->webEngineView->setHtml(html, QUrl("file://"));
}

void EditDocumentDialog::on_lineEditImageWidth_editingFinished()
{
    on_lineEditImageCaption_editingFinished();
}

void EditDocumentDialog::on_lineEditTableRow_editingFinished()
{
    ui->tableWidget->setRowCount(ui->lineEditTableRow->text().toInt());
    if (show_preview_) on_tableWidget_cellChanged(0, 0);
}

void EditDocumentDialog::on_lineEditTableColumn_editingFinished()
{
    ui->tableWidget->setColumnCount(ui->lineEditTableColumn->text().toInt());
    QStringList labels;
    for (int i = 0; i < ui->tableWidget->columnCount(); i++) labels << "";
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    if (show_preview_) on_tableWidget_cellChanged(0, 0);
}

void EditDocumentDialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* current = ui->tableWidget->item(row, column);
    current_text_ = current ? current->text() : "";
}

void EditDocumentDialog::on_tableWidget_cellChanged(int row, int column)
{
    QString text = ui->tableWidget->item(row, column) ? ui->tableWidget->item(row, column)->text() : "";
    if (!validate_delimiter(text))
    {
        QMessageBox::warning(this, "Document Editor", "Text must not contain delimiter " + DOC_DELIMITER + "!");
        if (ui->tableWidget->item(row, column)) ui->tableWidget->item(row, column)->setText(current_text_);
        return;
    }
    if (row < 0 || column < 0) return;
    QString html = HTML_TEMPLATE;
    int rows = ui->lineEditTableRow->text().toInt(),
        cols = ui->lineEditTableColumn->text().toInt();
    QVector<QVector<QString> > cells(rows, QVector<QString>(cols, ""));
    if (rows > 0 && cols > 0)
    {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                cells[i][j] = ui->tableWidget->item(i, j) ? ui->tableWidget->item(i, j)->text() : "";
    }
    QString caption = ui->lineEditTableCaption->text();
    QString table = DocumentGenerator::generate_table_html(caption, cells);
    html.replace("${HTML}", table).replace("${MATHJAX_ROOT}", MATHJAX_ROOT);
    ui->webEngineView->setHtml(html, QUrl("file://"));
    ui->tableWidget->resizeRowsToContents();
}

void EditDocumentDialog::on_lineEditTableCaption_editingFinished()
{
    QString caption = ui->lineEditTableCaption->text();
    if (!validate_delimiter(caption))
    {
        ui->lineEditTableCaption->clear();
        QMessageBox::warning(this, "Document Editor", "Caption must not contain delimiter " + DOC_DELIMITER + "!");
        return;
    }
    on_tableWidget_cellChanged(0, 0);
}

void EditDocumentDialog::on_buttonBox_accepted()
{
    if (!enabled_) on_buttonBox_rejected();
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

void EditDocumentDialog::on_buttonBox_rejected()
{
    setVisible(false);
}

bool EditDocumentDialog::sanity_check()
{
    if (ui->stackedWidget->currentIndex() == 1) // imaage
    {
        if (ui->lineEditImagePath->text().contains(" "))
        {
            QMessageBox::warning(this, "Document Editor", "Image path must not contain while space!");
            return false;
        }
        if (ui->lineEditImagePath->text().contains(DOC_DELIMITER))
        {
            QMessageBox::warning(this, "Document Editor", "Image path must not contain delimiter " + DOC_DELIMITER + "!");
            return false;
        }
    }
    return true;
}

bool EditDocumentDialog::add_document()
{

    QVector<QVector<QString> > items;
    QString doc_id_field, obj_field, table, obj_id;
    if (level_ == LEVEL::CHIP)
    {
        table = "doc_chip";
        doc_id_field = "chip_doc_id";
        obj_field = "chip_id";
        obj_id = chip_id_;
    }
    else if (level_ == LEVEL::BLOCK)
    {
        table = "doc_block";
        doc_id_field = "block_doc_id";
        obj_field = "block_id";
        obj_id = block_id_;

    }
    else if (level_ == LEVEL::REGISTER)
    {
        table = "doc_register";
        doc_id_field = "register_doc_id";
        obj_field = "reg_id";
        obj_id = register_id_;
    }
    else if (level_ == LEVEL::SIGNAL)
    {
        table = "doc_signal";
        doc_id_field = "signal_doc_id";
        obj_field = "sig_id";
        obj_id = signal_id_;
    }
    else return false;

    if (!DataBaseHandler::show_items(table, {doc_id_field}, {{"next", "-1"}, {obj_field, obj_id}}, items))
    {
        QMessageBox::warning(this, "Document Editor", "Unable to add document due to database connection issue.\nPlease try again.");
        return false;
    }
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    ;
    QString content = get_content();
    content.replace("\\", "\\\\");

    if (DataBaseHandler::get_next_auto_increment_id(table, doc_id_field, doc_id_) &&
        DataBaseHandler::insert_item(table, {doc_id_field, obj_field, "doc_type_id", "content", "prev", "next"},
                                    {doc_id_, obj_id, get_document_type_id(), content, prev, "-1"}))
    {
        bool success = true;
        if (prev != "-1") success = success && DataBaseHandler::update_items(table, {{doc_id_field, prev}}, {{"next", doc_id_}});
        if (success)
        {
            DataBaseHandler::commit();
            return true;
        }
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Document Editor", "Unable to add document.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditDocumentDialog::edit_document()
{
    QString doc_id_field, obj_field, table;
    if (level_ == LEVEL::CHIP)
    {
        table = "doc_chip";
        doc_id_field = "chip_doc_id";
    }
    else if (level_ == LEVEL::BLOCK)
    {
        table = "doc_block";
        doc_id_field = "block_doc_id";
    }
    else if (level_ == LEVEL::REGISTER)
    {
        table = "doc_register";
        doc_id_field = "register_doc_id";
    }
    else if (level_ == LEVEL::SIGNAL)
    {
        table = "doc_signal";
        doc_id_field = "signal_doc_id";
    }
    else return false;

    QString content = get_content();
    content.replace("\\", "\\\\");  // otherwise back slash will disapear when insert into the db
    if (DataBaseHandler::update_items(table, {{doc_id_field, doc_id_}}, {{"doc_type_id", get_document_type_id()}, {"content", content}}))
    {
        DataBaseHandler::commit();
        return true;
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Document Editor", "Unable to edit document.\nError message: " + DataBaseHandler::get_error_message());
    return false;
}

bool EditDocumentDialog::validate_delimiter(const QString& text)
{
    return !text.contains(DOC_DELIMITER);
}

DelegateLineEdit::DelegateLineEdit():
    c(nullptr)
{

}

DelegateLineEdit::~DelegateLineEdit()
{

}

void DelegateLineEdit::setCompleter(QCompleter *completer)
{
    c = completer;
}

QWidget *DelegateLineEdit::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    LineEdit * line_edit = new LineEdit(parent);
    line_edit->setCompleter(c);
    return line_edit;
}
