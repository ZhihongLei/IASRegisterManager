#include "edit_document_dialog.h"
#include "ui_edit_document_dialog.h"
#include <QFileDialog>
#include <QRegExpValidator>
#include <QRegExp>
#include "global_variables.h"
#include "database_handler.h"
#include <QPushButton>
#include <QMessageBox>

EditDocumentDialog::EditDocumentDialog(DOCUMENT_LEVEL level, const QString& block_id, const QString& reg_id, const QString& sig_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDocumentDialog),
    level_(level),
    block_id_(block_id),
    register_id_(reg_id),
    signal_id_(sig_id),
    doc_id_(""),
    enabled_(true),
    mode_(DIALOG_MODE::ADD)
{
    ui->setupUi(this);
    setup_ui();

    if (level == DOCUMENT_LEVEL::BLOCK) setWindowTitle("Add Block Document");
    if (level == DOCUMENT_LEVEL::SIGNAL) setWindowTitle("Add Signal Document");


}

EditDocumentDialog::EditDocumentDialog(DOCUMENT_LEVEL level, const QString& doc_id, const QString& doc_type, const QString& content, const QString& block_id, const QString& reg_id, const QString& sig_id, bool enabled, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditDocumentDialog),
    level_(level),
    block_id_(block_id),
    register_id_(reg_id),
    signal_id_(sig_id),
    doc_id_(doc_id),
    enabled_(enabled),
    mode_(DIALOG_MODE::EDIT)
{
    ui->setupUi(this);
    setup_ui();
    if (level == DOCUMENT_LEVEL::BLOCK) setWindowTitle("Edit Block Document");
    if (level == DOCUMENT_LEVEL::SIGNAL) setWindowTitle("Edit Signal Document");

    if (doc_type == ui->comboBoxDocType->itemText(0)) // pure text
    {
        ui->comboBoxDocType->setCurrentIndex(0);
        ui->textEditPureText->setText(content);
    }
    else if (doc_type == ui->comboBoxDocType->itemText(1)) // latex
    {
        ui->comboBoxDocType->setCurrentIndex(1);
        ui->textEditLaTeX->setText(content);
    }
    else if (doc_type == ui->comboBoxDocType->itemText(2)) // image
    {
        ui->comboBoxDocType->setCurrentIndex(2);
        QStringList ss = content.split(":");
        ui->lineEditImageCaption->setText(ss[0]);
        ui->lineEditImagePath->setText(ss[1]);
    }
    else if (doc_type == ui->comboBoxDocType->itemText(3)) // table
    {
        ui->comboBoxDocType->setCurrentIndex(3);
        QStringList ss = content.split(":");
        QString title = ss[0], rows = ss[1], cols = ss[2];
        ui->lineEditTableTitle->setText(title);
        ui->lineEditTableRow->setText(rows);
        ui->lineEditTableColumn->setText(cols);
        ss = content.split(DOC_TABLE_DELIMITER);
        for (int i = 0; i < rows.toInt(); i++)
            for (int j = 0; j < cols.toInt(); j++)
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(ss[i*rows.toInt() + j + 1]));
    }
}


EditDocumentDialog::~EditDocumentDialog()
{
    delete ui;
}

void EditDocumentDialog::setup_ui()
{
    show_preview_ = false;
    ui->frameTablePreview->setVisible(show_preview_);
    ui->pushButtonTablePreview->setText("Show Preview");
    ui->frameSignal->setVisible(level_ == DOCUMENT_LEVEL::SIGNAL);
    QVector<QWidget*> widgets = {ui->comboBoxDocType, ui->comboBoxSignal, ui->textEditPureText, ui->textEditLaTeX, ui->webEngineViewLaTeX,
                                ui->lineEditImagePath, ui->lineEditImageCaption, ui->pushButtonSelectImage,
                                ui->tableWidget, ui->lineEditTableRow, ui->lineEditTableColumn, ui->pushButtonTablePreview, ui->lineEditTableTitle, ui->webEngineViewTable};
    for (QWidget* w : widgets) w->setEnabled(enabled_);


    ui->stackedWidget->setCurrentIndex(0);
    //resize(500, 350);
    //ui->splitter->setSizes({INT_MAX, INT_MAX});
    ui->splitterLaTeX->setCollapsible(0, false);
    ui->splitterTable->setCollapsible(0, false);
    ui->splitterTable->setCollapsible(1, false);
    ui->lineEditTableRow->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*"), this));
    ui->lineEditTableColumn->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]*"), this));

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items("def_doc_type", {"doc_type_id", "doc_type"}, items, "", "order by doc_type_id");

    for (const auto& item : items)
    {
        ui->comboBoxDocType->addItem(item[1]);
        document_types_.push_back(item[1]);
        document_type2id_[item[1]] = item[0];
    }
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(enabled_ && ui->comboBoxDocType->count() > 0);

    if (level_ == DOCUMENT_LEVEL::SIGNAL)
    {
        setWindowTitle("Add Signal Document");
        DataBaseHandler dbhandler(gDBHost, gDatabase);
        QVector<QVector<QString> > items;

        bool msb_first_ = true;
        QVector<QString> ext_fields = {"block_reg_partition.reg_part_id",
                                           "block_reg_partition.lsb",
                                           "block_reg_partition.msb",
                                            "signal_signal.sig_name",
                                           "signal_signal.sig_id",
                                           "signal_reg_sig_partition.lsb",
                                           "signal_reg_sig_partition.msb"};
        dbhandler.show_items_inner_join(ext_fields, {{{"block_reg_partition", "reg_sig_part_id"}, {"signal_reg_sig_partition", "reg_sig_part_id"}},
                                                     {{"signal_reg_sig_partition", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                     {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, items, {{"block_reg_partition.reg_id", register_id_}});

        if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
        else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});

        QSet<QString> signal_set;
        for (const auto& signal_item : items)
        {
            QString sig_id = signal_item[4], sig_name = signal_item[3];
            if (signal_set.contains(sig_id)) continue;
            signal_set.insert(sig_id);
            signal_ids_.push_back(sig_id);
            ui->comboBoxSignal->addItem(sig_name);
        }
        if (signal_id_ != "")
        {
            for (int i = 0; i < signal_ids_.size(); i++)
            {
                if (signal_ids_[i] == signal_id_)
                {
                    ui->comboBoxSignal->setCurrentIndex(i);
                    break;
                }
            }
            ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(enabled_ && ui->comboBoxSignal->count() > 0);
            ui->comboBoxSignal->setEnabled(false);
        }
    }
}

void EditDocumentDialog::on_comboBoxDocType_currentIndexChanged(int index)
{
    if (index < 0) return;
    ui->stackedWidget->setCurrentIndex(index);
    if (index == 0) resize(500, 350 + 50 * ui->comboBoxSignal->isVisible());
    if (index == 1) resize(500, 400 + 50 * ui->comboBoxSignal->isVisible());
    if (index == 2) resize(500, 250 + 50 * ui->comboBoxSignal->isVisible());
    if (index == 3) resize(600, 400 + 50 * ui->comboBoxSignal->isVisible() + show_preview_ * 120);
}

void EditDocumentDialog::on_pushButtonSelectImage_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Select an image", "", "JPEG file (*.jpg *.jpeg);;PNG file (*.png);;PDF file (*.pdf);; All Files (*.* *)");
    ui->lineEditImagePath->setText(file);
}

void EditDocumentDialog::on_lineEditTableRow_editingFinished()
{
    ui->tableWidget->setRowCount(ui->lineEditTableRow->text().toInt());
}

void EditDocumentDialog::on_lineEditTableColumn_editingFinished()
{
    ui->tableWidget->setColumnCount(ui->lineEditTableColumn->text().toInt());
    QStringList labels;
    for (int i = 0; i < ui->tableWidget->columnCount(); i++) labels << "";
    ui->tableWidget->setHorizontalHeaderLabels(labels);
}

bool EditDocumentDialog::sanity_check()
{
    // TODO
    return true;
}


void EditDocumentDialog::accept()
{
    if (!enabled_) return QDialog::reject();
    if (sanity_check()) QDialog::accept();
}


bool EditDocumentDialog::add_document()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);

    QString content = get_content();
    QVector<QVector<QString> > items;

    QString doc_id_field, entity_id_field, table, entity_id;
    if (level_ == DOCUMENT_LEVEL::BLOCK)
    {
        table = "doc_block";
        doc_id_field = "block_doc_id";
        entity_id_field = "block_id";
        entity_id = block_id_;
    }
    else if (level_ == DOCUMENT_LEVEL::SIGNAL)
    {
        table = "doc_signal";
        doc_id_field = "signal_doc_id";
        entity_id_field = "sig_id";
        entity_id = signal_ids_[ui->comboBoxSignal->currentIndex()];
    }
    else return false;

    dbhandler.show_items(table, {doc_id_field}, {{"next", "-1"}, {entity_id_field, entity_id}}, items);
    assert (items.size() <= 1);
    QString prev("-1");
    if (items.size() == 1) prev = items[0][0];
    items.clear();

    QVector<QString> fields = {entity_id_field, "doc_type_id", "content", "prev", "next"};
    QVector<QString> values = {entity_id, get_document_type_id(), get_content(), prev, "-2"};

    if (dbhandler.insert_item(table, fields, values) && \
        dbhandler.show_items(table, {doc_id_field}, {{entity_id_field, entity_id}, {"next", "-2"}}, items))
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


bool EditDocumentDialog::edit_document()
{
    QString doc_id_field, entity_id_field, table, entity_id;
    if (level_ == DOCUMENT_LEVEL::BLOCK)
    {
        table = "doc_block";
        doc_id_field = "block_doc_id";
        entity_id_field = "block_id";
        entity_id = block_id_;
    }
    else if (level_ == DOCUMENT_LEVEL::SIGNAL)
    {
        table = "doc_signal";
        doc_id_field = "signal_doc_id";
        entity_id_field = "sig_id";
        entity_id = signal_ids_[ui->comboBoxSignal->currentIndex()];
    }
    else return false;

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.update_items(table, {{doc_id_field, doc_id_}}, {{"doc_type_id", get_document_type_id()}, {"content", get_content()}}))
        return true;
    else
    {
        QMessageBox::warning(this, "Edit Document", QString("Editting document failed\nError message: ") + dbhandler.get_error_message());
        return false;
    }
}


QString EditDocumentDialog::get_content() const
{
    if (ui->stackedWidget->currentIndex() == 0) return ui->textEditPureText->toPlainText();
    if (ui->stackedWidget->currentIndex() == 1) return ui->textEditLaTeX->toPlainText();
    if (ui->stackedWidget->currentIndex() == 2) return ui->lineEditImageCaption->text() + ":" + ui->lineEditImagePath->text();
    if (ui->stackedWidget->currentIndex() == 3)
    {
        QString content =  ui->lineEditTableTitle->text() + ":" + ui->lineEditTableRow->text() + ":" +   ui->lineEditTableColumn->text() + ":" + DOC_TABLE_DELIMITER;
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
            for (int j = 0; j < ui->tableWidget->columnCount(); j++)
            {
                QString text = ui->tableWidget->item(i, j) ? ui->tableWidget->item(i, j)->text() : "";
                content += (text + DOC_TABLE_DELIMITER);
            }
        return content;
    }
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

QString EditDocumentDialog::get_signal_id() const
{
    if (level_ != DOCUMENT_LEVEL::SIGNAL) return "-1";
    return signal_ids_[ui->comboBoxSignal->currentIndex()];
}


void EditDocumentDialog::on_pushButtonTablePreview_clicked()
{
    show_preview_ = !show_preview_;
    if (show_preview_) ui->pushButtonTablePreview->setText("Hide Preview");
    else ui->pushButtonTablePreview->setText("Show Preview");
    ui->frameTablePreview->setVisible(show_preview_);
    resize(width(), height() + (2 * (int)show_preview_ - 1) * 120);
}

void EditDocumentDialog::on_tableWidget_cellClicked(int row, int column)
{
    if (row < 0 || column < 0 || !show_preview_) return;
    QString html;
    QTableWidgetItem* current = ui->tableWidget->item(row, column);
    if (current)
    {
        QString content = current->text();
        html = html_template;
        html.replace("{CONTENT}", content).replace("{MATHJAX_ROOT}", mathjax_root);
    }
    ui->webEngineViewTable->setHtml(html, QUrl("file://"));
}


void EditDocumentDialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* current = ui->tableWidget->item(row, column);
    current_text_ = current ? current->text() : "";
}

void EditDocumentDialog::on_tableWidget_cellChanged(int row, int column)
{
    if (row < 0 || column < 0) return;
    QString html;
    QTableWidgetItem* current = ui->tableWidget->item(row, column);
    if (current)
    {
        QString content = current->text();
        if (!validate_table_cell_text(content))
        {
            QMessageBox::warning(this, "Table Document", "Field should not contain table delimiter " + DOC_TABLE_DELIMITER + "!");
            current->setText(current_text_);
            return;
        }
        html = html_template;
        html.replace("{CONTENT}", content).replace("{MATHJAX_ROOT}", mathjax_root);
    }
    ui->webEngineViewTable->setHtml(html, QUrl("file://"));
}

bool EditDocumentDialog::validate_table_cell_text(const QString& text)
{
    return !text.contains(DOC_TABLE_DELIMITER);
}


void EditDocumentDialog::on_textEditLaTeX_textChanged()
{
    QString content = ui->textEditLaTeX->toPlainText();
    QString html = html_template;
    html.replace("{CONTENT}", content).replace("{MATHJAX_ROOT}", mathjax_root);
    ui->webEngineViewLaTeX->setHtml(html, QUrl("file://"));
}
