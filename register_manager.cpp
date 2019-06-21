#include "register_manager.h"
#include "ui_register_manager.h"
#include "database_handler.h"
#include "create_user_dialog.h"
#include "user_management_dialog.h"
#include "change_password_dialog.h"
#include "global_variables.h"
#include "edit_chip_dialog.h"
#include "open_chip_dialog.h"
#include "edit_system_block_dialog.h"
#include <iostream>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "edit_chip_designer_dialog.h"
#include "edit_signal_dialog.h"
#include "edit_register_dialog.h"
#include "edit_signal_partition_dialog.h"
#include <assert.h>
#include <QDropEvent>
#include <QMimeData>
#include "data_utils.h"
#include "login_dialog.h"
#include "edit_document_dialog.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QtMath>
#include "edit_register_page_dialog.h"

RegisterManager::RegisterManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RegisterManager)
{
    ui->setupUi(this);
    ui->stackedWidgetChipEditor->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->tableSystem->setColumnHidden(0, true);
    ui->tableDesigner->setColumnHidden(0, true);
    ui->tableRegPage->setColumnHidden(0, true);
    ui->tableSignal->setColumnHidden(0, true);
    ui->tableSignal->setColumnHidden(1, true);
    ui->tableSignal->setColumnHidden(2, true);
    ui->tableSigPart->setColumnHidden(0, true);
    ui->tableRegister->setColumnHidden(0, true);
    ui->tableRegPart->setColumnHidden(0, true);
    ui->tableSigPart->setColumnWidth(2, 300);
    ui->tableRegPart->setColumnWidth(2, 300);

    //clear_db();
    //init_db();

    msb_first_ = true;
    QVector<QWidget*> widgets = {ui->pushButtonAddSys, ui->pushButtonRemoveSys, ui->pushButtonAddReg, ui->pushButtonRemoveReg, ui->pushButtonAddSig, ui->pushButtonRemoveSig, ui->pushButtonAddSigPart,
                                ui->pushButtonRemoveSigPart, ui->pushButtonAddDesigner, ui->pushButtonRemoveDesigner, ui->pushButtonAddRegPage, ui->pushButtonRemoveRegPage};
    for (QWidget* widget : widgets) widget->setEnabled(false);
    for (QAction* action : {ui->actionUserManagement, ui->actionNewChip}) action->setEnabled(false);

    //ui->splitterMain->setSizes({150, 600});
    ui->splitterMain->setCollapsible(0, false);
    ui->splitterMain->setCollapsible(1, false);
    ui->splitterWorking->setCollapsible(0, false);
    ui->splitterWorking->setCollapsible(1, false);

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

    QObject::connect(&login_dialog_, SIGNAL(logged_in(QString)), this, SLOT(on_loggedin(QString)));
    login_dialog_.show();
    ui->treeWidgetBlock->setColumnHidden(1, true);

    // TODO: set naming template, which is dependent of specific chips
    REGISTER_NAMING.set_naming_template(REGISTER_NAMING_TEMPLATE);
    SIGNAL_NAMING.set_naming_template(SIGNAL_NAMING_TEMPLATE);
    //int h = ui->documentEditor->height() + ui->stackedWidgetDoc->height();
    //ui->splitterDoc->setSizes({h - 100, 100});
    /*
    QVector<QTableWidget*> tables = {ui->tableChipBasics, ui->tableSystem, ui->tableDesigner, ui->tableRegPage,
                                    ui->tableSignal, ui->tableSigPart, ui->tableRegister, ui->tableRegPart};
    for (QTableWidget* table : tables) table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    */
    ui->tableSignal->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableRegPage->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableRegPage->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
}

void RegisterManager::on_actionAdd_triggered()
{
    if (ui->tableSystem->hasFocus()) on_pushButtonAddSys_clicked();
    else if (ui->tableDesigner->hasFocus()) on_pushButtonAddDesigner_clicked();
    else if (ui->tableRegPage->hasFocus()) on_pushButtonAddRegPage_clicked();
    else if (ui->tableSignal->hasFocus()) on_pushButtonAddSig_clicked();
    else if (ui->tableSigPart->hasFocus()) on_pushButtonAddSigPart_clicked();
    else if (ui->tableDoc->hasFocus()) on_pushButtonAddDoc_clicked();
    else if (ui->tableRegister->hasFocus()) on_pushButtonAddReg_clicked();
    // else if (ui->tableRegPart->hasFocus())
}

void RegisterManager::on_actionEdit_triggered()
{
    if (ui->tableSystem->hasFocus()) on_tableSystem_cellDoubleClicked(ui->tableSystem->currentRow(), ui->tableSystem->currentColumn());
    else if (ui->tableDesigner->hasFocus()) on_tableDesigner_cellDoubleClicked(ui->tableDesigner->currentRow(), ui->tableDesigner->currentColumn());
    else if (ui->tableRegPage->hasFocus()) on_tableRegPage_cellDoubleClicked(ui->tableRegPage->currentRow(), ui->tableRegPage->currentColumn());
    else if (ui->tableSignal->hasFocus()) on_tableSignal_cellDoubleClicked(ui->tableSignal->currentRow(), ui->tableSignal->currentColumn());
    //else if (ui->tableSigPart->hasFocus()) on_tableSigPart_cellDoubleClicked(ui->tableSigPart->currentRow(), ui->tableSigPart->currentColumn());
    else if (ui->tableDoc->hasFocus()); //on_treeWidgetDoc_itemDoubleClicked(ui->treeWidgetDoc->currentItem(), 0);
    else if (ui->tableRegister->hasFocus()) on_tableRegister_cellDoubleClicked(ui->tableRegister->currentRow(), ui->tableRegister->currentColumn());
    // else if (ui->tableRegPart->hasFocus())
}

void RegisterManager::on_actionRemove_triggered()
{
    if (ui->tableSystem->hasFocus()) on_pushButtonRemoveSys_clicked();
    else if (ui->tableDesigner->hasFocus()) on_pushButtonRemoveDesigner_clicked();
    else if (ui->tableRegPage->hasFocus()) on_pushButtonRemoveRegPage_clicked();
    else if (ui->tableSignal->hasFocus()) on_pushButtonRemoveSig_clicked();
    else if (ui->tableSigPart->hasFocus()) on_pushButtonRemoveSigPart_clicked();
    else if (ui->tableDoc->hasFocus()) on_pushButtonRemoveDoc_clicked();
    else if (ui->tableRegister->hasFocus()) on_pushButtonRemoveReg_clicked();
    // else if (ui->tableRegPart->hasFocus())
}


void RegisterManager::on_actionRefresh_triggered()
{
    if (ui->tableSystem->hasFocus()) display_system_blocks();
    else if (ui->tableDesigner->hasFocus()) display_designers();
    else if (ui->tableRegPage->hasFocus()) display_register_pages();
    else if (ui->tableSignal->hasFocus()) display_signals();
    else if (ui->tableSigPart->hasFocus()) display_signal_partitions();
    else if (ui->tableDoc->hasFocus()) display_documents();
    else if (ui->tableRegister->hasFocus()) display_registers();
    else if (ui->tableRegPart->hasFocus()) display_register_partitions();
}


void RegisterManager::on_tableDoc_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableDoc->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_.can_edit_document());
    actionAdd_->setEnabled(authenticator_.can_edit_document());
    actionRemove_->setEnabled(current && authenticator_.can_edit_document());
    context_menu_->popup(ui->tableDoc->viewport()->mapToGlobal(pos));
}

void RegisterManager::on_tableSignal_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableSignal->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_.can_add_signal() && authenticator_.can_remove_signal());
    actionAdd_->setEnabled(authenticator_.can_add_signal());
    actionRemove_->setEnabled(current && authenticator_.can_remove_signal());
    context_menu_->popup(ui->tableSignal->viewport()->mapToGlobal(pos));
}

void RegisterManager::on_tableRegister_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableRegister->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_.can_add_register() && authenticator_.can_remove_register());
    actionAdd_->setEnabled(authenticator_.can_add_register());
    actionRemove_->setEnabled(current && authenticator_.can_remove_register());
    context_menu_->popup(ui->tableRegister->viewport()->mapToGlobal(pos));
}

void RegisterManager::on_tableDesigner_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableDesigner->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_.can_add_chip_designer() && authenticator_.can_remove_chip_designer());
    actionAdd_->setEnabled(authenticator_.can_add_chip_designer());
    actionRemove_->setEnabled(current && authenticator_.can_remove_chip_designer());
    context_menu_->popup(ui->tableDesigner->viewport()->mapToGlobal(pos));
}

void RegisterManager::on_tableSigPart_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableSigPart->itemAt(pos);
    actionEdit_->setEnabled(false);
    actionAdd_->setEnabled(authenticator_.can_edit_signal_partition());
    actionRemove_->setEnabled(current && authenticator_.can_edit_signal_partition());
    context_menu_->popup(ui->tableSigPart->viewport()->mapToGlobal(pos));
}

void RegisterManager::on_tableRegPart_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableRegPart->itemAt(pos);
    // DO NOTHING
    /*
    actionEdit_->setEnabled(current && authenticator_.can_edit_register_partition());
    actionAdd_->setEnabled(current && authenticator_.can_edit_register_partition());
    actionRemove_->setEnabled(current && authenticator_.can_edit_register_partition());
    context_menu_->popup(ui->tableRegPart->viewport()->mapToGlobal(pos));
    */
}


void RegisterManager::on_tableSystem_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableSystem->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_.can_add_block() && ((authenticator_.can_remove_his_block() && ui->tableSystem->item(current->row(), 3)->text() == username_) || authenticator_.can_fully_access_all_blocks()));
    actionAdd_->setEnabled(authenticator_.can_add_block());
    actionRemove_->setEnabled(current && ((authenticator_.can_remove_his_block() && ui->tableSystem->item(current->row(), 3)->text() == username_) || authenticator_.can_fully_access_all_blocks()));
    context_menu_->popup(ui->tableSystem->viewport()->mapToGlobal(pos));
}


void RegisterManager::on_tableRegPage_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableRegPage->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_.can_fully_access_all_blocks());
    actionAdd_->setEnabled(authenticator_.can_fully_access_all_blocks());
    actionRemove_->setEnabled(current && authenticator_.can_fully_access_all_blocks());
    context_menu_->popup(ui->tableRegPage->viewport()->mapToGlobal(pos));
}

void RegisterManager::table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row)
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


bool RegisterManager::eventFilter(QObject *obj, QEvent *eve)
{
    if (obj == ui->tableSystem->viewport() ||
            obj == ui->tableRegister->viewport() ||
            obj == ui->treeWidgetBlock->viewport() ||
            obj == ui->tableDoc->viewport())
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
                if (obj == ui->tableSystem->viewport()) pDropItem = ui->tableSystem->itemAt((static_cast<QDropEvent*>(eve))->pos());
                if (obj == ui->tableRegister->viewport()) pDropItem = ui->tableRegister->itemAt((static_cast<QDropEvent*>(eve))->pos());
                if (obj == ui->tableDoc->viewport()) pDropItem = ui->tableDoc->itemAt((static_cast<QDropEvent*>(eve))->pos());
                //if (obj == ui->treeWidgetBlock->viewport()) pDropItem = ui->treeWidgetBlock->childAt((static_cast<QDropEvent*>(eve))->pos());
                if (!pDropItem) return true;
                if (pDropItem->row() == row) return true;

                if (obj == ui->tableRegister->viewport())
                {
                    table_drop_event_handling(ui->tableRegister, "block_register", "reg_id", row, pDropItem->row());
                    ui->tableRegister->setCurrentCell(pDropItem->row(), 0);
                }
                if (obj == ui->tableSystem->viewport())
                {
                    table_drop_event_handling(ui->tableSystem, "block_system_block", "block_id", row, pDropItem->row());
                    ui->tableSystem->setCurrentCell(pDropItem->row(), 0);

                    QTreeWidgetItem *topLevelItem = ui->treeWidgetBlock->topLevelItem(0);
                    QVector<QString> from;
                    for (int j = 0; j < ui->treeWidgetBlock->columnCount(); j++)
                        from.push_back(topLevelItem->child(row)->text(j));
                    if (row > pDropItem->row())
                    {
                        for (int i = row; i > pDropItem->row(); i--)
                            for (int j = 0; j < ui->treeWidgetBlock->columnCount(); j++)
                                topLevelItem->child(i)->setText(j, topLevelItem->child(i-1)->text(j));
                    }
                    else {
                        for (int i = row; i < pDropItem->row(); i++)
                            for (int j = 0; j < ui->treeWidgetBlock->columnCount(); j++)
                                topLevelItem->child(i)->setText(j, topLevelItem->child(i+1)->text(j));
                    }
                    for (int j = 0; j < ui->treeWidgetBlock->columnCount(); j++)
                        topLevelItem->child(pDropItem->row())->setText(j, from[j]);
                }
                if (obj == ui->tableDoc->viewport())
                {
                    QTreeWidgetItem* current = ui->treeWidgetBlock->currentItem();
                    QTreeWidgetItem* block_item = current;
                    while (block_item->parent()->parent()) block_item = block_item->parent();

                    if (current->parent() == block_item)   // register
                        table_drop_event_handling(ui->tableDoc, "doc_register", "register_doc_id", row, pDropItem->row());
                    else if (current->parent() && current->parent()->parent() == block_item)  // signal
                        table_drop_event_handling(ui->tableDoc, "doc_signal", "signal_doc_id", row, pDropItem->row());
                    else // block
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

RegisterManager::~RegisterManager()
{
    delete ui;
}


void RegisterManager::open_chip()
{
    QAbstractItemView* widget = ui->tableSystem;
    if (chip_owner_id_ == user_id_) widget->viewport()->installEventFilter(this);
    else widget->viewport()->removeEventFilter(this);
    widget->setDragEnabled(chip_owner_id_ == user_id_);
    widget->setAcceptDrops(chip_owner_id_ == user_id_);
    widget->setDragDropMode(chip_owner_id_ == user_id_ ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
    ui->stackedWidgetChipEditor->setCurrentIndex(0);
    ui->stackedWidgetDoc->setCurrentIndex(0);

    SIGNAL_NAMING.update_key("{CHIP_NAME}", chip_);
    REGISTER_NAMING.update_key("{CHIP_NAME}", chip_);
    display_chip_basics();
    display_system_blocks();
    display_designers();
    display_register_pages();
    display_overall_documents();
    // TODO: compile project widget
}

void RegisterManager::on_treeWidgetBlock_itemClicked(QTreeWidgetItem *item, int column)
{
    if (!item->parent())
    {
        ui->stackedWidgetChipEditor->setCurrentIndex(0);
        block_ = "";
        block_id_ = "-1";
        ui->stackedWidgetDoc->setCurrentIndex(0);
        display_overall_documents();
        display_system_blocks();
        display_register_pages();
    }
    else {
        ui->stackedWidgetDoc->setCurrentIndex(1);
        QString prev_block_id = block_id_;
        int prev_tab_index = ui->tabWidget->currentIndex(), prev_stack_index = ui->stackedWidgetChipEditor->currentIndex();

        ui->stackedWidgetChipEditor->setCurrentIndex(1);
        QString block_responsible, reg_id, sig_id;
        QTreeWidgetItem* block_item = item;
        while (block_item->parent()->parent()) block_item = block_item->parent();

        int row = ui->treeWidgetBlock->topLevelItem(0)->indexOfChild(block_item);
        block_id_ = ui->tableSystem->item(row, 0)->text();
        block_ = ui->tableSystem->item(row, 1)->text();
        block_responsible = ui->tableSystem->item(row, 3)->text();

        assert (block_id_ == block_item->text(1) && block_ == block_item->text(0));
        if (item->parent() == block_item)   // register
            reg_id = item->text(1);
        else if (item->parent() && item->parent()->parent() == block_item)  // signal
            sig_id = item->text(1);

        authenticator_.set_block_permissions(block_responsible == username_ || authenticator_.can_fully_access_all_blocks());

        REGISTER_NAMING.update_key("{BLOCK_NAME}", block_);
        REGISTER_NAMING.update_key("{BLOCK_ABBR}", block_id2abbr_[block_id_]);
        SIGNAL_NAMING.update_key("{BLOCK_NAME}", block_);
        SIGNAL_NAMING.update_key("{BLOCK_ABBR}", block_id2abbr_[block_id_]);


        QVector<QTableWidget*> tables = {ui->tableRegister, ui->tableDoc};
        QVector<bool> enables = {authenticator_.can_add_register() && authenticator_.can_remove_register(), authenticator_.can_edit_document()};
        for(int i = 0; i < 2; i++)
        {
            QTableWidget* table = tables[i];
            bool enabled = enables[i];
            if (enabled) table->viewport()->installEventFilter(this);
            else table->viewport()->removeEventFilter(this);
            table->setDragEnabled(enabled);
            table->setAcceptDrops(enabled);
            table->setDragDropMode(enabled ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
        }

        if (sig_id != "") ui->tabWidget->setCurrentIndex(0);
        else if (reg_id != "") ui->tabWidget->setCurrentIndex(1);
        //else ui->tabWidget->setCurrentIndex(0);  // default

        if (ui->tabWidget->currentIndex() == 0)
        {
            if (!(prev_stack_index == 1 && prev_tab_index == 0 && block_id_ == prev_block_id))
            display_signals();
        }
        else if (!(prev_stack_index == 1 && prev_tab_index == 1 && block_id_ == prev_block_id))
        {
            display_registers();
        }
        if (sig_id != "")
        {
            for (int row = 0; row < ui->tableSignal->rowCount(); row++)
                if (sig_id == ui->tableSignal->item(row, 0)->text())
                {
                    ui->tableSignal->setCurrentCell(row, 0);
                    break;
                }
        }
        else if (reg_id != "")
        {
            for (int row = 0; row < ui->tableRegister->rowCount(); row++)
                if (reg_id == ui->tableRegister->item(row, 0)->text())
                {
                    ui->tableRegister->setCurrentCell(row, 0);
                    break;
                }
        }

        display_documents(block_id_, reg_id, sig_id);
    }
}

void RegisterManager::on_tableSignal_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonAddSigPart->setEnabled(authenticator_.can_edit_signal_partition() && currentRow >= 0 && ui->tableSignal->item(currentRow, 1)->text() != "");
    ui->pushButtonRemoveSig->setEnabled(authenticator_.can_remove_signal() && currentRow >= 0);
    display_signal_partitions();
}

void RegisterManager::on_tableRegister_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveReg->setEnabled(authenticator_.can_remove_register() && currentRow >= 0);
    display_register_partitions();
}


void RegisterManager::on_tabWidget_currentChanged(int index)
{
    if (index == 0) display_signals();
    else if (index == 1) display_registers();
}


void RegisterManager::display_chip_basics()
{
    ui->tableChipBasics->setRowCount(0);
    ui->tableChipBasics->insertRow(0);
    ui->tableChipBasics->setItem(0, 0, new QTableWidgetItem(chip_));
    ui->tableChipBasics->setItem(0, 1, new QTableWidgetItem(chip_owner_));
    ui->tableChipBasics->setItem(0, 2, new QTableWidgetItem(QString::number(register_width_)));
    ui->tableChipBasics->setItem(0, 3, new QTableWidgetItem(QString::number(address_width_)));
    ui->tableChipBasics->setItem(0, 4, new QTableWidgetItem(msb_first_ ? "1" : "0"));
}

void RegisterManager::display_system_blocks()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;

    ui->treeWidgetBlock->clear();
    ui->tableSystem->setRowCount(0);

    QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui->treeWidgetBlock);
    ui->treeWidgetBlock->addTopLevelItem(topLevelItem);
    topLevelItem->setText(0, chip_);
    topLevelItem->setSelected(true);
    topLevelItem->setExpanded(true);

    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_.can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    dbhandler.show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                     "global_user.username",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id");

    items = sort_doubly_linked_list(items);
    for (const auto& item : items)
    {
        int row = ui->tableSystem->rowCount();
        ui->tableSystem->insertRow(row);
        for (int i = 0; i < item.size() - 2; i++)
            ui->tableSystem->setItem(row, i, new QTableWidgetItem(item[i]));

        QVector<QString> count;
        dbhandler.show_one_item("block_register", count, {"count(reg_id)"}, "block_id", item[0]);
        ui->tableSystem->setItem(row, ui->tableSystem->columnCount()-1, new QTableWidgetItem(count[0]));
        QTreeWidgetItem *block_item = new QTreeWidgetItem(topLevelItem);
        block_item->setText(0, item[1]);
        block_item->setText(1, item[0]);
        blocks_.push_back(item[1]);
        block_id2abbr_[item[0]] = item[2];
    }
    for (int i = 0; i < topLevelItem->childCount(); i++)
        refresh_block(topLevelItem->child(i));
    ui->pushButtonAddSys->setEnabled(authenticator_.can_add_block());
    ui->pushButtonRemoveSys->setEnabled(false);
    if (ui->treeWidgetBlock->topLevelItemCount()) ui->treeWidgetBlock->setCurrentItem(ui->treeWidgetBlock->topLevelItem(0));
}


void RegisterManager::refresh_block(QTreeWidgetItem *block_item)
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    QVector<QVector<QString> > registers;
    QString block_name = block_item->text(0),
            block_id = block_item->text(1),
            block_abbr = block_id2abbr_[block_id];

    QHash<QString, bool> reg_id2expanded;
    QVector<QTreeWidgetItem*> children;
    for (int i = 0; i < block_item->childCount(); i++) children.push_back(block_item->child(i));
    for (QTreeWidgetItem* child : children)
    {
        reg_id2expanded[child->text(1)] = child->isExpanded();
        block_item->removeChild(child);
    }

    dbhandler.show_items("block_register", {"reg_id", "reg_name", "prev", "next"}, "block_id", block_item->text(1), registers);
    registers = sort_doubly_linked_list(registers);

    SIGNAL_NAMING.update_key("{BLOCK_NAME}", block_name);
    REGISTER_NAMING.update_key("{BLOCK_NAME}", block_name);
    SIGNAL_NAMING.update_key("{BLOCK_ABBR}", block_abbr);
    REGISTER_NAMING.update_key("{BLOCK_ABBR}", block_abbr);
    for (const auto& reg : registers)
    {
        QTreeWidgetItem *topLevelItemReg = new QTreeWidgetItem(block_item);
        topLevelItemReg->setText(0, REGISTER_NAMING.get_extended_name(reg[1]));
        topLevelItemReg->setText(1, reg[0]);

        if (reg_id2expanded.contains(reg[0])) topLevelItemReg->setExpanded(reg_id2expanded[reg[0]]);

        QVector<QVector<QString> > signal_items;
        QVector<QString> ext_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb",
                                       "signal_signal.sig_name",
                                       "signal_signal.sig_id",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                      "signal_signal.add_port"};
        dbhandler.show_items_inner_join(ext_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                     {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, signal_items, {{"block_sig_reg_partition_mapping.reg_id", reg[0]}});

        if (msb_first_) qSort(signal_items.begin(), signal_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
        else qSort(signal_items.begin(), signal_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});

        QSet<QString> signal_set;
        QVector<QString> signal_ids, signal_names;

        for (const auto& signal_item : signal_items)
        {
            QString sig_id = signal_item[4], sig_name = signal_item[3];
            if (signal_set.contains(sig_id)) continue;
            signal_set.insert(sig_id);
            signal_ids.push_back(sig_id);
            signal_names.push_back(signal_item[7]=="1" ? SIGNAL_NAMING.get_extended_name(sig_name) : sig_name);
        }

        for (int i = 0; i < signal_ids.size(); i++)
        {
            QString sig_id = signal_ids[i], sig_name = signal_names[i];
            QTreeWidgetItem *top_level_item_signal = new QTreeWidgetItem(topLevelItemReg);
            top_level_item_signal->setText(0, sig_name);
            top_level_item_signal->setText(1, sig_id);
            //top_level_item_signal->setExpanded(true);
        }
    }
    if (!ui->treeWidgetBlock->currentItem()) ui->treeWidgetBlock->setCurrentItem(block_item ? block_item : ui->treeWidgetBlock->topLevelItem(0));
}


void RegisterManager::display_designers()
{

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;

    ui->tableDesigner->setRowCount(0);

    dbhandler.show_items_inner_join({"chip_designer.chip_designer_id",
                                     "global_user.username",
                                     "def_project_role.project_role"},
                                    {{{"global_user", "user_id"}, {"chip_designer", "user_id"}},
                                     {{"chip_designer", "project_role_id"}, {"def_project_role", "project_role_id"}}},
                                    items, {{"chip_designer.chip_id", chip_id_}},
                                    "order by chip_designer.chip_designer_id");
    for (const auto& item : items)
    {
        int row = ui->tableDesigner->rowCount();
        ui->tableDesigner->insertRow(row);
        for (int i = 0; i < item.size(); i++) ui->tableDesigner->setItem(row, i, new QTableWidgetItem(item[i]));
    }
    ui->pushButtonAddDesigner->setEnabled(authenticator_.can_add_chip_designer());
    ui->pushButtonRemoveDesigner->setEnabled(false);
}

void RegisterManager::display_register_pages()
{
    ui->tableRegPage->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    QVector<QString> ext_fields = {"chip_register_page.page_id", "chip_register_page.page_name",
                                   "signal_signal.sig_name", "chip_register_page.page_count",
                                   "block_system_block.block_name", "block_system_block.abbreviation", "signal_signal.add_port"};
    dbhandler.show_items_inner_join(ext_fields, {{{"chip_register_page", "ctrl_sig"}, {"signal_signal", "sig_id"}}, {{"signal_signal", "block_id"}, {"block_system_block", "block_id"}}}, items, {{"chip_register_page.chip_id", chip_id_}});
    for (const auto& item : items)
    {
        QString block_name = item[4], block_abbr = item[5];
        SIGNAL_NAMING.update_key("{BLOCK_NAME}", block_name);
        SIGNAL_NAMING.update_key("{BLOCK_ABBR}", block_abbr);
        int row = ui->tableRegPage->rowCount();
        ui->tableRegPage->insertRow(row);
        QVector<QString> values = {item[0], item[1], item[6]=="1" ? SIGNAL_NAMING.get_extended_name(item[2]) : item[2], item[3]};
        for (int i = 0; i < values.size(); i++) ui->tableRegPage->setItem(row, i, new QTableWidgetItem(values[i]));
    }
    ui->pushButtonAddRegPage->setEnabled(authenticator_.can_fully_access_all_blocks());
}

void RegisterManager::display_signals()
{
    ui->tableSignal->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    QVector<QString> fields = {"signal_signal.sig_id", "signal_signal.sig_name", "signal_signal.width", "def_signal_type.sig_type", "signal_signal.add_port"};
    dbhandler.show_items_inner_join(fields,
                                    {{{"signal_signal", "sig_type_id"}, {"def_signal_type", "sig_type_id"}}},
                                    items,
                                    {{"signal_signal.block_id", block_id_}});
    for (const auto& item : items)
    {
        int row = ui->tableSignal->rowCount();
        ui->tableSignal->insertRow(row);
        QString sig_id = item[0],
                reg_sig_id, reg_type_id,
                add_port = item[4],
                sig_name = item[4] == "1" ? SIGNAL_NAMING.get_extended_name(item[1]) : item[1],
                width = item[2],
                sig_type = item[3],
                reg_type, init_value;

//        ui->tableSignal->setItem(row, 0, new QTableWidgetItem(item[0]));
//        ui->tableSignal->setItem(row, 3, new QTableWidgetItem(item[1]));
//        ui->tableSignal->setItem(row, 4, new QTableWidgetItem(item[2]));
//        ui->tableSignal->setItem(row, 5, new QTableWidgetItem(item[3]));

        QVector<QVector<QString> > items_reg_sig;
        dbhandler.show_items_inner_join({"signal_reg_signal.reg_sig_id", "def_register_type.reg_type_id", "def_register_type.reg_type", "signal_reg_signal.init_value"},
                                        {{{"signal_reg_signal", "reg_type_id"}, {"def_register_type", "reg_type_id"}}},
                                        items_reg_sig, {{"signal_reg_signal.sig_id", item[0]}});

        if (items_reg_sig.size() == 1)
        {
            reg_sig_id = items_reg_sig[0][0];
            reg_type_id = items_reg_sig[0][1];
            reg_type = items_reg_sig[0][2];
            init_value = items_reg_sig[0][3];
        }
        QVector<QString> values = {sig_id, reg_sig_id, reg_type_id, sig_name, width, sig_type, reg_type, init_value, add_port};
        for (int i = 0; i < ui->tableSignal->columnCount(); i++)
            ui->tableSignal->setItem(row, i, new QTableWidgetItem(values[i]));
    }
    ui->pushButtonAddSig->setEnabled(authenticator_.can_add_signal());
    ui->pushButtonRemoveSig->setEnabled(false);
}

void RegisterManager::display_registers()
{
    ui->tableRegister->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"block_register.reg_id", "block_register.reg_name", "def_register_type.reg_type", "block_register.prev", "block_register.next"},
                                    {{{"block_register", "reg_type_id"}, {"def_register_type", "reg_type_id"}}},
                                    items, {{"block_register.block_id", block_id_}});

    items  = sort_doubly_linked_list(items);
    QVector<QString> item;
    dbhandler.show_one_item("block_system_block", item, {"start_address"}, "block_id", block_id_);
    int start_address = item[0].toInt();

    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        int row = ui->tableRegister->rowCount();
        QString reg_id = item[0], reg_name = item[1], reg_type = item[2], address = decimal2hex(start_address+i, address_width_);
        reg_name = REGISTER_NAMING.get_extended_name(reg_name);
        ui->tableRegister->insertRow(row);
        ui->tableRegister->setItem(row, 0, new QTableWidgetItem(reg_id));
        ui->tableRegister->setItem(row, 1, new QTableWidgetItem(reg_name));
        ui->tableRegister->setItem(row, 2, new QTableWidgetItem(reg_type));
        ui->tableRegister->setItem(row, 3, new QTableWidgetItem(address));

    }
    ui->pushButtonAddReg->setEnabled(authenticator_.can_add_register());
    ui->pushButtonRemoveReg->setEnabled(false);
}


void RegisterManager::display_documents(const QString& block_id, const QString& reg_id, const QString& sig_id)
{
    ui->tableDoc->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    if (sig_id != "")
        dbhandler.show_items_inner_join({"doc_signal.signal_doc_id", "def_doc_type.doc_type", "doc_signal.content", "doc_signal.doc_type_id", "doc_signal.prev", "doc_signal.next"},
                                        {{{"doc_signal", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_signal.sig_id", sig_id}});
    else if (reg_id != "")
        dbhandler.show_items_inner_join({"doc_register.register_doc_id", "def_doc_type.doc_type", "doc_register.content", "doc_register.doc_type_id", "doc_register.prev", "doc_register.next"},
                                        {{{"doc_register", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                        items, {{"doc_register.reg_id", reg_id}});
    else if (block_id != "")
        dbhandler.show_items_inner_join({"doc_block.block_doc_id", "def_doc_type.doc_type", "doc_block.content", "doc_block.doc_type_id", "doc_block.prev", "doc_block.next"},
                                    {{{"doc_block", "doc_type_id"}, {"def_doc_type", "doc_type_id"}}},
                                    items, {{"doc_block.block_id", block_id}});
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
    ui->pushButtonAddDoc->setEnabled(authenticator_.can_edit_document());
    ui->pushButtonRemoveDoc->setEnabled(false);
    ui->documentEditor->clear();
    ui->documentEditor->setVisible(false);
}

void RegisterManager::display_documents()
{
    QTreeWidgetItem* current = ui->treeWidgetBlock->currentItem();
    QTreeWidgetItem* block_item = current;
    while (block_item->parent()->parent()) block_item = block_item->parent();

    QString block_id, reg_id, sig_id;
    ui->documentEditor->clear();
    if (current->parent() == block_item)   // register
        reg_id = current->text(1);
    else if (current->parent() && current->parent()->parent() == block_item)  // signal
        sig_id = current->text(1);
    else // block
        block_id = current->text(1);
    display_documents(block_id, reg_id, sig_id);
}


void RegisterManager::display_overall_documents()
{
    ui->documentEditor->set_mode(DIALOG_MODE::EDIT);
    EditDocumentDialog::DOCUMENT_LEVEL level;

    ui->webDocOverview->setHtml("", QUrl("file://"));

    QString html_content, table_of_content;
    for (int i = 0; i < ui->tableSystem->rowCount(); i++)
    {
        QString block_id = ui->tableSystem->item(i, 0)->text(),
                block_name = ui->tableSystem->item(i, 1)->text(),
                block_abbr = ui->tableSystem->item(i, 2)->text();

        long long block_start_address = ui->tableSystem->item(i, 4)->text().toLongLong(nullptr, 16);

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
            QString address = decimal2hex(block_start_address + i, address_width_);

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


void RegisterManager::display_signal_partitions()
{
    ui->tableSigPart->setRowCount(0);
    int row = ui->tableSignal->currentRow();
    if (row < 0 || ui->tableSignal->item(row, 1)->text() == "") return;
    QString reg_sig_id = ui->tableSignal->item(row, 1)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> ext_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                       "block_register.reg_name",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb"};

    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join(ext_fields, {{{"block_sig_reg_partition_mapping", "reg_id"}, {"block_register", "reg_id"}}}, items,  {{"block_sig_reg_partition_mapping.reg_sig_id", reg_sig_id}});

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});
    for (const auto& item : items)
    {
        int row = ui->tableSigPart->rowCount();
        ui->tableSigPart->insertRow(row);
        QString sig_reg_part_mapping_id = item[0], sig_lsb = item[1], sig_msb = item[2], reg_name = item[3], reg_lsb = item[4], reg_msb = item[5];
        reg_name = REGISTER_NAMING.get_extended_name(reg_name);
        ui->tableSigPart->setItem(row, 0, new QTableWidgetItem(sig_reg_part_mapping_id));
        QString sig_part = msb_first_ ? "<" + sig_msb + ":"+ sig_lsb + ">" : "<" + sig_lsb + ":"+ sig_msb + ">",
                reg_part = msb_first_ ? reg_name + "<" + reg_msb + ":"+ reg_lsb +">" : reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
        ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
        ui->tableSigPart->setItem(row, 2, new QTableWidgetItem(reg_part));
    }
}


void RegisterManager::display_register_partitions()
{
    int row = ui->tableRegister->currentRow();
    ui->tableRegPart->setRowCount(0);
    if (row < 0) return;
    QString reg_id = ui->tableRegister->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> ext_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                   "block_sig_reg_partition_mapping.reg_lsb",
                                   "block_sig_reg_partition_mapping.reg_msb",
                                   "signal_signal.sig_name",
                                   "block_sig_reg_partition_mapping.sig_lsb",
                                   "block_sig_reg_partition_mapping.sig_msb",
                                  "signal_signal.add_port"};
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join(ext_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                 {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, items, {{"block_sig_reg_partition_mapping.reg_id", reg_id}});

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});
    for (const auto& item : items)
    {
        int row = ui->tableRegPart->rowCount();
        ui->tableRegPart->insertRow(row);
        QString sig_reg_part_mapping_id = item[0], reg_lsb = item[1], reg_msb = item[2], sig_name = item[3], sig_lsb = item[4], sig_msb = item[5];
        sig_name =  item[6]=="1" ? SIGNAL_NAMING.get_extended_name(sig_name) : sig_name;
        ui->tableRegPart->setItem(row, 0, new QTableWidgetItem(sig_reg_part_mapping_id));
        QString reg_part = msb_first_ ? "<" + reg_msb + ":"+ reg_lsb + ">" : "<" + reg_lsb + ":"+ reg_msb + ">",
                sig_part = msb_first_ ? sig_name + "<" + sig_msb + ":"+ sig_lsb +">" : sig_name + "<" + sig_lsb + ":"+ sig_msb +">";

        ui->tableRegPart->setItem(row, 1, new QTableWidgetItem(reg_part));
        ui->tableRegPart->setItem(row, 2, new QTableWidgetItem(sig_part));
    }
}

void RegisterManager::on_loggedin(QString username)
{

    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"global_user.user_id", "def_db_role.db_role", "def_db_role.db_role_id"},
                                    {{{"global_user", "db_role_id"}, {"def_db_role", "db_role_id"}}}, items, {{"username", username}});
    username_ = username;
    user_id_ = items[0][0];
    db_role_ = items[0][1];
    db_role_id_ = items[0][2];
    authenticator_.set_database_permissions(db_role_id_);

    ui->actionUserManagement->setEnabled(authenticator_.can_add_user() && authenticator_.can_remove_user());
    ui->actionNewChip->setEnabled(authenticator_.can_add_project());
    ui->actionChipManagement->setEnabled(authenticator_.can_add_project() && authenticator_.can_remove_project());
    setWindowTitle("IAS Register Manager - " + username);

    login_dialog_.hide();
    OpenChipDialog open_dial(user_id_, authenticator_.can_add_project(), this);
    show();
    on_actionOpenChip_triggered();
}

void RegisterManager::on_actionUserManagement_triggered()
{
    if (!authenticator_.can_add_user() || !authenticator_.can_remove_user())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to User Manager!");
        return;
    }
    UserManagementDialog user_management(username_, this);
    user_management.exec();
}

void RegisterManager::on_actionChangePassword_triggered()
{
    ChangePasswordDialog change_password(username_, this);
    if (change_password.exec() == QDialog::Accepted) change_password.change_password();
}

void RegisterManager::on_actionLogOut_triggered()
{
    hide();
    on_actionCloseChip_triggered();
    authenticator_.clear_database_permission();
    for (QString* s : {&username_, &user_id_, &db_role_, &db_role_id_}) s->clear();
    for (QAction* action : {ui->actionUserManagement, ui->actionNewChip, ui->actionChipManagement}) action->setEnabled(false);

    login_dialog_.clear();
    login_dialog_.show();
}

void RegisterManager::on_actionNewChip_triggered()
{
    if (!authenticator_.can_add_project())
    {
        QMessageBox::warning(this, "New Chip", "You are not eligible to add projects!");
        return;
    }
    EditChipDialog new_chip(user_id_, this);
    if (new_chip.exec() == QDialog::Accepted && new_chip.add_chip())
    {
        chip_ = new_chip.get_chip_name();
        chip_id_ = new_chip.get_chip_id();
        address_width_ = new_chip.get_address_width();
        register_width_ = new_chip.get_register_width();

        chip_owner_ = username_;
        chip_owner_id_ = user_id_;

        int row = ui->tableDesigner->rowCount();
        ui->tableDesigner->insertRow(row);
        ui->tableDesigner->setItem(row, 0, new QTableWidgetItem(new_chip.get_chip_designer_id()));
        ui->tableDesigner->setItem(row, 1, new QTableWidgetItem(username_));
        ui->tableDesigner->setItem(row, 2, new QTableWidgetItem(new_chip.get_project_role()));
        authenticator_.set_project_permissions(new_chip.get_project_role_id());

        open_chip();
    }
}

void RegisterManager::on_actionOpenChip_triggered()
{
    OpenChipDialog open_dial(user_id_, authenticator_.can_add_project(), this);
    if (open_dial.exec() == QDialog::Accepted)
    {
        on_actionCloseChip_triggered();
        chip_ = open_dial.get_chip_name();
        chip_id_ = open_dial.get_chip_id();
        address_width_ = open_dial.get_address_width();
        register_width_ = open_dial.get_register_width();
        msb_first_ = open_dial.is_msb_first();
        chip_owner_ = open_dial.get_owner();
        chip_owner_id_ = open_dial.get_owner_id();
        authenticator_.set_project_permissions(open_dial.get_project_role_id());
        open_chip();
    }
    //
}

void RegisterManager::on_actionCloseChip_triggered()
{
    QVector<QTableWidget*> tables = {ui->tableChipBasics, ui->tableSystem, ui->tableDesigner, ui->tableRegPage,
                                     ui->tableSignal, ui->tableSigPart, ui->tableRegister, ui->tableRegPart, ui->tableDoc};
    for (QTableWidget* table : tables) table->setRowCount(0);
    ui->treeWidgetBlock->clear();
    ui->webDocOverview->setHtml("", QUrl("file://"));

    QVector<QWidget*> widgets = {ui->pushButtonAddSys, ui->pushButtonRemoveSys, ui->pushButtonAddReg, ui->pushButtonRemoveReg,
                                 ui->pushButtonAddSig, ui->pushButtonRemoveSig, ui->pushButtonAddSigPart,
                                ui->pushButtonRemoveSigPart, ui->pushButtonAddDesigner, ui->pushButtonRemoveDesigner,
                                ui->pushButtonAddDoc, ui->pushButtonRemoveDoc, ui->pushButtonAddRegPage, ui->pushButtonRemoveRegPage};
    for (QWidget* widget : widgets) widget->setEnabled(false);

    QVector<QString*> variables = {&chip_, &chip_id_, &chip_owner_, &chip_owner_id_};
    for (QString* &v : variables) v->clear();

    address_width_ = 0;
    register_width_ = 0;
    msb_first_ = true;

    authenticator_.clear_project_permission();
    authenticator_.clear_block_permission();

    ui->stackedWidgetDoc->setCurrentIndex(0);
    ui->stackedWidgetChipEditor->setCurrentIndex(0);

    block_id2abbr_.clear();
}

void RegisterManager::on_actionChipManagement_triggered()
{
    if (!authenticator_.can_add_project() || !authenticator_.can_remove_project())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to Chip Manager!");
        return;
    }
    OpenChipDialog open_dial(user_id_, chip_id_, this);
    open_dial.exec();
}

void RegisterManager::on_actionDocEditorView_triggered()
{
    ui->frameDoc->setVisible(ui->actionDocEditorView->isChecked());
}

void RegisterManager::on_actionChipEditorView_triggered()
{
    ui->frameChipEditor->setVisible(ui->actionChipEditorView->isChecked());
}

void RegisterManager::on_pushButtonAddSys_clicked()
{
    if (!authenticator_.can_add_block())
    {
        QMessageBox::warning(this, "New System Block", "You are not eligible to add system blocks!");
        return;
    }
    EditSystemBlockDialog new_system(chip_id_, address_width_, authenticator_.can_add_chip_designer(), this);
    new_system.setWindowTitle("New System Block");
    if (new_system.exec() == QDialog::Accepted && new_system.add_system_block())
    {
        int row = ui->tableSystem->rowCount();
        ui->tableSystem->insertRow(row);
        //
        ui->tableSystem->setItem(row, 1, new QTableWidgetItem(new_system.get_block_name()));
        ui->tableSystem->setItem(row, 2, new QTableWidgetItem(new_system.get_block_abbr()));
        ui->tableSystem->setItem(row, 3, new QTableWidgetItem(new_system.get_responsible()));
        ui->tableSystem->setItem(row, 4, new QTableWidgetItem(new_system.get_start_addr()));
        ui->tableSystem->setItem(row, 5, new QTableWidgetItem("0"));

        block_id_ = new_system.get_block_id();
        block_ = new_system.get_block_name();
        ui->tableSystem->setItem(row, 0, new QTableWidgetItem(block_id_));
        block_id2abbr_[block_id_] = new_system.get_block_abbr();

        QTreeWidgetItem *top_level_item = ui->treeWidgetBlock->topLevelItem(0);
        QTreeWidgetItem *tree_item = new QTreeWidgetItem(top_level_item);
        tree_item->setText(0, new_system.get_block_name());
        tree_item->setText(1, new_system.get_block_id());
        ui->tableSystem->setCurrentCell(row, 0);
    }
    if (new_system.designer_added()) display_designers();
}

void RegisterManager::on_pushButtonRemoveSys_clicked()
{
    int row = ui->tableSystem->currentRow();
    if (row < 0) return;
    QString responsible = ui->tableSystem->item(row, 3)->text();
    if (!((authenticator_.can_remove_his_block() && responsible == username_) || authenticator_.can_fully_access_all_blocks()))
    {
        QMessageBox::warning(this, "Remove System Block", "You are not eligible to this system block!");
        return;
    }

    if (QMessageBox::warning(this,
                         "Remove System Block",
                         "Are you sure you want to remove this blcok?\nThis operation is not reversible!",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    block_id_ = ui->tableSystem->item(row, 0)->text();
    QString error_msg;
    if (!DataBaseHandler::remove_block(block_id_, &error_msg))
    {
        QMessageBox::warning(this, "Remove System Block", QString("Removing system block failed\nError message: ")+ error_msg);
        return;
    }
    ui->tableSystem->removeRow(row);
    QTreeWidgetItem *top_level_item = ui->treeWidgetBlock->topLevelItem(0);
    top_level_item->removeChild(top_level_item->child(row));
    block_id2abbr_.remove(block_id_);

}

void RegisterManager::on_pushButtonAddDesigner_clicked()
{
    if (!authenticator_.can_add_chip_designer())
    {
        QMessageBox::warning(this, "Add Chip Designer", "You are not eligible to add chip designer!");
        return;
    }
    EditChipDesignerDialog add_designer(chip_id_, this);
    if (add_designer.exec() == QDialog::Accepted && add_designer.add_designer())
    {
        QString username = add_designer.get_username();
        QString user_id = add_designer.get_user_id();
        QString project_role = add_designer.get_project_role();
        int row = ui->tableDesigner->rowCount();
        ui->tableDesigner->insertRow(row);
        ui->tableDesigner->setItem(row, 0, new QTableWidgetItem(add_designer.get_chip_designer_id()));
        ui->tableDesigner->setItem(row, 1, new QTableWidgetItem(username));
        ui->tableDesigner->setItem(row, 2, new QTableWidgetItem(project_role));
    }
}

void RegisterManager::on_pushButtonRemoveDesigner_clicked()
{
    if (!authenticator_.can_remove_chip_designer())
    {
        QMessageBox::warning(this, "Remove Chip Designer", "You are not eligible to remove chip designer!");
        return;
    }
    int row = ui->tableDesigner->currentRow();
    if (row < 0) return;
    QString designer_id = ui->tableDesigner->item(row, 0)->text(), username = ui->tableDesigner->item(row, 1)->text();
    if (username == username_)
    {
        QMessageBox::warning(this, "Remove Designer", "You cannot remove yourself!");
        return;
    }
    if (username == chip_owner_)
    {
        QMessageBox::warning(this, "Remove Designer", "You cannot remove owner of the project!");
        return;
    }
    if (QMessageBox::warning(this,
                         "Remove Designer",
                         "Are you sure you want to remove this designer?",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    for (int i = 0; i < ui->tableSystem->rowCount(); i++)
    {
        if (ui->tableSystem->item(i, 3)->text() == username)
        {
            QString block_id = ui->tableSystem->item(i, 0)->text();
            if (!dbhandler.update_items("block_system_block", "block_id", block_id, {{"responsible", chip_owner_id_}}))
            {
                QMessageBox::critical(this, "Remove Designer", "Unnable to remove designer!\nError message: " + dbhandler.get_error_message());
                return;
            }
            ui->tableSystem->item(i, 3)->setText(chip_owner_);
        }
    }
    if (dbhandler.delete_items("chip_designer", "chip_designer_id", designer_id))
    {
        int row = ui->tableDesigner->currentRow();
        ui->tableDesigner->removeRow(row);
    }
    else QMessageBox::critical(this, "Remove Designer", "Unnable to remove designer!\nError message: " + dbhandler.get_error_message());
}

void RegisterManager::on_pushButtonAddSig_clicked()
{
    if (!authenticator_.can_add_signal())
    {
        QMessageBox::warning(this, "Add Signal", "You are not eligible to add signals!");
        return;
    }
    EditSignalDialog new_signal(chip_id_, block_id_, register_width_, msb_first_, this);
    if (new_signal.exec() == QDialog::Accepted && new_signal.add_signal())
    {
        int row = ui->tableSignal->rowCount();
        QVector<QVector<QString> > items;
        ui->tableSignal->insertRow(row);
        QVector<QString> values = {new_signal.get_signal_id(), new_signal.get_reg_sig_id(),
                                  new_signal.get_register_type_id(), new_signal.get_signal_name(),
                                  new_signal.get_width(), new_signal.get_signal_type(),
                                  new_signal.get_register_type(), new_signal.get_value(), new_signal.add_port() ? "1" : "0"};
        for (int i = 0; i < values.size(); i++)
            ui->tableSignal->setItem(row, i, new QTableWidgetItem(values[i]));
        ui->tableSignal->setCurrentCell(row, 0);

        if (new_signal.is_register_signal())
        {
            items.clear();
            DataBaseHandler dbhandler(gDBHost, gDatabase);
            dbhandler.show_items_inner_join({"block_register.reg_id", "block_register.reg_name"},
                                            {{{"block_sig_reg_partition_mapping", "reg_id"}, {"block_register", "reg_id"}}}, items,
                                            {{"block_sig_reg_partition_mapping.reg_sig_id", new_signal.get_reg_sig_id()}});
            if (items.size() == 0) return;
            QHash<QString, QString> reg_id2name;
            QSet<QString> existing_reg_ids;
            for (const auto& item : items) reg_id2name[item[0]] = REGISTER_NAMING.get_extended_name(item[1]);
            QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
            while (block_item->parent()->parent()) block_item = block_item->parent();
            refresh_block(block_item);
        }
    }
}


void RegisterManager::on_pushButtonRemoveSig_clicked()
{
    if (!authenticator_.can_remove_signal())
    {
        QMessageBox::warning(this, "Remove Signal", "You are not eligible to remove signals!");
        return;
    }
    int row = ui->tableSignal->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Signal",
                             "Are you sure you want to remove this signal?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString sig_id = ui->tableSignal->item(row, 0)->text(),
            reg_sig_id = ui->tableSignal->item(row, 1)->text();

    QString error_msg;
    if (!DataBaseHandler::remove_signal(sig_id, reg_sig_id, &error_msg))
    {
        QMessageBox::warning(this, "Remove Signal", QString("Removing signal failed\nError message: ") + error_msg);
        return;
    }

    if (reg_sig_id != "") ui->tableSigPart->setRowCount(0);
    ui->tableSignal->removeRow(row);
    QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
    while (block_item->parent()->parent()) block_item = block_item->parent();
    refresh_block(block_item);
}

void RegisterManager::on_pushButtonAddReg_clicked()
{
    if (!authenticator_.can_add_register())
    {
        QMessageBox::warning(this, "Add Register", "You are not eligible to add registers!");
        return;
    }
    EditRegisterDialog new_reg(chip_id_, block_id_, this);
    if (new_reg.exec() == QDialog::Accepted && new_reg.add_register())
    {
        int row = ui->tableRegister->rowCount();
        ui->tableRegister->insertRow(row);
        ui->tableRegister->setItem(row, 0, new QTableWidgetItem(new_reg.get_reg_id()));
        ui->tableRegister->setItem(row, 1, new QTableWidgetItem(new_reg.get_reg_name()));
        ui->tableRegister->setItem(row, 2, new QTableWidgetItem(new_reg.get_reg_type()));
        ui->tableRegister->setCurrentCell(row, 0);

        QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
        while (block_item->parent()->parent()) block_item = block_item->parent();
        QTreeWidgetItem* reg_item = new QTreeWidgetItem(block_item);
        reg_item->setText(0, new_reg.get_reg_name());
        reg_item->setText(1, new_reg.get_reg_id());
    }
}

void RegisterManager::on_pushButtonRemoveReg_clicked()
{
    if (!authenticator_.can_remove_register())
    {
        QMessageBox::warning(this, "Remove Signal", "You are not eligible to remove signals!");
        return;
    }
    int row = ui->tableRegister->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Register",
                             "Are you sure you want to remove this register?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString reg_id = ui->tableRegister->item(row, 0)->text();
    QString error_msg;
    if (!DataBaseHandler::remove_register(reg_id, &error_msg))
    {
        QMessageBox::warning(this, "Remove Register", QString("Removing register failed\nError message: ") + error_msg);
        return;
    }

    ui->tableRegPart->setRowCount(0);
    ui->tableRegister->removeRow(row);
    QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
    while (block_item->parent()->parent()) block_item = block_item->parent();
    for (int i = 0; i < block_item->childCount(); i++)
        if (block_item->child(i)->text(1) == reg_id) block_item->removeChild(block_item->child(i));
}


void RegisterManager::on_pushButtonAddSigPart_clicked()
{
    int row = ui->tableSignal->currentRow();
    if (row < 0 || ui->tableSignal->item(row, 1)->text() == "") return;


    QString reg_sig_id = ui->tableSignal->item(row, 1)->text(),
            reg_type_id = ui->tableSignal->item(row, 2)->text(),
            sig_id = ui->tableSignal->item(row, 0)->text(),
            sig_name = ui->tableSignal->item(row, 3)->text();
    int signal_width = ui->tableSignal->item(row, 4)->text().toInt();

    QVector<QString> item;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.show_one_item("chip_register_page", item, {"page_name"}, "ctrl_sig", sig_id);
    if (item.size() > 0)
    {
        QMessageBox::warning(this, "Add Signal Partition", "Cannot add partitions to this signal.\n\It is the control signal of register page "+ item[0] + "!");
        return;
    }

    EditSignalPartitionDialog new_sig_part(block_id_, reg_sig_id, reg_type_id, signal_width, register_width_, msb_first_, this);
    if (new_sig_part.exec() == QDialog::Accepted && new_sig_part.add_signal_partition())
    {
        QString sig_lsb = new_sig_part.get_signal_lsb();
        QString sig_msb = new_sig_part.get_signal_msb();
        QString reg_lsb = new_sig_part.get_register_lsb();
        QString reg_msb = new_sig_part.get_register_msb();
        QString reg_name = new_sig_part.get_register_name();
        QString reg_id = new_sig_part.get_register_id();

        QString sig_reg_part_mapping_id = new_sig_part.get_sig_reg_part_mapping_id();
        int row = ui->tableSigPart->rowCount();
        ui->tableSigPart->insertRow(row);
        ui->tableSigPart->setItem(row, 0, new QTableWidgetItem(sig_reg_part_mapping_id));
        QString sig_part = msb_first_ ? "<" + sig_msb + ":"+ sig_lsb + ">" : "<" + sig_lsb + ":"+ sig_msb + ">",
                reg_part = msb_first_ ? reg_name + "<" + reg_msb + ":"+ reg_lsb +">" : reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
        ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
        ui->tableSigPart->setItem(row, 2, new QTableWidgetItem(reg_part));

        QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
        while (block_item->parent()->parent()) block_item = block_item->parent();
        refresh_block(block_item);
    }
}

void RegisterManager::on_pushButtonRemoveSigPart_clicked()
{
    int row = ui->tableSigPart->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Signal Partition",
                             "Are you sure you want to remove this signal partition?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    QString sig_reg_part_mapping_id = ui->tableSigPart->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.delete_items("block_sig_reg_partition_mapping", "sig_reg_part_mapping_id", sig_reg_part_mapping_id))
    {
        ui->tableSigPart->removeRow(row);
        QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
        while (block_item->parent()->parent()) block_item = block_item->parent();
        refresh_block(block_item);
    }
    else
        QMessageBox::warning(this, "Remove Signal Partition", "Removing signal partition failed\nError message: "+ dbhandler.get_error_message());

}



void RegisterManager::init_db()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QString dbname;
    QVector<QVector<QString> > table_define;
    QString primary_key;
    QVector<QVector<QString> > foreign_keys;
    QVector<QString> unique_keys;
    QVector<QString> fields, values;

    dbhandler.create_database(gDatabase);
    // def_db_role
    dbname = "def_db_role";
    table_define = {{"db_role_id", "int", "not null auto_increment"},
                {"db_role", "varchar(20)", "not null"},
                {"add_user", "tinyint(1)", "not null"},
                {"remove_user", "tinyint(1)", "not null"},
                {"add_project", "tinyint(1)", "not null"},
                {"remove_project", "tinyint(1)", "not null"}};
    primary_key = "db_role_id";
    unique_keys = {"db_role"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"db_role", "add_user", "remove_user", "add_project", "remove_project"};
    values = {"super user", "1", "1", "1", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"standard user", "0", "0", "1", "0"};
    dbhandler.insert_item(dbname, fields, values);

    // def_register_type
    dbname = "def_register_type";
    table_define = {{"reg_type_id", "int", "not null auto_increment"},
                {"reg_type", "varchar(20)", "not null"},
                {"description", "varchar(255)", "not null"}};
    primary_key = "reg_type_id";
    unique_keys = {"reg_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"reg_type", "description"};
    values = {"R/W", "Register for standard control signals"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"RO", "Read-only register for data provided by the ASIC"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"W/SC", "Register clears after being written"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"R/SC", "Register clears after being read"};
    dbhandler.insert_item(dbname, fields, values);

    // def_signal_type
    dbname = "def_signal_type";
    table_define = {{"sig_type_id", "int", "not null auto_increment"},
                {"sig_type", "varchar(20)", "not null"},
                {"regable", "tinyint(1)", "not null"}};
    primary_key = "sig_type_id";
    unique_keys = {"sig_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);
    fields = {"sig_type", "regable"};
    values = {"Control Signal", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"Info Signal", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_A2A", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_A2D", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_D2A", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"TOP_D2D", "0"};
    dbhandler.insert_item(dbname, fields, values);


    // def_sig_reg_type_mapping
    dbname = "def_sig_reg_type_mapping";
    table_define = {{"mapping_id", "int", "not null auto_increment"},
                {"sig_type_id", "int", "not null"},
                {"reg_type_id", "int", "not null"}};
    primary_key = "mapping_id";
    foreign_keys = {{"sig_type_id", "def_signal_type", "sig_type_id"},
                    {"reg_type_id", "def_register_type", "reg_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);
    fields = {"sig_type_id", "reg_type_id"};
    dbhandler.insert_item(dbname, fields, {"1", "1"});
    dbhandler.insert_item(dbname, fields, {"2", "2"});
    dbhandler.insert_item(dbname, fields, {"1", "3"});
    dbhandler.insert_item(dbname, fields, {"2", "4"});

    // def_project_role
    dbname = "def_project_role";
    table_define = {{"project_role_id", "int", "not null auto_increment"},
                {"project_role", "varchar(20)", "not null"},
                {"add_block", "tinyint(1)", "not null"},
                {"remove_his_block", "tinyint(1)", "not null"},
                {"read_all_blocks", "tinyint(1)", "not null"},
                {"compile_project", "tinyint(1)", "not null"},
                {"add_chip_designer", "tinyint(1)", "not null"},
                {"remove_chip_designer", "tinyint(1)", "not null"},
                {"full_access_to_all_blocks", "tinyint(1)", "not null"}};
    primary_key = "project_role_id";
    unique_keys = {"project_role"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"project_role", "add_block", "remove_his_block", "read_all_blocks", "compile_project", "add_chip_designer", "remove_chip_designer", "full_access_to_all_blocks"};
    values = {"admin", "1", "1", "1", "1", "1", "1", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"standard designer", "1", "1", "1", "1", "0", "0", "0"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"limited designer", "0", "0", "0", "0", "0", "0", "0"};
    dbhandler.insert_item(dbname, fields, values);

    // def_doc_type
    dbname = "def_doc_type";
    table_define = {{"doc_type_id", "int", "not null auto_increment"},
                {"doc_type", "varchar(20)", "not null"}};
    primary_key = "doc_type_id";
    unique_keys = {"doc_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);
    dbhandler.insert_item(dbname, {"doc_type"}, {"Text"});
    dbhandler.insert_item(dbname, {"doc_type"}, {"Image"});
    dbhandler.insert_item(dbname, {"doc_type"}, {"Table"});


    // global_user
    dbname = "global_user";
    table_define = {{"user_id", "int", "not null auto_increment"},
                {"username", "varchar(256)", "not null"},
                {"password", "varchar(256)", "not null"},
                {"db_role_id", "int", "not null"}};
    primary_key = "user_id";
    foreign_keys = {{"db_role_id", "def_db_role", "db_role_id"}};
    unique_keys = {"username"};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, &unique_keys);

    dbname = "chip_chip";
    table_define = {{"chip_id", "int", "not null auto_increment"},
                {"chip_name", "varchar(256)", "not null"},
                {"owner", "int", "not null"},
                {"register_width", "int", "not null"},
                {"address_width", "int", "not null"},
                {"msb_first", "tinyint(1)", "not null"}};
    primary_key = "chip_id";
    foreign_keys = {{"owner", "global_user", "user_id"}};
    unique_keys = {"chip_name"};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, &unique_keys);

    dbname = "chip_designer";
    table_define = {{"chip_designer_id", "int", "not null auto_increment"},
                {"chip_id", "int", "not null"},
                {"user_id", "int", "not null"},
                {"project_role_id", "int", "not null"}};
    primary_key = "chip_designer_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"user_id", "global_user", "user_id"},
                    {"project_role_id", "def_project_role", "project_role_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "block_system_block";
    table_define = {{"block_id", "int", "not null auto_increment"},
                {"block_name", "varchar(256)", "not null"},
                {"abbreviation", "varchar(32)", "not null"},
                {"chip_id", "int", "not null"},
                {"responsible", "int", "not null"},
                {"start_address", "varchar(256)"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "block_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"responsible", "global_user", "user_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "signal_signal";
    table_define = {{"sig_id", "int", "not null auto_increment"},
                {"sig_name", "varchar(256)", "not null"},
                {"block_id", "int", "not null"},
                {"width", "int", "not null"},
                {"sig_type_id", "int", "not null"},
                {"add_port", "tinyint(1)", "not null"}};
    primary_key = "sig_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"sig_type_id", "def_signal_type", "sig_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "chip_register_page";
    table_define = {{"page_id", "int", "not null auto_increment"},
                {"page_name", "varchar(256)", "not null"},
                {"chip_id", "int", "not null"},
                {"ctrl_sig", "int", "not null"},
                {"count", "int", "not null"}};
    primary_key = "page_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"ctrl_sig", "signal_signal", "sig_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "block_register";
    table_define = {{"reg_id", "int", "not null auto_increment"},
                {"reg_name", "varchar(256)", "not null"},
                {"block_id", "int", "not null"},
                {"reg_type_id", "int", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"} };
    primary_key = "reg_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"reg_type_id", "def_register_type", "reg_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "chip_register_page_content";
    table_define = {{"page_content_id", "int", "not null auto_increment"},
                {"page_id", "int", "not null"},
                {"reg_id", "int", "not null"}};
    primary_key = "page_content_id";
    foreign_keys = {{"page_id", "chip_register_page", "page_id"},
                    {"reg_id", "block_register", "reg_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "signal_reg_signal";
    table_define = {{"reg_sig_id", "int", "not null auto_increment"},
                {"sig_id", "int", "not null"},
                {"init_value", "varchar(256)", "not null"},
                {"reg_type_id", "int", "not null"}};
    primary_key = "reg_sig_id";
    foreign_keys = {{"sig_id", "signal_signal", "sig_id"},
                    {"reg_type_id", "def_register_type", "reg_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "block_sig_reg_partition_mapping";
    table_define = {{"sig_reg_part_mapping_id", "int", "not null auto_increment"},
                {"reg_sig_id", "int", "not null"},
                {"sig_lsb", "int", "not null"},
                {"sig_msb", "int", "not null"},
                {"reg_id", "int", "not null"},
                {"reg_lsb", "int", "not null"},
                {"reg_msb", "int", "not null"}};
    primary_key = "sig_reg_part_mapping_id";
    foreign_keys = {{"reg_sig_id", "signal_reg_signal", "reg_sig_id"},
                    {"reg_id", "block_register", "reg_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "doc_chip";
    table_define = {{"chip_doc_id", "int", "not null auto_increment"},
                {"chip_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "chip_doc_id";
    foreign_keys = {{"chip_id", "chip_chip", "chip_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "doc_block";
    table_define = {{"block_doc_id", "int", "not null auto_increment"},
                {"block_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "block_doc_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "doc_register";
    table_define = {{"register_doc_id", "int", "not null auto_increment"},
                {"reg_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "register_doc_id";
    foreign_keys = {{"reg_id", "block_register", "reg_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "doc_signal";
    table_define = {{"signal_doc_id", "int", "not null auto_increment"},
                {"sig_id", "int", "not null"},
                {"doc_type_id", "int", "not null"},
                {"content", "text", "not null"},
                {"prev", "int", "not null"},
                {"next", "int", "not null"}};
    primary_key = "signal_doc_id";
    foreign_keys = {{"sig_id", "signal_signal", "sig_id"},
                    {"doc_type_id", "def_doc_type", "doc_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    // add admin
    fields = {"username", "password", "db_role_id"};
    values = {"admin", "admin", "1"};
    dbhandler.insert_item("global_user", fields, values);

}

void RegisterManager::clear_db()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    dbhandler.delete_database(gDatabase);
}

void RegisterManager::on_tableSignal_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString sig_id = ui->tableSignal->item(row, 0)->text();
    QString reg_sig_id = ui->tableSignal->item(row, 1)->text();
    EditSignalDialog edit_signal(chip_id_, block_id_, sig_id, reg_sig_id, register_width_, msb_first_, authenticator_.can_add_signal() && authenticator_.can_remove_signal(), this);
    if (edit_signal.exec() == QDialog::Accepted && edit_signal.edit_signal())
    {
        QVector<QString> values = {ui->tableSignal->item(row, 0)->text(), edit_signal.get_reg_sig_id(),
                                   edit_signal.get_register_type_id(), edit_signal.get_signal_name(),
                                   edit_signal.get_width(), edit_signal.get_signal_type(),
                                   edit_signal.get_register_type(), edit_signal.get_value(), edit_signal.add_port() ? "1" : "0"};
        for (int i = 0; i < values.size(); i++) ui->tableSignal->item(row, i)->setText(values[i]);
        QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
        while (block_item->parent()->parent()) block_item = block_item->parent();
        refresh_block(block_item);

    }
    emit(ui->tableSignal->currentCellChanged(row, column, row, column));
}

void RegisterManager::on_tableRegister_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString reg_id = ui->tableRegister->item(row, 0)->text();
    EditRegisterDialog edit_reg(chip_id_, block_id_, reg_id, authenticator_.can_add_register() && authenticator_.can_remove_register(), this);
    if (edit_reg.exec() == QDialog::Accepted && edit_reg.edit_register())
    {
        ui->tableRegister->item(row, 1)->setText(edit_reg.get_reg_name());
        QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
        while (block_item->parent()->parent()) block_item = block_item->parent();
        refresh_block(block_item);
    }
}

void RegisterManager::on_tableSystem_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString responsible = ui->tableSystem->item(row, 3)->text();
    bool enabled = authenticator_.can_add_block() && ((authenticator_.can_remove_his_block() && responsible == username_) || authenticator_.can_fully_access_all_blocks());
    QString block_id = ui->tableSystem->item(row, 0)->text();
    QString old_block_name = ui->tableSystem->item(row, 1)->text();
    EditSystemBlockDialog edit_sys(chip_id_, block_id, address_width_, authenticator_.can_add_chip_designer(), enabled, this);
    if (edit_sys.exec() == QDialog::Accepted && edit_sys.edit_system_block())
    {
        ui->tableSystem->item(row, 1)->setText(edit_sys.get_block_name());
        ui->tableSystem->item(row, 2)->setText(edit_sys.get_block_abbr());
        ui->tableSystem->item(row, 3)->setText(edit_sys.get_responsible());
        ui->tableSystem->item(row, 4)->setText(edit_sys.get_start_addr());
        QTreeWidgetItem* top = ui->treeWidgetBlock->topLevelItem(0);
        top->child(row)->setText(0, edit_sys.get_block_name());
    }
    if (edit_sys.designer_added()) display_designers();

}


void RegisterManager::on_pushButtonAddDoc_clicked()
{    
    QTreeWidgetItem* current = ui->treeWidgetBlock->currentItem();
    QTreeWidgetItem* block_item = current;
    while (block_item->parent()->parent()) block_item = block_item->parent();

    ui->documentEditor->clear();
    QString block_id, reg_id, sig_id;
    EditDocumentDialog::DOCUMENT_LEVEL level;
    if (current->parent() == block_item)   // register
    {
        level = EditDocumentDialog::REGISTER;
        ui->documentEditor->set_register_id(current->text(1));
    }
    else if (current->parent() && current->parent()->parent() == block_item)  // signal
    {
        level = EditDocumentDialog::SIGNAL;
        ui->documentEditor->set_signal_id(current->text(1));
    }
    else // block
    {
        level = EditDocumentDialog::BLOCK;
        ui->documentEditor->set_block_id(current->text(1));
    }
    ui->documentEditor->set_level(level);
    ui->documentEditor->set_mode(DIALOG_MODE::ADD);
    ui->documentEditor->setVisible(true);

}

void RegisterManager::on_pushButtonRemoveDoc_clicked()
{
    int row = ui->tableDoc->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Document",
                             "Are you sure you want to remove this document?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    QString doc_id = ui->tableDoc->item(row, 0)->text();

    QString table, id_field;
    QTreeWidgetItem* current = ui->treeWidgetBlock->currentItem();
    QTreeWidgetItem* block_item = current;
    while (block_item->parent()->parent()) block_item = block_item->parent();

    DataBaseHandler dbhandler(gDBHost, gDatabase);

    if (current->parent() == block_item)   // register
    {
        table = "doc_register";
        id_field = "register_doc_id";
    }
    else if (current->parent() && current->parent()->parent() == block_item)  // signal
    {
        table = "doc_signal";
        id_field = "signal_doc_id";
    }
    else // block
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

void RegisterManager::on_tableSystem_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow < 0) ui->pushButtonRemoveSys->setEnabled(false);
    else {
        QString responsible = ui->tableSystem->item(currentRow, 3)->text();
        ui->pushButtonRemoveSys->setEnabled((authenticator_.can_remove_his_block() && responsible == username_) || authenticator_.can_fully_access_all_blocks());
    }
}

void RegisterManager::on_tableSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveSigPart->setEnabled(authenticator_.can_edit_signal_partition() && currentRow >= 0);
}

void RegisterManager::on_pushButtonAddRegPage_clicked()
{
    if (!authenticator_.can_fully_access_all_blocks())
    {
        QMessageBox::warning(this, "Remove Register Page", "You are not eligible to add register pages!");
        return;
    }
    EditRegisterPageDialog edit_page(chip_id_, this);
    if (edit_page.exec() == QDialog::Accepted && edit_page.add_register_page())
    {
        int row = ui->tableRegPage->rowCount();
        ui->tableRegPage->insertRow(row);
        QVector<QString> values = {edit_page.get_register_page_id(), edit_page.get_register_page_name(), edit_page.get_control_signal_name(), edit_page.get_page_count()};
        for (int i = 0; i < values.size(); i++)
            ui->tableRegPage->setItem(row, i, new QTableWidgetItem(values[i]));
    }
}

void RegisterManager::on_pushButtonRemoveRegPage_clicked()
{
    if (!authenticator_.can_fully_access_all_blocks())
    {
        QMessageBox::warning(this, "Remove Register Page", "You are not eligible to remove register pages!");
        return;
    }
    if (QMessageBox::warning(this, "Remove Register Page", "Are you sure to remove this register page?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        return;
    int row = ui->tableRegPage->currentRow();
    if (row < 0) return;
    QString page_id = ui->tableRegPage->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.delete_items("chip_register_page_content", "page_id", page_id) &&
            dbhandler.delete_items("chip_register_page", "page_id", page_id))
    {
        ui->tableRegPage->removeRow(row);
    }
    else {
        QMessageBox::warning(this, "Remove Register Page", "Unable to remove this register page!\nError massage: " + dbhandler.get_error_message());
    }
}

void RegisterManager::on_tableRegPage_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    if (!authenticator_.can_fully_access_all_blocks())
    {
        QMessageBox::warning(this, "Remove Register Page", "You are not eligible to edit register pages!");
        return;
    }
    QString page_id = ui->tableRegPage->item(row, 0)->text();
    EditRegisterPageDialog edit_page(chip_id_, page_id, this);
    if (edit_page.exec() == QDialog::Accepted && edit_page.edit_register_page())
    {
        QVector<QString> values = {edit_page.get_register_page_id(), edit_page.get_register_page_name(), edit_page.get_control_signal_name(), edit_page.get_page_count()};
        for (int i = 0; i < values.size(); i++)
            ui->tableRegPage->setItem(row, i, new QTableWidgetItem(values[i]));
    }

}

void RegisterManager::on_tableDesigner_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString chip_designer = ui->tableDesigner->item(row, 1)->text(),
            project_role = ui->tableDesigner->item(row, 2)->text();
    EditChipDesignerDialog edit_designer(chip_id_, chip_designer, project_role,
                                         authenticator_.can_remove_chip_designer() && authenticator_.can_add_chip_designer() && \
                                         username_ != chip_designer && chip_owner_ != chip_designer,
                                         this);
    if (edit_designer.exec() == QDialog::Accepted && edit_designer.edit_designer())
    {
        ui->tableDesigner->item(row, 2)->setText(edit_designer.get_project_role());
    }
}

bool RegisterManager::search(QTreeWidgetItem* item, const QString &s, bool visible)
{
    if (!item) return false;
    visible |= item->text(0).contains(s);
    bool res = false;
    for (int i = 0; i < item->childCount(); i++)
    {
        res |= search(item->child(i), s, visible);
    }
    item->setHidden(!res && !visible);
    item->setExpanded(res && s != "");
    QFont font = item->font(0);
    QString color;
    font.setBold(s != "" && item->text(0).contains(s));
    font.setItalic(s != "" && item->text(0).contains(s));
    color = (s != "" && item->text(0).contains(s)) ? "yellow" : "white";
    item->setFont(0, font);
    item->setBackgroundColor(0, QColor(color));
    return res || item->text(0).contains(s);
}

void RegisterManager::on_lineEditSearch_editingFinished()
{
    search(ui->treeWidgetBlock->topLevelItem(0), ui->lineEditSearch->text(), false);
    ui->treeWidgetBlock->topLevelItem(0)->setExpanded(true);
}

void RegisterManager::on_tableDoc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (ui->stackedWidgetDoc->currentIndex() == 0 || currentRow < 0) return;
    ui->pushButtonRemoveDoc->setEnabled(authenticator_.can_edit_document() && currentRow>=0);
}

void RegisterManager::on_tableDoc_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    ui->documentEditor->clear();
    QTreeWidgetItem* current = ui->treeWidgetBlock->currentItem();
    QTreeWidgetItem* block_item = current;
    while (block_item->parent()->parent()) block_item = block_item->parent();

    EditDocumentDialog::DOCUMENT_LEVEL level;
    if (current->parent() == block_item)   // register
        level = EditDocumentDialog::REGISTER;
    else if (current->parent() && current->parent()->parent() == block_item)  // signal
        level = EditDocumentDialog::SIGNAL;
    else // block
        level = EditDocumentDialog::BLOCK;

    ui->documentEditor->set_level(level);
    ui->documentEditor->set_mode(DIALOG_MODE::EDIT);
    ui->documentEditor->setEnabled(authenticator_.can_edit_document());
    ui->documentEditor->setVisible(true);

    QString doc_id = ui->tableDoc->item(row, 0)->text(),
            type = ui->tableDoc->item(row, 1)->text(),
            content = ui->tableDoc->item(row, 2)->text();
    ui->documentEditor->set_content(doc_id, type, content);
}

void RegisterManager::on_stackedWidgetDoc_currentChanged(int index)
{
    if (index == 1) ui->pushButtonRemoveDoc->setEnabled(false);
}


void RegisterManager::on_document_added()
{
    int row = ui->tableDoc->rowCount();
    ui->tableDoc->insertRow(row);
    ui->tableDoc->setItem(row, 0, new QTableWidgetItem(ui->documentEditor->get_doc_id()));
    ui->tableDoc->setItem(row, 1, new QTableWidgetItem(ui->documentEditor->get_document_type()));
    ui->tableDoc->setItem(row, 2, new QTableWidgetItem(ui->documentEditor->get_content()));
    ui->tableDoc->resizeRowToContents(row);
}

void RegisterManager::on_document_edited()
{
    int row = ui->tableDoc->currentRow();
    ui->tableDoc->item(row, 1)->setText(ui->documentEditor->get_document_type());
    ui->tableDoc->item(row, 2)->setText(ui->documentEditor->get_content());
    ui->tableDoc->resizeRowToContents(row);
}

void RegisterManager::on_tableDesigner_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveDesigner->setEnabled(currentRow >= 0 && authenticator_.can_remove_chip_designer());
}

void RegisterManager::on_lineEditSearch_textChanged(const QString &pattern)
{
    search(ui->treeWidgetBlock->topLevelItem(0), ui->lineEditSearch->text(), false);
    ui->treeWidgetBlock->topLevelItem(0)->setExpanded(true);
}

void RegisterManager::on_tableRegPage_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveRegPage->setEnabled(currentRow >= 0 && authenticator_.can_fully_access_all_blocks());
}
