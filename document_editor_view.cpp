#include "document_editor_view.h"
#include "ui_document_editor_view.h"
#include "database_handler.h"
#include "global_variables.h"
#include "data_utils.h"
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>

DocumentEditorView::DocumentEditorView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DocumentEditorView)
{
    ui->setupUi(this);
    actionEdit_ = new QAction("Edit", this);
    actionRemove_ = new QAction("Remove", this);
    actionAdd_ = new QAction("Add", this);
    actionRefresh_ = new QAction("Refresh", this);

    context_menu_ = new QMenu(this);

    context_menu_->addAction(actionEdit_);
    context_menu_->addAction(actionRemove_);
    context_menu_->addAction(actionAdd_);
    context_menu_->addSeparator();
    context_menu_->addAction(actionRefresh_);

    ui->tableDoc->setColumnHidden(0, true);
    ui->documentEditor->setVisible(false);
    ui->stackedWidgetDoc->setCurrentIndex(0);
    ui->splitterDoc->setCollapsible(0, false);
    ui->splitterDoc->setCollapsible(1, false);

    connect(ui->documentEditor, SIGNAL(document_edited()), this, SLOT(on_document_edited()));
    connect(ui->documentEditor, SIGNAL(document_added()), this, SLOT(on_document_added()));

    connect(actionAdd_, SIGNAL(triggered()), this, SLOT(on_actionAdd_triggered()));
    connect(actionEdit_, SIGNAL(triggered()), this, SLOT(on_actionEdit_triggered()));
    connect(actionRemove_, SIGNAL(triggered()), this, SLOT(on_actionRemove_triggered()));
    connect(actionRefresh_, SIGNAL(triggered()), this, SLOT(on_actionRefresh_triggered()));
}

DocumentEditorView::~DocumentEditorView()
{
    delete ui;
}


void DocumentEditorView::on_tableDoc_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableDoc->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_edit_document());
    actionAdd_->setEnabled(authenticator_->can_edit_document());
    actionRemove_->setEnabled(current && authenticator_->can_edit_document());
    context_menu_->popup(ui->tableDoc->viewport()->mapToGlobal(pos));
}

void DocumentEditorView::on_actionAdd_triggered()
{
    if (ui->tableDoc->hasFocus()) on_pushButtonAddDoc_clicked();
}

void DocumentEditorView::on_actionEdit_triggered()
{
    if (ui->tableDoc->hasFocus()); //on_treeWidgetDoc_itemDoubleClicked(ui->treeWidgetDoc->currentItem(), 0);
}

void DocumentEditorView::on_actionRemove_triggered()
{
    if (ui->tableDoc->hasFocus()) on_pushButtonRemoveDoc_clicked();
}


void DocumentEditorView::on_actionRefresh_triggered()
{
    if (ui->tableDoc->hasFocus()) display_documents();
}

void DocumentEditorView::set_authenticator(Authenticator *authenticator)
{
    authenticator_ = authenticator;
}

void DocumentEditorView::set_doc_level(const LEVEL& level)
{
    level_ = level;
    ui->stackedWidgetDoc->setCurrentIndex(level == LEVEL::CHIP ? 0 : 1);
}

void DocumentEditorView::set_user_id(const QString& user_id)
{
    user_id_ = user_id;
}

void DocumentEditorView::set_chip_id(const QString& chip_id)
{
    chip_id_ = chip_id;
}
void DocumentEditorView::set_block_id(const QString& block_id)
{
    block_id_ = block_id;
}
void DocumentEditorView::set_register_id(const QString& register_id)
{
    register_id_ = register_id;
}
void DocumentEditorView::set_signal_id(const QString& signal_id)
{
    signal_id_ = signal_id;
}

void DocumentEditorView::set_address_width(int width)
{
    address_width_ = width;
}

void DocumentEditorView::set_register_width(int width)
{
    register_width_ = width;
}

void DocumentEditorView::set_msb_first(bool msb_first)
{
    msb_first_ = msb_first;
}

void DocumentEditorView::set_install_event_filter()
{
    bool enabled = authenticator_->can_edit_document();
    if (enabled) ui->tableDoc->viewport()->installEventFilter(this);
    else ui->tableDoc->viewport()->removeEventFilter(this);
    ui->tableDoc->setDragEnabled(enabled);
    ui->tableDoc->setAcceptDrops(enabled);
    ui->tableDoc->setDragDropMode(enabled ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
}

void DocumentEditorView::table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row)
{
    if (from_row < 0 || to_row < 0 || to_row >= table->rowCount()) return;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    QString from = table->item(from_row, 0)->text();
    QString to = table->item(to_row, 0)->text();
    dbhandler.show_one_item(table_name, item, {"prev", "next"}, key, from);
    QString from_prev = item[0], from_next = item[1];

    item.clear();
    dbhandler.show_one_item(table_name, item, {"prev", "next"}, key, to);
    QString to_prev = item[0], to_next = item[1];

    if (from_row >= to_row)
    {
        if (from != "-1") dbhandler.update_items(table_name, key, from, {{"prev", to_prev}, {"next", to}});
        if (to_prev != "-1") dbhandler.update_items(table_name, key, to_prev, {{"next", from}});
        if (to != "-1") dbhandler.update_items(table_name, key, to, {{"prev", from}});
    }
    else {
        if (from != "-1") dbhandler.update_items(table_name, key, from, {{"prev", to}, {"next", to_next}});
        if (to != "-1") dbhandler.update_items(table_name, key, to, {{"next", from}});
        if (to_next != "-1") dbhandler.update_items(table_name, key, to_next, {{"prev", from}});
    }
    if (from_next != "-1") dbhandler.update_items(table_name, key, from_next, {{"prev", from_prev}});
    if (from_prev != "-1") dbhandler.update_items(table_name, key, from_prev, {{"next", from_next}});


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
                    ui->tableDoc->setCurrentCell(pDropItem->row(), 0);
                    ui->tableDoc->resizeRowsToContents();
                }
                return true;
            }
        } else return QWidget::eventFilter(obj, eve);
    }
    return QWidget::eventFilter(obj,eve);
}


void DocumentEditorView::display_documents()
{
    if (level_ == LEVEL::CHIP)
    {
        display_overall_documents();
        return;
    }
    ui->tableDoc->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    if (level_ == LEVEL::SIGNAL)
        dbhandler.show_items_inner_join({"doc_signal.signal_doc_id", "def_doc_type.doc_type", "doc_signal.content", "doc_signal.doc_type_id", "doc_signal.prev", "doc_signal.next"},
                                        {{{"doc_signal", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_signal.sig_id", signal_id_}});
    else if (level_ == LEVEL::REGISTER)
        dbhandler.show_items_inner_join({"doc_register.register_doc_id", "def_doc_type.doc_type", "doc_register.content", "doc_register.doc_type_id", "doc_register.prev", "doc_register.next"},
                                        {{{"doc_register", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_register.reg_id", register_id_}});
    else if (level_ == LEVEL::BLOCK)
        dbhandler.show_items_inner_join({"doc_block.block_doc_id", "def_doc_type.doc_type", "doc_block.content", "doc_block.doc_type_id", "doc_block.prev", "doc_block.next"},
                                    {{{"doc_block", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_block.block_id", block_id_}});

    items = sort_doubly_linked_list(items);

    for (const auto& item : items)
    {
        int row = ui->tableDoc->rowCount();
        ui->tableDoc->insertRow(row);
        ui->tableDoc->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableDoc->setItem(row, 1, new QTableWidgetItem(item[1]));
        ui->tableDoc->setItem(row, 2, new QTableWidgetItem(item[2]));
    }
    ui->tableDoc->resizeRowsToContents();
    ui->pushButtonAddDoc->setEnabled(authenticator_->can_edit_document());
    ui->pushButtonRemoveDoc->setEnabled(false);
    ui->documentEditor->clear();
    ui->documentEditor->setVisible(false);
}


void DocumentEditorView::display_overall_documents()
{
    ui->documentEditor->set_mode(DIALOG_MODE::EDIT);
    ui->webDocOverview->setHtml("", QUrl("file://"));

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    dbhandler.show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id");

    items = sort_doubly_linked_list(items);

    QString html_content, table_of_content;
    for (const auto& item : items)
    {
        QString block_id = item[0],
                block_name = item[1],
                block_abbr = item[2];
        quint64 block_start_address = item[3].toULongLong(nullptr, 16);

        REGISTER_NAMING.update_key("{BLOCK_NAME}", block_name);
        REGISTER_NAMING.update_key("{BLOCK_ABBR}", block_abbr);
        SIGNAL_NAMING.update_key("{BLOCK_NAME}", block_name);
        SIGNAL_NAMING.update_key("{BLOCK_ABBR}", block_abbr);

        DataBaseHandler dbhandler(gDBHost, gDatabase);
        QVector<QVector<QString> > items;
        dbhandler.show_items_inner_join({"doc_block.block_doc_id", "def_doc_type.doc_type", "doc_block.content", "doc_block.doc_type_id", "doc_block.prev", "doc_block.next"},
                                        {{{"doc_block", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_block.block_id", block_id}});

        items = sort_doubly_linked_list(items);
        QString block_content = "<h2 id=" + block_name +">"+ block_name +"</h2>\n";
        table_of_content += "<li>\n<a href=#" + block_name + ">" + block_name + "</a>\n</li>\n";
        for (const auto& item : items)
        {
            ui->documentEditor->clear();
            ui->documentEditor->set_content(item[0], item[1], item[2]);
            block_content = block_content + ui->documentEditor->generate_html() + '\n';
        }

        QVector<QVector<QString> > registers;
        dbhandler.show_items("block_register", {"reg_id", "reg_name", "prev", "next"}, "block_id", block_id, registers);
        registers = sort_doubly_linked_list(registers);

        QString reg_bullets;
        for (int i = 0; i < registers.size(); i++)
        {
            const auto& reg = registers[i];
            QString reg_id = reg[0], reg_name = REGISTER_NAMING.get_extended_name(reg[1]);
            QString address = decimal2hex(block_start_address + static_cast<quint64>(i), address_width_);

            block_content = block_content + "<h4 id=" + reg_name + ">" + reg_name +" - " + address + "</h4>\n";
            reg_bullets += "<li>\n<a href=#" + reg_name + ">" + reg_name + "</a>\n</li>\n";

            QVector<QVector<QString> > signal_items;
            QVector<QString> ext_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                               "block_sig_reg_partition_mapping.reg_lsb",
                                               "block_sig_reg_partition_mapping.reg_msb",
                                                "signal_signal.sig_name",
                                               "signal_signal.sig_id",
                                               "block_sig_reg_partition_mapping.sig_lsb",
                                               "block_sig_reg_partition_mapping.sig_msb",
                                                "signal_reg_signal.init_value",
                                                "signal_signal.width",
                                                "signal_signal.add_port"};
            dbhandler.show_items_inner_join(ext_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                         {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, signal_items, {{"block_sig_reg_partition_mapping.reg_id", reg_id}});

            if (msb_first_) qSort(signal_items.begin(), signal_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
            else qSort(signal_items.begin(), signal_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});

            QString reg_table = "<style>\n\
                    table#t_reg, td#t_reg {border: 1px solid black;border-collapse: collapse;}\n\
                    td#t_reg {padding: 2px;text-align: center;}\n\
                    </style>\n";

            QString header, signal_row, value_row;
            for (int j = 0; j < register_width_; j++)
            {
                if (msb_first_) header += "<td id=t_reg width=" + QString::number(100/register_width_) + "%>" + QString::number(register_width_-1-j) + "</td>\n";
                else header += "<td id=t_reg width=" + QString::number(100/register_width_) + "%>" + QString::number(j) + "</td>\n";
            }
            header = "<tr id=t_reg>\n" + header + "</tr>\n";



            if (signal_items.size() == 0) {
                QString span = QString::number(register_width_);
                QString cell = "<td id=t_reg colspan=" +span + ">" + "..." + "</td>\n";
                signal_row = cell;
                for (int j = 0; j < register_width_; j++) value_row += "<td id=t_reg>.</td>\n";
            }
            else {
                for (int j = 0; j < signal_items.size(); j++)
                {
                    const auto &curr = signal_items[j];
                    QString reg_msb = curr[2],
                            reg_lsb = curr[1],
                            sig_name = curr[9]=="1" ? SIGNAL_NAMING.get_extended_name(curr[3]) : curr[3],
                            sig_value = QString::number(curr[7].toLongLong(nullptr, 16), 2);
                    int sig_width = curr[8].toInt();
                    sig_value = QString(sig_width - sig_value.size(), '0') + sig_value;
                    QString span = QString::number(reg_msb.toInt() - reg_lsb.toInt() + 1);
                    signal_row += "<td id=t_reg colspan=" +span + ">" + "<font size=\"2\">" + sig_name +"</font>" + "</td>";

                    for (int bit = 0; bit < sig_width; bit++) value_row += "<td id=t_reg>" + sig_value[msb_first_ ? bit : sig_width - bit - 1] + "</td>\n";
                    if (j < signal_items.size()-1)
                    {
                        const auto &next = signal_items[j+1];
                        int gap = msb_first_ ? reg_lsb.toInt() - next[2].toInt() - 1 : next[1].toInt() - reg_msb.toInt() -1;
                        if (gap > 0)
                        {
                            signal_row += "<td id=t_reg colspan=" + QString::number(gap) + ">" + "..." + "</td>";
                            for (int bit = 0; bit < gap; bit ++) value_row += "<td id=t_reg>.</td>\n";
                        }

                    }
                    if (j == 0)
                    {
                        int gap = msb_first_ ? register_width_ - reg_msb.toInt() -1 : reg_lsb.toInt();
                        if (gap > 0)
                        {
                            signal_row = "<td id=t_reg colspan=" +QString::number(gap) + ">" + "..." + "</td>" + signal_row;
                            for (int bit = 0; bit < gap; bit ++) value_row = "<td id=t_reg>.</td>\n" + value_row;
                        }
                    }
                    if (j == signal_items.size()-1)
                    {
                        int gap = msb_first_ ? reg_lsb.toInt() : register_width_ - reg_msb.toInt() -1;
                        if (gap > 0)
                        {
                            signal_row += "<td id=t_reg colspan=" +QString::number(gap) + ">" + "..." + "</td>";
                            for (int bit = 0; bit < gap; bit ++) value_row += "<td id=t_reg>.</td>\n";
                        }
                    }
                }
            }
            signal_row = "<tr id=t_reg>\n" + signal_row + "</tr>\n";
            value_row = "<tr id=t_reg>\n" + value_row + "</tr>\n";
            reg_table = "<p>\n<table id=t_reg width=90%>\n" + reg_table + header + signal_row + value_row + "</table>\n</p>\n";
            block_content = block_content + reg_table;

            items.clear();
            dbhandler.show_items_inner_join({"doc_register.register_doc_id", "def_doc_type.doc_type", "doc_register.content", "doc_register.doc_type_id", "doc_register.prev", "doc_register.next"},
                                            {{{"doc_register", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                            items, {{"doc_register.reg_id", reg_id}});

            for (const auto& item : items)
            {
                ui->documentEditor->clear();
                ui->documentEditor->set_content(item[0], item[1], item[2]);
                block_content = block_content + ui->documentEditor->generate_html() + '\n';
            }

            QSet<QString> signal_set;
            QVector<QString> signal_ids, signal_names;
            for (const auto& signal_item : signal_items)
            {
                QString sig_id = signal_item[4],
                        sig_name = signal_item[9]=="1" ? SIGNAL_NAMING.get_extended_name(signal_item[3]) : signal_item[3];
                if (signal_set.contains(sig_id)) continue;
                signal_set.insert(sig_id);
                signal_ids.push_back(sig_id);
                signal_names.push_back(sig_name);
            }

            QString bullet_list;
            for (int j = 0; j < signal_ids.size(); j++)
            {
                QString sig_id = signal_ids[j], sig_name = signal_names[j];
                signal_items.clear();
                dbhandler.show_items_inner_join({"doc_signal.signal_doc_id", "def_doc_type.doc_type", "doc_signal.content", "doc_signal.doc_type_id", "doc_signal.prev", "doc_signal.next"},
                                                {{{"doc_signal", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                                signal_items, {{"doc_signal.sig_id", sig_id}});
                signal_items = sort_doubly_linked_list(signal_items);

                QString bullet = sig_name + " ";
                for (const auto& signal_item : signal_items)
                {
                    ui->documentEditor->clear();
                    ui->documentEditor->set_content(signal_item[0], signal_item[1], signal_item[2]);
                    bullet += ui->documentEditor->generate_html() + "<br>\n" ;
                }
                bullet = "<li>\n" + bullet + "</li>\n";
                bullet_list += bullet;
            }
            bullet_list = "<ul>\n" + bullet_list + "</ul>\n";
            block_content += bullet_list;
        }

        reg_bullets = "<ul>\n" + reg_bullets + "</ul>\n";
        table_of_content += reg_bullets;
        html_content = html_content + "<section>\n" + block_content + "</section>\n";
    }
    table_of_content = "<h2>Table of Content</h2>\n<ol>\n" + table_of_content + "</ol>\n";
    html_content = table_of_content + html_content;
    QString html = html_template;
    html.replace("{HTML}", html_content).replace("{MATHJAX_ROOT}", mathjax_root);
    ui->webDocOverview->setHtml(html, QUrl("file://"));
}

void DocumentEditorView::close_chip()
{
    ui->webDocOverview->setHtml("", QUrl("file://"));
    ui->tableDoc->setRowCount(0);
    QVector<QWidget*> widgets = {ui->pushButtonAddDoc, ui->pushButtonRemoveDoc};
    for (QWidget* widget : widgets) widget->setEnabled(false);
    ui->stackedWidgetDoc->setCurrentIndex(0);
}


void DocumentEditorView::on_pushButtonAddDoc_clicked()
{
    ui->documentEditor->clear();
    if (level_ == LEVEL::BLOCK)
        ui->documentEditor->set_block_id(block_id_);
    else if (level_ == LEVEL::REGISTER)
        ui->documentEditor->set_register_id(register_id_);
    else if (level_ == LEVEL::SIGNAL)
        ui->documentEditor->set_signal_id(signal_id_);
    ui->documentEditor->set_doc_level(level_);
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
    DataBaseHandler dbhandler(gDBHost, gDatabase);

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

    QVector<QVector<QString> > items;
    if (dbhandler.show_items(table, {"prev", "next"}, id_field, doc_id, items) && dbhandler.delete_items(table, id_field, doc_id) )
    {
        QString prev = items[0][0], next = items[0][1];
        if (prev != "-1") dbhandler.update_items(table, {{id_field, prev}}, {{"next", next}});
        if (next != "-1") dbhandler.update_items(table, {{id_field, next}}, {{"prev", prev}});
        ui->tableDoc->removeRow(row);
    }
}



void DocumentEditorView::on_tableDoc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (ui->stackedWidgetDoc->currentIndex() == 0 || currentRow < 0) return;
    ui->pushButtonRemoveDoc->setEnabled(authenticator_->can_edit_document() && currentRow>=0);
}

void DocumentEditorView::on_tableDoc_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    ui->documentEditor->clear();
    ui->documentEditor->set_doc_level(level_);
    ui->documentEditor->set_mode(DIALOG_MODE::EDIT);
    ui->documentEditor->setEnabled(authenticator_->can_edit_document());
    ui->documentEditor->setVisible(true);

    QString doc_id = ui->tableDoc->item(row, 0)->text(),
            type = ui->tableDoc->item(row, 1)->text(),
            content = ui->tableDoc->item(row, 2)->text();
    ui->documentEditor->set_content(doc_id, type, content);
}

void DocumentEditorView::on_stackedWidgetDoc_currentChanged(int index)
{
    if (index == 1) ui->pushButtonRemoveDoc->setEnabled(false);
}


void DocumentEditorView::on_document_added()
{
    int row = ui->tableDoc->rowCount();
    ui->tableDoc->insertRow(row);
    ui->tableDoc->setItem(row, 0, new QTableWidgetItem(ui->documentEditor->get_doc_id()));
    ui->tableDoc->setItem(row, 1, new QTableWidgetItem(ui->documentEditor->get_document_type()));
    ui->tableDoc->setItem(row, 2, new QTableWidgetItem(ui->documentEditor->get_content()));
    ui->tableDoc->resizeRowToContents(row);
}

void DocumentEditorView::on_document_edited()
{
    int row = ui->tableDoc->currentRow();
    ui->tableDoc->item(row, 1)->setText(ui->documentEditor->get_document_type());
    ui->tableDoc->item(row, 2)->setText(ui->documentEditor->get_content());
    ui->tableDoc->resizeRowToContents(row);
}

