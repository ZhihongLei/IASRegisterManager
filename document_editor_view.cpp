#include "document_editor_view.h"
#include "ui_document_editor_view.h"
#include "database_handler.h"
#include "global_variables.h"
#include "document_generator.h"
#include "data_utils.h"
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QCompleter>
#include <QDir>
#include <QSettings>

DocumentEditorView::DocumentEditorView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DocumentEditorView)
{
    ui->setupUi(this);
    actionEdit_ = new QAction("Edit", this);
    actionRemove_ = new QAction("Remove", this);
    actionAdd_ = new QAction("Add", this);
    actionCopy_ = new QAction("Copy", this);
    actionPaste_ = new QAction("Paste", this);
    actionRefresh_ = new QAction("Refresh", this);

    context_menu_ = new QMenu(this);

    context_menu_->addAction(actionEdit_);
    context_menu_->addAction(actionRemove_);
    context_menu_->addAction(actionAdd_);
    context_menu_->addSeparator();
    context_menu_->addAction(actionCopy_);
    context_menu_->addAction(actionPaste_);
    context_menu_->addSeparator();
    context_menu_->addAction(actionRefresh_);

    actionPaste_->setEnabled(false);

    ui->tableDoc->setColumnHidden(0, true);
    ui->tableDoc->setColumnHidden(1, true);
    ui->documentEditor->setVisible(false);
    ui->stackedWidgetDoc->setCurrentIndex(0);
    ui->splitterDoc->setCollapsible(0, false);
    ui->splitterDoc->setCollapsible(1, false);

    connect(ui->documentEditor, SIGNAL(document_edited()), this, SLOT(on_document_edited()));
    connect(ui->documentEditor, SIGNAL(document_added()), this, SLOT(on_document_added()));

    connect(actionAdd_, SIGNAL(triggered()), this, SLOT(on_actionAdd_triggered()));
    connect(actionEdit_, SIGNAL(triggered()), this, SLOT(on_actionEdit_triggered()));
    connect(actionRemove_, SIGNAL(triggered()), this, SLOT(on_actionRemove_triggered()));
    connect(actionCopy_, SIGNAL(triggered()), this, SLOT(on_actionCopy_triggered()));
    connect(actionPaste_, SIGNAL(triggered()), this, SLOT(on_actionPaste_triggered()));
    connect(actionRefresh_, SIGNAL(triggered()), this, SLOT(on_actionRefresh_triggered()));

    ui->tableDoc->horizontalHeader()->setVisible(true);
    level_ = LEVEL::CHIP;
}

DocumentEditorView::~DocumentEditorView()
{
    delete ui;
}

void DocumentEditorView::login(const QString &username, const QString &user_id)
{
    username_ = username;
    user_id_ = user_id;
}

void DocumentEditorView::open_chip(const QString &chip, const QString &chip_id, int register_width, int address_width, bool msb_first)
{
    chip_name_ = chip;
    chip_id_ = chip_id;
    register_width_ = register_width;
    address_width_ = address_width;
    msb_first_ = msb_first;
    set_doc_level(LEVEL::CHIP);
    set_chip_id(chip_id);
}

void DocumentEditorView::close_chip()
{
    ui->webDocViewer->setHtml("", QUrl("file://"));
    ui->tableDoc->setRowCount(0);
    QVector<QWidget*> widgets = {ui->pushButtonAddDoc, ui->pushButtonRemoveDoc};
    for (QWidget* widget : widgets) widget->setEnabled(false);
    ui->stackedWidgetDoc->setCurrentIndex(0);
    ui->documentEditor->clear();
}

void DocumentEditorView::set_authenticator(Authenticator *authenticator)
{
    authenticator_ = authenticator;
}

void DocumentEditorView::set_doc_level(const LEVEL& level)
{
    level_ = level;
    bool enabled = level_  == LEVEL::CHIP ? authenticator_->can_fully_access_all_blocks() : authenticator_->can_edit_document();
    ui->documentEditor->set_doc_level(level_);
    set_install_event_filter(enabled);
    ui->pushButtonAddDoc->setEnabled(enabled);
    ui->pushButtonRemoveDoc->setEnabled(false);
}

void DocumentEditorView::set_view(const DOCUMENT_VIEW& view)
{
    if (view == DOCUMENT_VIEW::PREVIEW) ui->stackedWidgetDoc->setCurrentIndex(0);
    else if(view == DOCUMENT_VIEW::EDITOR_VIEW) ui->stackedWidgetDoc->setCurrentIndex(1);
}

void DocumentEditorView::set_chip_id(const QString &chip_id)
{
    chip_id_ = chip_id;
    ui->documentEditor->set_chip_id(chip_id);
}

void DocumentEditorView::set_block_id(const QString& block_id)
{
    block_id_ = block_id;
    ui->documentEditor->set_block_id(block_id);
}
void DocumentEditorView::set_register_id(const QString& register_id)
{
    register_id_ = register_id;
    ui->documentEditor->set_register_id(register_id);
}
void DocumentEditorView::set_signal_id(const QString& signal_id)
{
    signal_id_ = signal_id;
    ui->documentEditor->set_signal_id(signal_id);
}

void DocumentEditorView::set_completer(QCompleter *c)
{
    ui->documentEditor->set_completer(c);
}

void DocumentEditorView::display_documents()
{
    ui->stackedWidgetDoc->setCurrentIndex(1);
    ui->tableDoc->setRowCount(0);

    QVector<QVector<QString> > items;
    bool success = false;
    if (level_ == LEVEL::SIGNAL)
        success = DataBaseHandler::show_items_inner_join({"doc_signal.signal_doc_id", "def_doc_type.doc_type", "doc_signal.content", "doc_signal.doc_type_id", "doc_signal.prev", "doc_signal.next"},
                                        {{{"doc_signal", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_signal.sig_id", signal_id_}});
    else if (level_ == LEVEL::REGISTER)
        success = DataBaseHandler::show_items_inner_join({"doc_register.register_doc_id", "def_doc_type.doc_type", "doc_register.content", "doc_register.doc_type_id", "doc_register.prev", "doc_register.next"},
                                        {{{"doc_register", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_register.reg_id", register_id_}});
    else if (level_ == LEVEL::BLOCK)
        success = DataBaseHandler::show_items_inner_join({"doc_block.block_doc_id", "def_doc_type.doc_type", "doc_block.content", "doc_block.doc_type_id", "doc_block.prev", "doc_block.next"},
                                    {{{"doc_block", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_block.block_id", block_id_}});
    else if (level_ == LEVEL::CHIP)
        success = DataBaseHandler::show_items_inner_join({"doc_chip.chip_doc_id", "def_doc_type.doc_type", "doc_chip.content", "doc_chip.doc_type_id", "doc_chip.prev", "doc_chip.next"},
                                    {{{"doc_chip", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_chip.chip_id", chip_id_}});

    if (!success)
    {
        QMessageBox::warning(this, "Document Editor", "Unable to read documents from database.\nPlease try again.");
        return;
    }
    items = sort_doubly_linked_list(items);

    for (const auto& item : items)
    {
        int row = ui->tableDoc->rowCount();
        ui->tableDoc->insertRow(row);
        ui->tableDoc->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableDoc->setItem(row, 1, new QTableWidgetItem(item[3]));
        ui->tableDoc->setItem(row, 2, new QTableWidgetItem(item[1]));
        ui->tableDoc->setItem(row, 3, new QTableWidgetItem(item[2]));
    }
    ui->tableDoc->resizeRowsToContents();
    ui->documentEditor->clear_content();
    ui->documentEditor->setVisible(false);
}


void DocumentEditorView::display_overall_documents()
{
    ui->stackedWidgetDoc->setCurrentIndex(0);
    ui->webDocViewer->setHtml("", QUrl("file://"));
    QSettings chip_setttings("chip_setttings.ini", QSettings::IniFormat);
    chip_setttings.beginGroup(chip_id_);
    QString img_pos = chip_setttings.value("image_caption_position").toString(),
            tab_pos = chip_setttings.value("table_caption_position").toString(),
            show_paged_reg = chip_setttings.value("show_paged_register").toString();

    QString html = DocumentGenerator(chip_id_,
                                     chip_name_,
                                     address_width_,
                                     register_width_,
                                     msb_first_,
                                     user_id_,
                                     authenticator_,
                                     img_pos == "top" ? DocumentGenerator::TOP : DocumentGenerator::BOTTOM,
                                     tab_pos == "top" ? DocumentGenerator::TOP : DocumentGenerator::BOTTOM,
                                     show_paged_reg).generate_html_document();
    ui->webDocViewer->setHtml(html, QUrl("file://"));
}

void DocumentEditorView::on_tableDoc_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableDoc->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_edit_document());
    actionAdd_->setEnabled(authenticator_->can_edit_document());
    actionRemove_->setEnabled(current && authenticator_->can_edit_document());
    actionCopy_->setEnabled(current);
    actionPaste_->setEnabled(authenticator_->can_edit_document() && copied_ != "");
    context_menu_->popup(ui->tableDoc->viewport()->mapToGlobal(pos));
}

void DocumentEditorView::on_actionAdd_triggered()
{
    if (ui->tableDoc->hasFocus()) on_pushButtonAddDoc_clicked();
}

void DocumentEditorView::on_actionEdit_triggered()
{
    if (ui->tableDoc->hasFocus()) on_tableDoc_cellDoubleClicked(ui->tableDoc->currentRow(), 0);
}

void DocumentEditorView::on_actionRemove_triggered()
{
    if (ui->tableDoc->hasFocus()) on_pushButtonRemoveDoc_clicked();
}

void DocumentEditorView::on_actionCopy_triggered()
{
    if (ui->tableDoc->hasFocus() && ui->tableDoc->currentRow() >= 0)
    {
        int row = ui->tableDoc->currentRow();
        copied_ = ui->tableDoc->item(row, 1)->text() + DOC_DELIMITER + ui->tableDoc->item(row, 2)->text() +
                DOC_DELIMITER + ui->tableDoc->item(row, 3)->text();
    }
}

void DocumentEditorView::on_actionPaste_triggered()
{
    on_pushButtonAddDoc_clicked();
    ui->documentEditor->setVisible(false);
    QStringList ss = copied_.split(DOC_DELIMITER);
    QString doc_type_id = ss[0],
            doc_type = ss[1],
            content = copied_.right(copied_.size() - 2 * DOC_DELIMITER.size() - doc_type.size() - doc_type_id.size());

    ui->documentEditor->set_content("", doc_type, content);
    if (ui->documentEditor->add_document()) on_document_added();
    copied_ = "";
}

void DocumentEditorView::on_actionRefresh_triggered()
{
    if (ui->tableDoc->hasFocus()) display_documents();
}

void DocumentEditorView::on_stackedWidgetDoc_currentChanged(int index)
{
    if (index == 1) ui->pushButtonRemoveDoc->setEnabled(false);
}

void DocumentEditorView::on_pushButtonAddDoc_clicked()
{
    ui->documentEditor->clear_content();
    ui->documentEditor->set_mode(DIALOG_MODE::ADD);
    ui->documentEditor->setVisible(true);
}

void DocumentEditorView::on_pushButtonRemoveDoc_clicked()
{
    int row = ui->tableDoc->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Document",
                             "Are you sure you want to remove this document?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    QString doc_id = ui->tableDoc->item(row, 0)->text();
    QString table, id_field;


    if (level_ == LEVEL::REGISTER)   // register
    {
        table = "doc_register";
        id_field = "register_doc_id";
    }
    else if (level_ == LEVEL::SIGNAL)  // signal
    {
        table = "doc_signal";
        id_field = "signal_doc_id";
    }
    else if (level_ == LEVEL::BLOCK)
    {
        table = "doc_block";
        id_field = "block_doc_id";
    }
    else if (level_ == LEVEL::CHIP)
    {
        table = "doc_chip";
        id_field = "chip_doc_id";
    }

    QVector<QVector<QString> > items;
    bool success = true;
    if (DataBaseHandler::show_items(table, {"prev", "next"}, id_field, doc_id, items) && DataBaseHandler::delete_items(table, id_field, doc_id) )
    {
        QString prev = items[0][0], next = items[0][1];
        if (prev != "-1") success = success && DataBaseHandler::update_items(table, {{id_field, prev}}, {{"next", next}});
        if (next != "-1") success = success && DataBaseHandler::update_items(table, {{id_field, next}}, {{"prev", prev}});
        if (success)
        {
            DataBaseHandler::commit();
            ui->tableDoc->removeRow(row);
            return;
        }
    }
    DataBaseHandler::rollback();
    QMessageBox::warning(this, "Remove Document", "Unable to remove document.\nError message: " + DataBaseHandler::get_error_message());
}



void DocumentEditorView::on_tableDoc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    bool enabled = level_ == LEVEL::CHIP ? authenticator_->can_fully_access_all_blocks() : authenticator_->can_edit_document();
    ui->pushButtonRemoveDoc->setEnabled(enabled && currentRow>=0);
}

void DocumentEditorView::on_tableDoc_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    ui->documentEditor->clear_content();
    ui->documentEditor->set_mode(DIALOG_MODE::EDIT);
    ui->documentEditor->setEnabled(level_ == LEVEL::CHIP ? authenticator_ ->can_fully_access_all_blocks() : authenticator_->can_edit_document());
    ui->documentEditor->setVisible(true);

    QString doc_id = ui->tableDoc->item(row, 0)->text(),
            type = ui->tableDoc->item(row, 2)->text(),
            content = ui->tableDoc->item(row, 3)->text();
    ui->documentEditor->set_content(doc_id, type, content);
}

void DocumentEditorView::on_document_added()
{
    int row = ui->tableDoc->rowCount();
    ui->tableDoc->insertRow(row);
    ui->tableDoc->setItem(row, 0, new QTableWidgetItem(ui->documentEditor->get_doc_id()));
    ui->tableDoc->setItem(row, 1, new QTableWidgetItem(ui->documentEditor->get_document_type_id()));
    ui->tableDoc->setItem(row, 2, new QTableWidgetItem(ui->documentEditor->get_document_type()));
    ui->tableDoc->setItem(row, 3, new QTableWidgetItem(ui->documentEditor->get_content()));
    ui->tableDoc->resizeRowToContents(row);
}

void DocumentEditorView::on_document_edited()
{
    int row = ui->tableDoc->currentRow();
    ui->tableDoc->item(row, 1)->setText(ui->documentEditor->get_document_type_id());
    ui->tableDoc->item(row, 2)->setText(ui->documentEditor->get_document_type());
    ui->tableDoc->item(row, 3)->setText(ui->documentEditor->get_content());
    ui->tableDoc->resizeRowToContents(row);
}

void DocumentEditorView::set_install_event_filter(bool enabled)
{
    if (enabled) ui->tableDoc->viewport()->installEventFilter(this);
    else ui->tableDoc->viewport()->removeEventFilter(this);
    ui->tableDoc->setDragEnabled(enabled);
    ui->tableDoc->setAcceptDrops(enabled);
    ui->tableDoc->setDragDropMode(enabled ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
}

void DocumentEditorView::table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row)
{
    if (from_row < 0 || to_row < 0 || to_row >= table->rowCount()) return;

    QVector<QString> item;
    QString from = table->item(from_row, 0)->text();
    QString to = table->item(to_row, 0)->text();
    if (!DataBaseHandler::show_one_item(table_name, item, {"prev", "next"}, key, from)) return;
    QString from_prev = item[0], from_next = item[1];

    item.clear();
    if (!DataBaseHandler::show_one_item(table_name, item, {"prev", "next"}, key, to)) return;
    QString to_prev = item[0], to_next = item[1];

    bool success = true;
    if (from_row >= to_row)
    {
        if (from != "-1") success = success && DataBaseHandler::update_items(table_name, key, from, {{"prev", to_prev}, {"next", to}});
        if (to_prev != "-1") success = success && DataBaseHandler::update_items(table_name, key, to_prev, {{"next", from}});
        if (to != "-1") success = success && DataBaseHandler::update_items(table_name, key, to, {{"prev", from}});
    }
    else {
        if (from != "-1") success = success && DataBaseHandler::update_items(table_name, key, from, {{"prev", to}, {"next", to_next}});
        if (to != "-1") success = success && DataBaseHandler::update_items(table_name, key, to, {{"next", from}});
        if (to_next != "-1") success = success && DataBaseHandler::update_items(table_name, key, to_next, {{"prev", from}});
    }
    if (from_next != "-1") success = success && DataBaseHandler::update_items(table_name, key, from_next, {{"prev", from_prev}});
    if (from_prev != "-1") success = success && DataBaseHandler::update_items(table_name, key, from_prev, {{"next", from_next}});
    if (success) DataBaseHandler::commit();
    else
    {
        DataBaseHandler::rollback();
        return;
    }

    QVector<QString> from_items;
    for (int col = 0; col < table->columnCount(); col++) from_items.push_back(table->item(from_row, col)->text());
    if (to_row < from_row)
        for (int row = from_row; row > to_row; row--)
        {
            for (int col = 0; col < table->columnCount(); col++) table->item(row, col)->setText(table->item(row-1, col)->text());
        }
    else {
        for (int row = from_row; row < to_row; row++)
        {
            for (int col = 0; col < table->columnCount(); col++) table->item(row, col)->setText(table->item(row+1, col)->text());
        }
    }
    for (int col = 0; col < table->columnCount(); col++) table->item(to_row, col)->setText(from_items[col]);
}


bool DocumentEditorView::eventFilter(QObject *obj, QEvent *eve)
{
    if (obj == ui->tableDoc->viewport())
    {
        if (eve->type() == QEvent::Drop)
        {
            const QMimeData *mime = (static_cast<QDropEvent*>(eve))->mimeData();
            QByteArray encodedata = mime->data("application/x-qabstractitemmodeldatalist");
            if (encodedata.isEmpty()) return false;
            QDataStream stream(&encodedata, QIODevice::ReadOnly);
            while (!stream.atEnd())
            {
                int row, col;
                QMap<int,  QVariant> roleDataMap;
                stream >> row >> col >> roleDataMap;
                QTableWidgetItem* pDropItem = nullptr;
                if (obj == ui->tableDoc->viewport()) pDropItem = ui->tableDoc->itemAt((static_cast<QDropEvent*>(eve))->pos());
                if (!pDropItem) return true;
                if (pDropItem->row() == row) return true;
                if (obj == ui->tableDoc->viewport())
                {

                    if (level_ == LEVEL::REGISTER)   // register
                        table_drop_event_handling(ui->tableDoc, "doc_register", "register_doc_id", row, pDropItem->row());
                    else if (level_ == LEVEL::SIGNAL)  // signal
                        table_drop_event_handling(ui->tableDoc, "doc_signal", "signal_doc_id", row, pDropItem->row());
                    else if (level_ == LEVEL::BLOCK) // block
                        table_drop_event_handling(ui->tableDoc, "doc_block", "block_doc_id", row, pDropItem->row());
                    else if (level_ == LEVEL::CHIP) // chip
                        table_drop_event_handling(ui->tableDoc, "doc_chip", "chip_doc_id", row, pDropItem->row());
                    ui->tableDoc->setCurrentCell(pDropItem->row(), 0);
                    ui->tableDoc->resizeRowsToContents();
                }
                return true;
            }
        } else return QWidget::eventFilter(obj, eve);
    }
    return QWidget::eventFilter(obj,eve);
}
