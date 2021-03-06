#include "chip_editor_view.h"
#include "ui_chip_editor_view.h"
#include "global_variables.h"
#include "database_handler.h"
#include "data_utils.h"
#include "edit_system_block_dialog.h"
#include "edit_signal_dialog.h"
#include "edit_register_dialog.h"
#include "edit_chip_designer_dialog.h"
#include "edit_signal_partition_dialog.h"
#include "edit_register_page_dialog.h"
#include "edit_chip_dialog.h"
#include "database_utils.h"
#include <QMimeData>
#include <QDropEvent>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QMenu>

ChipEditorView::ChipEditorView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChipEditorView)
{

    ui->setupUi(this);
    ui->tabWidget->blockSignals(true);
    ui->stackedWidgetChipEditor->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->tableBlock->setColumnHidden(0, true);
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

    QVector<QWidget*> widgets = {ui->pushButtonAddBlock, ui->pushButtonRemoveBlock, ui->pushButtonAddReg, ui->pushButtonRemoveReg,
                                 ui->pushButtonAddSig, ui->pushButtonRemoveSig, ui->pushButtonAddSigPart, ui->pushButtonRemoveSigPart,
                                 ui->pushButtonAddDesigner, ui->pushButtonRemoveDesigner, ui->pushButtonAddRegPage,
                                 ui->pushButtonRemoveRegPage, ui->pushButtonEditChip};
    for (QWidget* widget : widgets) widget->setEnabled(false);

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

    connect(actionAdd_, SIGNAL(triggered()), this, SLOT(to_add()));
    connect(actionEdit_, SIGNAL(triggered()), this, SLOT(to_edit()));
    connect(actionRemove_, SIGNAL(triggered()), this, SLOT(to_remove()));
    connect(actionRefresh_, SIGNAL(triggered()), this, SLOT(to_refresh()));

    ui->tableBlock->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableDesigner->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableRegPage->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableRegPage->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);

    ui->tableSignal->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableSignal->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableSigPart->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);

    ui->tableRegister->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    ui->tableRegPart->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);

    for (QTableWidget* t: {ui->tableBlock, ui->tableDesigner, ui->tableRegPage, ui->tableSignal, ui->tableRegister})
    {
        t->verticalHeader()->setVisible(true);
        t->verticalHeader()->setMinimumWidth(25);
    }
    ui->tableSignal->setSortingEnabled(true);
    ui->tableSignal->sortByColumn(3, Qt::SortOrder::AscendingOrder);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("ui");
    ui->splitterBlocks->setSizes({settings.value("table_block_height").toInt(),
                                  settings.value("table_designer_height").toInt()});
    ui->splitterDesignerRegPage->setSizes({settings.value("table_designer_width").toInt(),
                                  settings.value("table_register_page_width").toInt()});
    ui->splitterSignal->setSizes({settings.value("table_signal_height").toInt(),
                                  settings.value("table_signal_partition_height").toInt()});
    ui->splitterRegister->setSizes({settings.value("table_register_height").toInt(),
                                  settings.value("table_register_partition_height").toInt()});
}

ChipEditorView::~ChipEditorView()
{
    delete ui;
}

void ChipEditorView::login(const QString &username, const QString &user_id)
{
    username_ = username;
    user_id_ = user_id;
}

void ChipEditorView::open_chip(const QString &chip, const QString &chip_id, const QString &chip_owner, const QString &chip_owner_id, int register_width, int address_width, bool msb_first)
{
    chip_name_ = chip;
    chip_id_ = chip_id;
    chip_owner_ = chip_owner;
    chip_owner_id_ = chip_owner_id;
    register_width_ = register_width;
    address_width_ = address_width;
    msb_first_ = msb_first;
    open_chip();
}

void ChipEditorView::close_chip()
{
    QVector<QTableWidget*> tables = {ui->tableChipBasics, ui->tableBlock, ui->tableDesigner, ui->tableRegPage,
                                     ui->tableSignal, ui->tableSigPart, ui->tableRegister, ui->tableRegPart};
    for (QTableWidget* table : tables) table->setRowCount(0);
    QVector<QWidget*> widgets = {ui->pushButtonAddBlock, ui->pushButtonRemoveBlock, ui->pushButtonAddReg, ui->pushButtonRemoveReg,
                                 ui->pushButtonAddSig, ui->pushButtonRemoveSig, ui->pushButtonAddSigPart,
                                ui->pushButtonRemoveSigPart, ui->pushButtonAddDesigner, ui->pushButtonRemoveDesigner,
                                ui->pushButtonAddRegPage, ui->pushButtonRemoveRegPage, ui->pushButtonEditChip};
    for (QWidget* widget : widgets) widget->setEnabled(false);
    ui->stackedWidgetChipEditor->setCurrentIndex(0);
    ui->tabWidget->blockSignals(true);
}

void ChipEditorView::set_block_id(const QString &block_id)
{
    previous_block_id_ = block_id_;
    block_id_ = block_id;
}

void ChipEditorView::set_authenticator(Authenticator* authenticator)
{
    authenticator_ = authenticator;
}

void ChipEditorView::display_chip_level_info()
{
    ui->stackedWidgetChipEditor->setCurrentIndex(0);
    display_chip_basics();
    display_system_blocks();
    display_designers();
    display_register_pages();
}

void ChipEditorView::display_system_level_info(const QString& reg_id, const QString& sig_id)
{
    int prev_tab_index = ui->tabWidget->currentIndex(), prev_stack_index = ui->stackedWidgetChipEditor->currentIndex();
    ui->stackedWidgetChipEditor->setCurrentIndex(1);
    set_install_event_filter_register();

    if (sig_id != "") ui->tabWidget->setCurrentIndex(0);
    else if (reg_id != "") ui->tabWidget->setCurrentIndex(1);

    if (ui->tabWidget->currentIndex() == 0)
    {
        if (!(prev_stack_index == 1 && prev_tab_index == 0 && block_id_ == previous_block_id_))
        display_signals();
    }
    else if (!(prev_stack_index == 1 && prev_tab_index == 1 && block_id_ == previous_block_id_))
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
}

void ChipEditorView::on_tableBlock_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableBlock->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_add_block() && ((authenticator_->can_remove_responsible_block() && ui->tableBlock->item(current->row(), 3)->text() == username_) || authenticator_->can_fully_access_all_blocks()));
    actionAdd_->setEnabled(authenticator_->can_add_block());
    actionRemove_->setEnabled(current && ((authenticator_->can_remove_responsible_block() && ui->tableBlock->item(current->row(), 3)->text() == username_) || authenticator_->can_fully_access_all_blocks()));
    context_menu_->popup(ui->tableBlock->viewport()->mapToGlobal(pos));
}

void ChipEditorView::on_tableDesigner_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableDesigner->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_add_chip_designer() && authenticator_->can_remove_chip_designer());
    actionAdd_->setEnabled(authenticator_->can_add_chip_designer());
    actionRemove_->setEnabled(current && authenticator_->can_remove_chip_designer());
    context_menu_->popup(ui->tableDesigner->viewport()->mapToGlobal(pos));
}

void ChipEditorView::on_tableRegPage_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableRegPage->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_fully_access_all_blocks());
    actionAdd_->setEnabled(authenticator_->can_fully_access_all_blocks());
    actionRemove_->setEnabled(current && authenticator_->can_fully_access_all_blocks());
    context_menu_->popup(ui->tableRegPage->viewport()->mapToGlobal(pos));
}

void ChipEditorView::on_tableSignal_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableSignal->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_add_signal() && authenticator_->can_remove_signal());
    actionAdd_->setEnabled(authenticator_->can_add_signal());
    actionRemove_->setEnabled(current && authenticator_->can_remove_signal());
    context_menu_->popup(ui->tableSignal->viewport()->mapToGlobal(pos));
}

void ChipEditorView::on_tableRegister_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableRegister->itemAt(pos);
    actionEdit_->setEnabled(current && authenticator_->can_add_register() && authenticator_->can_remove_register());
    actionAdd_->setEnabled(authenticator_->can_add_register());
    actionRemove_->setEnabled(current && authenticator_->can_remove_register());
    context_menu_->popup(ui->tableRegister->viewport()->mapToGlobal(pos));
}

void ChipEditorView::on_tableSigPart_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableSigPart->itemAt(pos);
    actionEdit_->setEnabled(false);
    actionAdd_->setEnabled(authenticator_->can_edit_signal_partition());
    actionRemove_->setEnabled(current && authenticator_->can_edit_signal_partition());
    context_menu_->popup(ui->tableSigPart->viewport()->mapToGlobal(pos));
}

void ChipEditorView::on_tableRegPart_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem *current = ui->tableRegPart->itemAt(pos);
    // DO NOTHING
    /*
    actionEdit_->setEnabled(current && authenticator_->can_edit_register_partition());
    actionAdd_->setEnabled(current && authenticator_->can_edit_register_partition());
    actionRemove_->setEnabled(current && authenticator_->can_edit_register_partition());
    context_menu_->popup(ui->tableRegPart->viewport()->mapToGlobal(pos));
    */
}

void ChipEditorView::to_add()
{
    if (ui->tableBlock->hasFocus()) on_pushButtonAddBlock_clicked();
    else if (ui->tableDesigner->hasFocus()) on_pushButtonAddDesigner_clicked();
    else if (ui->tableRegPage->hasFocus()) on_pushButtonAddRegPage_clicked();
    else if (ui->tableSignal->hasFocus()) on_pushButtonAddSig_clicked();
    else if (ui->tableSigPart->hasFocus()) on_pushButtonAddSigPart_clicked();
    else if (ui->tableRegister->hasFocus()) on_pushButtonAddReg_clicked();
    // else if (ui->tableRegPart->hasFocus())
}

void ChipEditorView::to_edit()
{
    if (ui->tableBlock->hasFocus()) on_tableBlock_cellDoubleClicked(ui->tableBlock->currentRow(), ui->tableBlock->currentColumn());
    else if (ui->tableDesigner->hasFocus()) on_tableDesigner_cellDoubleClicked(ui->tableDesigner->currentRow(), ui->tableDesigner->currentColumn());
    else if (ui->tableRegPage->hasFocus()) on_tableRegPage_cellDoubleClicked(ui->tableRegPage->currentRow(), ui->tableRegPage->currentColumn());
    else if (ui->tableSignal->hasFocus()) on_tableSignal_cellDoubleClicked(ui->tableSignal->currentRow(), ui->tableSignal->currentColumn());
    //else if (ui->tableSigPart->hasFocus()) on_tableSigPart_cellDoubleClicked(ui->tableSigPart->currentRow(), ui->tableSigPart->currentColumn());
    else if (ui->tableRegister->hasFocus()) on_tableRegister_cellDoubleClicked(ui->tableRegister->currentRow(), ui->tableRegister->currentColumn());
    // else if (ui->tableRegPart->hasFocus())
}

void ChipEditorView::to_remove()
{
    if (ui->tableBlock->hasFocus()) on_pushButtonRemoveBlock_clicked();
    else if (ui->tableDesigner->hasFocus()) on_pushButtonRemoveDesigner_clicked();
    else if (ui->tableRegPage->hasFocus()) on_pushButtonRemoveRegPage_clicked();
    else if (ui->tableSignal->hasFocus()) on_pushButtonRemoveSig_clicked();
    else if (ui->tableSigPart->hasFocus()) on_pushButtonRemoveSigPart_clicked();
    else if (ui->tableRegister->hasFocus()) on_pushButtonRemoveReg_clicked();
    // else if (ui->tableRegPart->hasFocus())
}

void ChipEditorView::to_refresh()
{
    if (ui->tableBlock->hasFocus()) display_system_blocks();
    else if (ui->tableDesigner->hasFocus()) display_designers();
    else if (ui->tableRegPage->hasFocus()) display_register_pages();
    else if (ui->tableSignal->hasFocus()) display_signals();
    else if (ui->tableSigPart->hasFocus()) display_signal_partitions();
    else if (ui->tableRegister->hasFocus()) display_registers();
    else if (ui->tableRegPart->hasFocus()) display_register_partitions();
}

void ChipEditorView::on_tabWidget_currentChanged(int index)
{
    if (index == 0) display_signals();
    else if (index == 1) display_registers();
}

void ChipEditorView::on_pushButtonEditChip_clicked()
{
    EditChipDialog edit_chip(chip_id_,
                             ui->tableChipBasics->item(0, 0)->text(),
                             ui->tableChipBasics->item(0, 1)->text(),
                             ui->tableChipBasics->item(0, 2)->text().toInt(),
                             ui->tableChipBasics->item(0, 3)->text().toInt(),
                             ui->tableChipBasics->item(0, 4)->text() == "true" ? true : false);
    if (edit_chip.exec() == QDialog::Accepted && edit_chip.edit_chip())
    {
        emit(chip_basics_edited(edit_chip.get_chip_name(), edit_chip.get_owner(), edit_chip.get_owner_id(), edit_chip.get_register_width(), edit_chip.get_address_width(), edit_chip.msb_first()));
    }
}

void ChipEditorView::on_pushButtonAddBlock_clicked()
{
    if (!authenticator_->can_add_block())
    {
        QMessageBox::warning(this, "New System Block", "You are not eligible to add system blocks!");
        return;
    }
    EditSystemBlockDialog new_system(chip_id_, address_width_, authenticator_->can_add_chip_designer(), this);
    if (new_system.exec() == QDialog::Accepted && new_system.add_system_block())
    {
        int row = ui->tableBlock->rowCount();
        ui->tableBlock->insertRow(row);
        ui->tableBlock->setItem(row, 1, new QTableWidgetItem(new_system.get_block_name()));
        ui->tableBlock->setItem(row, 2, new QTableWidgetItem(new_system.get_block_abbr()));
        ui->tableBlock->setItem(row, 3, new QTableWidgetItem(new_system.get_responsible()));
        ui->tableBlock->setItem(row, 4, new QTableWidgetItem(new_system.get_start_addr()));
        ui->tableBlock->setItem(row, 5, new QTableWidgetItem("0"));

        previous_block_id_ = block_id_;
        block_id_ = new_system.get_block_id();
        block_name_ = new_system.get_block_name();
        ui->tableBlock->setItem(row, 0, new QTableWidgetItem(block_id_));
        emit(block_added(block_id_, block_name_, new_system.get_block_abbr(), new_system.get_responsible_id()));
        ui->tableBlock->setCurrentCell(row, 0);
    }
    if (new_system.designer_added()) display_designers();
}

void ChipEditorView::on_pushButtonAddDesigner_clicked()
{
    if (!authenticator_->can_add_chip_designer())
    {
        QMessageBox::warning(this, "Add Chip Designer", "You are not eligible to add chip designers!");
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

void ChipEditorView::on_pushButtonAddRegPage_clicked()
{
    if (!authenticator_->can_fully_access_all_blocks())
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

void ChipEditorView::on_pushButtonAddSig_clicked()
{
    if (!authenticator_->can_add_signal())
    {
        QMessageBox::warning(this, "Add Signal", "You are not eligible to add signals!");
        return;
    }
    EditSignalDialog new_signal(chip_id_, block_id_, register_width_, msb_first_, this);
    if (new_signal.exec() == QDialog::Accepted && new_signal.add_signal())
    {
        ui->tableSignal->setSortingEnabled(false);
        int row = ui->tableSignal->rowCount();
        QVector<QVector<QString> > items;
        ui->tableSignal->insertRow(row);
        QVector<QString> values = {new_signal.get_signal_id(), new_signal.get_reg_sig_id(),
                                   new_signal.get_register_type_id(), new_signal.get_signal_name(),
                                   new_signal.get_width(), new_signal.get_signal_type(),
                                   new_signal.get_register_type(),
                                   new_signal.is_register_signal() ? new_signal.get_value() : "",
                                   new_signal.add_port() ? "true" : "false"};
        for (int i = 0; i < values.size(); i++)
            ui->tableSignal->setItem(row, i, new QTableWidgetItem(values[i]));
        ui->tableSignal->setCurrentCell(row, 0);

        if (new_signal.is_register_signal())
            emit(to_refresh_navigator_block());
        ui->tableSignal->setSortingEnabled(true);
    }
}

void ChipEditorView::on_pushButtonAddReg_clicked()
{
    if (!authenticator_->can_add_register())
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
        ui->tableRegister->setItem(row, 3, new QTableWidgetItem(new_reg.get_address()));
        ui->tableRegister->setCurrentCell(row, 0);
        emit(to_refresh_navigator_block());
    }
}

void ChipEditorView::on_pushButtonAddSigPart_clicked()
{
    int row = ui->tableSignal->currentRow();
    if (row < 0 || ui->tableSignal->item(row, 1)->text() == "") return;


    QString reg_sig_id = ui->tableSignal->item(row, 1)->text(),
            reg_type_id = ui->tableSignal->item(row, 2)->text(),
            sig_id = ui->tableSignal->item(row, 0)->text(),
            sig_name = ui->tableSignal->item(row, 3)->text();
    int signal_width = ui->tableSignal->item(row, 4)->text().toInt();

    EditSignalPartitionDialog new_sig_part(block_id_, sig_id, reg_sig_id, reg_type_id, signal_width, register_width_, msb_first_, this);
    if (new_sig_part.exec() == QDialog::Accepted && new_sig_part.add_signal_partition())
    {
        display_signal_partitions();
        emit(to_refresh_navigator_block());
    }
}

void ChipEditorView::on_pushButtonRemoveBlock_clicked()
{
    int row = ui->tableBlock->currentRow();
    if (row < 0) return;
    QString responsible = ui->tableBlock->item(row, 3)->text();
    if (!((authenticator_->can_remove_responsible_block() && responsible == username_) || authenticator_->can_fully_access_all_blocks()))
    {
        QMessageBox::warning(this, "Remove System Block", "You are not eligible to remove this system block!");
        return;
    }

    if (QMessageBox::warning(this,
                         "Remove System Block",
                         "Are you sure you want to remove this system blcok?\nAll registers, signals and documents belonging to this system block will also be removed!\nThis operation is not reversible!",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString block_id = ui->tableBlock->item(row, 0)->text();
    if (!DatabaseUtils::remove_block(block_id))
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove System Block", "Unable to remove the system block.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    DataBaseHandler::commit();
    ui->tableBlock->removeRow(row);
    emit(block_removed(row));
}

void ChipEditorView::on_pushButtonRemoveDesigner_clicked()
{
    if (!authenticator_->can_remove_chip_designer())
    {
        QMessageBox::warning(this, "Remove Chip Designer", "You are not eligible to remove chip designers!");
        return;
    }
    int row = ui->tableDesigner->currentRow();
    if (row < 0) return;
    QString designer_id = ui->tableDesigner->item(row, 0)->text(),
            username = ui->tableDesigner->item(row, 1)->text();
    if (username == username_)
    {
        QMessageBox::warning(this, "Remove Designer", "You cannot remove yourself!");
        return;
    }
    if (username == chip_owner_)
    {
        QMessageBox::warning(this, "Remove Designer", "You cannot remove the owner of this project!");
        return;
    }
    if (QMessageBox::warning(this,
                         "Remove Designer",
                         "Are you sure you want to remove designer" + username + "?\nAll blocks " +username+ " is responsible for will be assgined to the chip owner!",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    for (int i = 0; i < ui->tableBlock->rowCount(); i++)
    {
        if (ui->tableBlock->item(i, 3)->text() == username)
        {
            QString block_id = ui->tableBlock->item(i, 0)->text();
            if (!DataBaseHandler::update_items("block_system_block", "block_id", block_id, {{"responsible", chip_owner_id_}}))
            {
                DataBaseHandler::rollback();
                QMessageBox::warning(this, "Remove Designer", "Unable to remove the designer.\nError message: " + DataBaseHandler::get_error_message());
                return;
            }
        }
    }
    if (DataBaseHandler::delete_items("chip_designer", "chip_designer_id", designer_id))
    {
        DataBaseHandler::commit();
        int row = ui->tableDesigner->currentRow();
        ui->tableDesigner->removeRow(row);
        for (int i = 0; i < ui->tableBlock->rowCount(); i++)
            if (ui->tableBlock->item(i, 3)->text() == username)
                ui->tableBlock->item(i, 3)->setText(chip_owner_);
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove Designer", "Unable to remove the designer.\nError message: " + DataBaseHandler::get_error_message());
    }
}

void ChipEditorView::on_pushButtonRemoveRegPage_clicked()
{
    if (!authenticator_->can_fully_access_all_blocks())
    {
        QMessageBox::warning(this, "Remove Register Page", "You are not eligible to remove register pages!");
        return;
    }
    if (QMessageBox::warning(this, "Remove Register Page", "Are you sure you want to remove this register page?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        return;
    int row = ui->tableRegPage->currentRow();
    if (row < 0) return;
    QString page_id = ui->tableRegPage->item(row, 0)->text();

    if (DataBaseHandler::delete_items("chip_register_page_content", "page_id", page_id) &&
        DataBaseHandler::delete_items("chip_register_page", "page_id", page_id))
    {
        DataBaseHandler::commit();
        ui->tableRegPage->removeRow(row);
    }
    else {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove Register Page", "Unable to remove the register page.\nError massage: " + DataBaseHandler::get_error_message());
    }
}

void ChipEditorView::on_pushButtonRemoveSig_clicked()
{
    if (!authenticator_->can_remove_signal())
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

    if (!DatabaseUtils::remove_signal(sig_id, reg_sig_id))
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove Signal", "Unable to remove the signal.\nError message: " + DataBaseHandler::get_error_message());

        return;
    }

    ui->tableSignal->setSortingEnabled(false);
    DataBaseHandler::commit();
    if (reg_sig_id != "") ui->tableSigPart->setRowCount(0);
    ui->tableSignal->removeRow(row);
    ui->tableSignal->setSortingEnabled(true);
    emit(to_refresh_navigator_block());
}

void ChipEditorView::on_pushButtonRemoveReg_clicked()
{
    if (!authenticator_->can_remove_register())
    {
        QMessageBox::warning(this, "Remove Signal", "You are not eligible to remove registers!");
        return;
    }
    int row = ui->tableRegister->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Register",
                             "Are you sure you want to remove this register?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString reg_id = ui->tableRegister->item(row, 0)->text();
    if (!DatabaseUtils::remove_register(reg_id))
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove Register", "Unable to remove register.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }

    DataBaseHandler::commit();
    ui->tableRegPart->setRowCount(0);
    ui->tableRegister->removeRow(row);
    emit(to_refresh_navigator_block());
}

void ChipEditorView::on_pushButtonRemoveSigPart_clicked()
{
    int row = ui->tableSigPart->currentRow();
    if (row < 0) return;
    if (QMessageBox::warning(this,
                             "Remove Signal Partition",
                             "Are you sure you want to remove this signal partition?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    QString sig_reg_part_mapping_id = ui->tableSigPart->item(row, 0)->text();

    if (DataBaseHandler::delete_items("block_sig_reg_partition_mapping", "sig_reg_part_mapping_id", sig_reg_part_mapping_id))
    {
        DataBaseHandler::commit();
        display_signal_partitions();
        emit(to_refresh_navigator_block());
    }
    else
    {
        DataBaseHandler::rollback();
        QMessageBox::warning(this, "Remove Signal Partition", "Unable to remove the signal partition.\nError message: "+ DataBaseHandler::get_error_message());
    }
}

void ChipEditorView::on_tableBlock_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString responsible = ui->tableBlock->item(row, 3)->text();
    bool enabled = authenticator_->can_add_block() && ((authenticator_->can_remove_responsible_block() && responsible == username_) || authenticator_->can_fully_access_all_blocks());
    QString block_id = ui->tableBlock->item(row, 0)->text();
    QString old_block_name = ui->tableBlock->item(row, 1)->text();
    EditSystemBlockDialog edit_sys(chip_id_, block_id, address_width_, authenticator_->can_add_chip_designer(), enabled, this);
    if (edit_sys.exec() == QDialog::Accepted && edit_sys.edit_system_block())
    {
        ui->tableBlock->item(row, 1)->setText(edit_sys.get_block_name());
        ui->tableBlock->item(row, 2)->setText(edit_sys.get_block_abbr());
        ui->tableBlock->item(row, 3)->setText(edit_sys.get_responsible());
        ui->tableBlock->item(row, 4)->setText(edit_sys.get_start_addr());
        emit(block_modified(row, edit_sys.get_block_name(), edit_sys.get_block_abbr(), edit_sys.get_responsible_id()));
    }
    if (edit_sys.designer_added()) display_designers();

}

void ChipEditorView::on_tableBlock_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow < 0) ui->pushButtonRemoveBlock->setEnabled(false);
    else {
        QString responsible = ui->tableBlock->item(currentRow, 3)->text();
        ui->pushButtonRemoveBlock->setEnabled((authenticator_->can_remove_responsible_block() && responsible == username_) || authenticator_->can_fully_access_all_blocks());
    }
}

void ChipEditorView::on_tableDesigner_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString chip_designer = ui->tableDesigner->item(row, 1)->text(),
            project_role = ui->tableDesigner->item(row, 2)->text();
    EditChipDesignerDialog edit_designer(chip_id_, chip_designer, project_role,
                                         authenticator_->can_remove_chip_designer() && authenticator_->can_add_chip_designer() && \
                                         username_ != chip_designer && chip_owner_ != chip_designer,
                                         this);
    if (edit_designer.exec() == QDialog::Accepted && edit_designer.edit_designer())
    {
        ui->tableDesigner->item(row, 2)->setText(edit_designer.get_project_role());
    }
}

void ChipEditorView::on_tableDesigner_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveDesigner->setEnabled(currentRow >= 0 && authenticator_->can_remove_chip_designer());
}

void ChipEditorView::on_tableRegPage_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    if (!authenticator_->can_fully_access_all_blocks())
    {
        QMessageBox::warning(this, "Edit Register Page", "You are not eligible to edit register pages!");
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

void ChipEditorView::on_tableRegPage_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveRegPage->setEnabled(currentRow >= 0 && authenticator_->can_fully_access_all_blocks());
}

void ChipEditorView::on_tableSignal_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString sig_id = ui->tableSignal->item(row, 0)->text();
    QString reg_sig_id = ui->tableSignal->item(row, 1)->text();
    EditSignalDialog edit_signal(chip_id_, block_id_, sig_id, reg_sig_id, register_width_, msb_first_, authenticator_->can_add_signal() && authenticator_->can_remove_signal(), this);
    if (edit_signal.exec() == QDialog::Accepted && edit_signal.edit_signal())
    {
        ui->tableSignal->setSortingEnabled(false);
        QVector<QString> values = {ui->tableSignal->item(row, 0)->text(), edit_signal.get_reg_sig_id(),
                                   edit_signal.get_register_type_id(), edit_signal.get_signal_name(),
                                   edit_signal.get_width(), edit_signal.get_signal_type(),
                                   edit_signal.get_register_type(),
                                   edit_signal.is_register_signal() ? edit_signal.get_value() : "",
                                   edit_signal.add_port() ? "true" : "false"};
        for (int i = 0; i < values.size(); i++) ui->tableSignal->item(row, i)->setText(values[i]);
        emit(to_refresh_navigator_block());
        emit(ui->tableSignal->currentCellChanged(row, column, row, column));
        ui->tableSignal->setSortingEnabled(true);
    }
}

void ChipEditorView::on_tableSignal_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonAddSigPart->setEnabled(authenticator_->can_edit_signal_partition() && currentRow >= 0 && ui->tableSignal->item(currentRow, 1)->text() != "");
    ui->pushButtonRemoveSig->setEnabled(authenticator_->can_remove_signal() && currentRow >= 0);
    display_signal_partitions();
}

void ChipEditorView::on_tableRegister_cellDoubleClicked(int row, int column)
{
    if (row < 0) return;
    QString reg_id = ui->tableRegister->item(row, 0)->text();
    EditRegisterDialog edit_reg(chip_id_, block_id_, reg_id, authenticator_->can_add_register() && authenticator_->can_remove_register(), this);
    if (edit_reg.exec() == QDialog::Accepted && edit_reg.edit_register())
    {
        ui->tableRegister->item(row, 1)->setText(edit_reg.get_reg_name());
        emit(to_refresh_navigator_block());
    }
}

void ChipEditorView::on_tableRegister_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveReg->setEnabled(authenticator_->can_remove_register() && currentRow >= 0);
    display_register_partitions();
}

void ChipEditorView::on_tableSigPart_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->pushButtonRemoveSigPart->setEnabled(authenticator_->can_edit_signal_partition() && \
                                            currentRow >= 0 && \
                                            ui->tableSigPart->item(currentRow, 0)->text() != "-1");
}

void ChipEditorView::display_chip_basics()
{
    ui->tableChipBasics->setRowCount(0);
    ui->tableChipBasics->insertRow(0);
    ui->tableChipBasics->setItem(0, 0, new QTableWidgetItem(chip_name_));
    ui->tableChipBasics->setItem(0, 1, new QTableWidgetItem(chip_owner_));
    ui->tableChipBasics->setItem(0, 2, new QTableWidgetItem(QString::number(register_width_)));
    ui->tableChipBasics->setItem(0, 3, new QTableWidgetItem(QString::number(address_width_)));
    ui->tableChipBasics->setItem(0, 4, new QTableWidgetItem(msb_first_ ? "true" : "false"));
    ui->pushButtonEditChip->setEnabled((chip_owner_ == username_ || authenticator_->can_fully_access_all_projects()) && !authenticator_->frozen());
}

void ChipEditorView::display_system_blocks()
{
    ui->pushButtonAddBlock->setEnabled(authenticator_->can_add_block());
    ui->pushButtonRemoveBlock->setEnabled(false);

    QVector<QVector<QString> > items;
    ui->tableBlock->setRowCount(0);

    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    if (!DataBaseHandler::show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                     "global_user.username",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id"))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read system blocks from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    if (authenticator_->can_read_all_blocks()) items = sort_doubly_linked_list(items);
    for (const auto& item : items)
    {
        int row = ui->tableBlock->rowCount();
        ui->tableBlock->insertRow(row);
        for (int i = 0; i < item.size() - 2; i++)
            ui->tableBlock->setItem(row, i, new QTableWidgetItem(item[i]));

        QVector<QString> count;
        if (DataBaseHandler::show_one_item("block_register", count, {"count(reg_id)"}, "block_id", item[0]))
            ui->tableBlock->setItem(row, ui->tableBlock->columnCount()-1, new QTableWidgetItem(count[0]));
    }
}

void ChipEditorView::display_designers()
{
    ui->pushButtonAddDesigner->setEnabled(authenticator_->can_add_chip_designer());
    ui->pushButtonRemoveDesigner->setEnabled(false);


    QVector<QVector<QString> > items;
    ui->tableDesigner->setRowCount(0);

    if (!DataBaseHandler::show_items_inner_join({"chip_designer.chip_designer_id",
                                     "global_user.username",
                                     "def_project_role.project_role"},
                                    {{{"global_user", "user_id"}, {"chip_designer", "user_id"}},
                                     {{"chip_designer", "project_role_id"}, {"def_project_role", "project_role_id"}}},
                                    items, {{"chip_designer.chip_id", chip_id_}},
                                    "order by chip_designer.chip_designer_id"))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read registers from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    for (const auto& item : items)
    {
        int row = ui->tableDesigner->rowCount();
        ui->tableDesigner->insertRow(row);
        for (int i = 0; i < item.size(); i++) ui->tableDesigner->setItem(row, i, new QTableWidgetItem(item[i]));
    }
}

void ChipEditorView::display_register_pages()
{
    ui->pushButtonAddRegPage->setEnabled(authenticator_->can_fully_access_all_blocks());

    ui->tableRegPage->setRowCount(0);

    QVector<QVector<QString> > items;
    QVector<QString> extended_fields = {"chip_register_page.page_id", "chip_register_page.page_name",
                                   "signal_signal.sig_name", "chip_register_page.page_count",
                                   "block_system_block.block_name", "block_system_block.abbreviation", "signal_signal.add_port"};
    if (!DataBaseHandler::show_items_inner_join(extended_fields, {{{"chip_register_page", "ctrl_sig"}, {"signal_signal", "sig_id"}}, {{"signal_signal", "block_id"}, {"block_system_block", "block_id"}}}, items, {{"chip_register_page.chip_id", chip_id_}}))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read register pages from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING;
    for (const auto& item : items)
    {
        QString block_name = item[4], block_abbr = item[5];
        signal_naming.update_key("${BLOCK_NAME}", block_name);
        signal_naming.update_key("${BLOCK_ABBR}", block_abbr);
        int row = ui->tableRegPage->rowCount();
        ui->tableRegPage->insertRow(row);
        QVector<QString> values = {item[0], item[1], item[6]=="1" ? signal_naming.get_extended_name(item[2]) : item[2], item[3]};
        for (int i = 0; i < values.size(); i++) ui->tableRegPage->setItem(row, i, new QTableWidgetItem(values[i]));
    }
}

void ChipEditorView::display_signals()
{
    ui->tableSignal->setSortingEnabled(false);
    ui->pushButtonAddSig->setEnabled(authenticator_->can_add_signal());
    ui->pushButtonRemoveSig->setEnabled(false);

    ui->tableSignal->setRowCount(0);

    QVector<QVector<QString> > items;
    QVector<QString> fields = {"signal_signal.sig_id", "signal_signal.sig_name", "signal_signal.width", "def_signal_type.sig_type", "signal_signal.add_port"};
    if (!DataBaseHandler::show_items_inner_join(fields,
                                    {{{"signal_signal", "sig_type_id"}, {"def_signal_type", "sig_type_id"}}},
                                    items,
                                    {{"signal_signal.block_id", block_id_}}))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read signals from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    for (const auto& item : items)
    {
        int row = ui->tableSignal->rowCount();
        ui->tableSignal->insertRow(row);
        QString sig_id = item[0],
                reg_sig_id, reg_type_id,
                add_port = item[4] == "1" ? "true" : "false",
                sig_name = item[4] == "1" ? GLOBAL_SIGNAL_NAMING.get_extended_name(item[1]) : item[1],
                width = item[2],
                sig_type = item[3],
                reg_type, init_value;

        QVector<QVector<QString> > items_reg_sig;
        if (!DataBaseHandler::show_items_inner_join({"signal_reg_signal.reg_sig_id", "def_register_type.reg_type_id", "def_register_type.reg_type", "signal_reg_signal.init_value"},
                                        {{{"signal_reg_signal", "reg_type_id"}, {"def_register_type", "reg_type_id"}}},
                                        items_reg_sig, {{"signal_reg_signal.sig_id", item[0]}}))
        {
            QMessageBox::warning(this, "Chip Editor", "Unable to read signals from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
            break;
        }

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
    ui->tableSignal->setSortingEnabled(true);
}

void ChipEditorView::display_registers()
{
    ui->pushButtonAddReg->setEnabled(authenticator_->can_add_register());
    ui->pushButtonRemoveReg->setEnabled(false);

    ui->tableRegister->setRowCount(0);

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join({"block_register.reg_id", "block_register.reg_name", "def_register_type.reg_type", "block_register.prev", "block_register.next"},
                                    {{{"block_register", "reg_type_id"}, {"def_register_type", "reg_type_id"}}},
                                    items, {{"block_register.block_id", block_id_}}))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read registers from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }

    items  = sort_doubly_linked_list(items);
    QVector<QString> item;
    quint64 decimal_start_address = 0;
    bool display_address = false;
    if (DataBaseHandler::show_one_item("block_system_block", item, {"start_address"}, "block_id", block_id_))
    {
        display_address = true;
        decimal_start_address = item[0].toULongLong(nullptr, 16);
    }

    for (int i = 0; i < items.size(); i++)
    {
        const auto& item = items[i];
        int row = ui->tableRegister->rowCount();
        QString reg_id = item[0], reg_name = item[1], reg_type = item[2], address = display_address ? decimal2hex(decimal_start_address + static_cast<quint64>(i), address_width_) : "unknown";
        reg_name = GLOBAL_REGISTER_NAMING.get_extended_name(reg_name);
        ui->tableRegister->insertRow(row);
        ui->tableRegister->setItem(row, 0, new QTableWidgetItem(reg_id));
        ui->tableRegister->setItem(row, 1, new QTableWidgetItem(reg_name));
        ui->tableRegister->setItem(row, 2, new QTableWidgetItem(reg_type));
        ui->tableRegister->setItem(row, 3, new QTableWidgetItem(address));
    }
}

void ChipEditorView::display_signal_partitions()
{
    ui->tableSigPart->setRowCount(0);
    int row = ui->tableSignal->currentRow();
    if (row < 0 || ui->tableSignal->item(row, 1)->text() == "") return;
    QString reg_sig_id = ui->tableSignal->item(row, 1)->text();

    QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                       "block_register.reg_name",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb"};

    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join(extended_fields, {{{"block_sig_reg_partition_mapping", "reg_id"}, {"block_register", "reg_id"}}}, items,  {{"block_sig_reg_partition_mapping.reg_sig_id", reg_sig_id}}))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read signal partitions from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toInt() < b[1].toInt();});

    int signal_width = ui->tableSignal->item(row, 4)->text().toInt();
    int occupied_bits = 0;

    if (items.size() == 0)
    {
        QString sig_part = msb_first_ ? "<" + QString::number(signal_width-1) + ":"+ "0>" :
                                        "<0:"+ QString::number(signal_width-1) + ">";
        int row = ui->tableSigPart->rowCount();
        ui->tableSigPart->insertRow(row);
        ui->tableSigPart->setItem(row, 0, new QTableWidgetItem("-1"));
        ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
        ui->tableSigPart->setItem(row, 2, new QTableWidgetItem("NaN"));
    }
    else {
        for (int i = 0; i < items.size(); i++)
        {
            const auto& item = items[i];
            QString sig_reg_part_mapping_id = item[0], sig_lsb = item[1], sig_msb = item[2], reg_name = item[3], reg_lsb = item[4], reg_msb = item[5];
            if (i == 0)
            {
                QString sig_part;
                if (msb_first_ && sig_msb.toInt() < signal_width - 1)
                    sig_part = "<" + QString::number(signal_width-1) + ":"+ QString::number(sig_msb.toInt()+1) + ">";
                if (!msb_first_ && sig_lsb.toInt() > 0)
                    sig_part = "<0:"+ QString::number(sig_lsb.toInt()-1) + ">";
                if (sig_part != "")
                {
                    int row = ui->tableSigPart->rowCount();
                    ui->tableSigPart->insertRow(row);
                    ui->tableSigPart->setItem(row, 0, new QTableWidgetItem("-1"));
                    ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
                    ui->tableSigPart->setItem(row, 2, new QTableWidgetItem("NaN"));
                }
            }

            int row = ui->tableSigPart->rowCount();
            ui->tableSigPart->insertRow(row);
            reg_name = GLOBAL_REGISTER_NAMING.get_extended_name(reg_name);
            ui->tableSigPart->setItem(row, 0, new QTableWidgetItem(sig_reg_part_mapping_id));
            QString sig_part = msb_first_ ? "<" + sig_msb + ":"+ sig_lsb + ">" : "<" + sig_lsb + ":"+ sig_msb + ">",
                    reg_part = msb_first_ ? reg_name + "<" + reg_msb + ":"+ reg_lsb +">" : reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
            ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
            ui->tableSigPart->setItem(row, 2, new QTableWidgetItem(reg_part));
            occupied_bits += sig_msb.toInt() - sig_lsb.toInt() + 1;

            sig_part = "";
            if (i < items.size() - 1)
            {
                const auto& next = items[i+1];
                QString next_sig_lsb = next[1], next_sig_msb = next[2];
                if (msb_first_ && sig_lsb.toInt() - 1 > next_sig_msb.toInt())
                    sig_part = "<" + QString::number(sig_lsb.toInt() - 1) + ":"+ QString::number(next_sig_msb.toInt() + 1) + ">";
                if (!msb_first_ && sig_msb.toInt() + 1 < next_sig_lsb.toInt())
                    sig_part = "<" + QString::number(sig_msb.toInt() + 1) + ":"+ QString::number(next_sig_lsb.toInt() - 1) + ">";
            }
            if (i == items.size() - 1)
            {
                if (msb_first_ && sig_lsb.toInt() > 0)
                    sig_part = "<" + QString::number(sig_lsb.toInt() - 1) + ":0>";
                if (!msb_first_ && sig_msb.toInt() < signal_width - 1)
                    sig_part = "<" + QString::number(sig_msb.toInt() + 1) + ":"+ QString::number(signal_width - 1) + ">";
            }
            if (sig_part != "")
            {
                int row = ui->tableSigPart->rowCount();
                ui->tableSigPart->insertRow(row);
                ui->tableSigPart->setItem(row, 0, new QTableWidgetItem("-1"));
                ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
                ui->tableSigPart->setItem(row, 2, new QTableWidgetItem("NaN"));
            }

        }
    }
    if (occupied_bits == signal_width) ui->pushButtonAddSigPart->setEnabled(false);
}

void ChipEditorView::display_register_partitions()
{
    int row = ui->tableRegister->currentRow();
    ui->tableRegPart->setRowCount(0);
    if (row < 0) return;
    QString reg_id = ui->tableRegister->item(row, 0)->text();

    QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                        "block_sig_reg_partition_mapping.reg_lsb",
                                        "block_sig_reg_partition_mapping.reg_msb",
                                        "signal_signal.sig_name",
                                        "block_sig_reg_partition_mapping.sig_lsb",
                                        "block_sig_reg_partition_mapping.sig_msb",
                                        "signal_signal.add_port",
                                        "signal_signal.width",
                                        "signal_reg_signal.init_value"};
    QVector<QVector<QString> > items;
    if (!DataBaseHandler::show_items_inner_join(extended_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                 {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, items, {{"block_sig_reg_partition_mapping.reg_id", reg_id}}))
    {
        QMessageBox::warning(this, "Chip Editor", "Unable to read register partitions from database.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toInt() < b[1].toInt();});

    if (items.size() == 0)
    {
        QString reg_part = msb_first_ ? "<" + QString::number(register_width_-1) + ":"+ "0>" :
                                        "<0:"+ QString::number(register_width_-1) + ">";
        int row = ui->tableRegPart->rowCount();
        ui->tableRegPart->insertRow(row);
        ui->tableRegPart->setItem(row, 0, new QTableWidgetItem("-1"));
        ui->tableRegPart->setItem(row, 1, new QTableWidgetItem(reg_part));
        ui->tableRegPart->setItem(row, 2, new QTableWidgetItem("NaN"));
        ui->tableRegPart->setItem(row, 3, new QTableWidgetItem("NaN"));
    }
    else {
        for (int i = 0; i < items.size(); i++)
        {
            const auto& item = items[i];
            QString sig_reg_part_mapping_id = item[0], reg_lsb = item[1], reg_msb = item[2],
                    sig_name = item[3], sig_lsb = item[4], sig_msb = item[5], sig_width = item[7], sig_init_value = item[8];
            sig_name =  item[6]=="1" ? GLOBAL_SIGNAL_NAMING.get_extended_name(sig_name) : sig_name;

            if (i == 0)
            {
                QString reg_part;
                if (msb_first_ && reg_msb.toInt() < register_width_ - 1)
                    reg_part = "<" + QString::number(register_width_-1) + ":"+ QString::number(reg_msb.toInt()+1) + ">";
                if (!msb_first_ && reg_lsb.toInt() > 0)
                    reg_part = "<0:"+ QString::number(reg_lsb.toInt()-1) + ">";
                if (reg_part != "")
                {
                    int row = ui->tableRegPart->rowCount();
                    ui->tableRegPart->insertRow(row);
                    ui->tableRegPart->setItem(row, 0, new QTableWidgetItem("-1"));
                    ui->tableRegPart->setItem(row, 1, new QTableWidgetItem(reg_part));
                    ui->tableRegPart->setItem(row, 2, new QTableWidgetItem("NaN"));
                    ui->tableRegPart->setItem(row, 3, new QTableWidgetItem("NaN"));
                }
            }

            int row = ui->tableRegPart->rowCount();
            ui->tableRegPart->insertRow(row);
            ui->tableRegPart->setItem(row, 0, new QTableWidgetItem(sig_reg_part_mapping_id));
            QString reg_part = msb_first_ ? "<" + reg_msb + ":"+ reg_lsb + ">" : "<" + reg_lsb + ":"+ reg_msb + ">",
                    sig_part = sig_width == "1" ? sig_name : (msb_first_ ? sig_name + "<" + sig_msb + ":"+ sig_lsb +">" : sig_name + "<" + sig_lsb + ":"+ sig_msb +">");

            ui->tableRegPart->setItem(row, 1, new QTableWidgetItem(reg_part));
            ui->tableRegPart->setItem(row, 2, new QTableWidgetItem(sig_part));
            ui->tableRegPart->setItem(row, 3, new QTableWidgetItem(sig_init_value));

            reg_part = "";
            if (i < items.size() - 1)
            {
                const auto& next = items[i+1];
                QString next_reg_lsb = next[1], next_reg_msb = next[2];
                if (msb_first_ && reg_lsb.toInt() - 1 > next_reg_msb.toInt())
                    reg_part = "<" + QString::number(reg_lsb.toInt() - 1) + ":"+ QString::number(next_reg_msb.toInt() + 1) + ">";
                if (!msb_first_ && reg_msb.toInt() + 1 < next_reg_lsb.toInt())
                    reg_part = "<" + QString::number(reg_msb.toInt() + 1) + ":"+ QString::number(next_reg_lsb.toInt() - 1) + ">";
            }
            if (i == items.size() - 1)
            {
                if (msb_first_ && reg_lsb.toInt() > 0)
                    reg_part = "<" + QString::number(reg_lsb.toInt() - 1) + ":0>";
                if (!msb_first_ && reg_msb.toInt() < register_width_ - 1)
                    reg_part = "<" + QString::number(reg_msb.toInt() + 1) + ":"+ QString::number(register_width_ - 1) + ">";
            }
            if (reg_part != "")
            {
                int row = ui->tableRegPart->rowCount();
                ui->tableRegPart->insertRow(row);
                ui->tableRegPart->setItem(row, 0, new QTableWidgetItem("-1"));
                ui->tableRegPart->setItem(row, 1, new QTableWidgetItem(reg_part));
                ui->tableRegPart->setItem(row, 2, new QTableWidgetItem("NaN"));
                ui->tableRegPart->setItem(row, 3, new QTableWidgetItem("NaN"));
            }

        }
    }
}

void ChipEditorView::open_chip()
{
    if (authenticator_->can_fully_access_all_blocks() && authenticator_->can_read_all_blocks()) ui->tableBlock->viewport()->installEventFilter(this);
    else ui->tableBlock->viewport()->removeEventFilter(this);
    ui->tableBlock->setDragEnabled(authenticator_->can_fully_access_all_blocks());
    ui->tableBlock->setAcceptDrops(authenticator_->can_fully_access_all_blocks());
    ui->tableBlock->setDragDropMode(authenticator_->can_fully_access_all_blocks() ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);

    ui->tabWidget->blockSignals(false);
    display_chip_level_info();
}

void ChipEditorView::set_install_event_filter_register()
{
    bool enabled = authenticator_->can_add_register() && authenticator_->can_remove_register();
    if (enabled) ui->tableRegister->viewport()->installEventFilter(this);
    else ui->tableRegister->viewport()->removeEventFilter(this);
    ui->tableRegister->setDragEnabled(enabled);
    ui->tableRegister->setAcceptDrops(enabled);
    ui->tableRegister->setDragDropMode(enabled ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
}

bool ChipEditorView::eventFilter(QObject *obj, QEvent *eve)
{
    if (obj == ui->tableBlock->viewport() ||
            obj == ui->tableRegister->viewport())
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
                if (obj == ui->tableBlock->viewport()) pDropItem = ui->tableBlock->itemAt((static_cast<QDropEvent*>(eve))->pos());
                if (obj == ui->tableRegister->viewport()) pDropItem = ui->tableRegister->itemAt((static_cast<QDropEvent*>(eve))->pos());
                if (!pDropItem) return true;
                if (pDropItem->row() == row) return true;

                if (obj == ui->tableRegister->viewport())
                {
                    table_drop_event_handling(ui->tableRegister, "block_register", "reg_id", row, pDropItem->row());
                    ui->tableRegister->setCurrentCell(pDropItem->row(), 0);
                    emit(to_refresh_navigator_block());
                }
                if (obj == ui->tableBlock->viewport())
                {
                    table_drop_event_handling(ui->tableBlock, "block_system_block", "block_id", row, pDropItem->row());
                    ui->tableBlock->setCurrentCell(pDropItem->row(), 0);
                    emit(block_order_exchanged(row, pDropItem->row()));
                }
                return true;
            }
        } else return QWidget::eventFilter(obj, eve);
    }
    return QWidget::eventFilter(obj,eve);
}

void ChipEditorView::table_drop_event_handling(QTableWidget* table, const QString& table_name, const QString& key, int from_row, int to_row)
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
    int cols = table->columnCount();
    if (table == ui->tableRegister) cols--;
    if (to_row < from_row)
        for (int row = from_row; row > to_row; row--)
        {
            for (int col = 0; col < cols; col++) table->item(row, col)->setText(table->item(row-1, col)->text());
        }
    else {
        for (int row = from_row; row < to_row; row++)
        {
            for (int col = 0; col < cols; col++) table->item(row, col)->setText(table->item(row+1, col)->text());
        }
    }
    for (int col = 0; col < cols; col++) table->item(to_row, col)->setText(from_items[col]);
}

void ChipEditorView::on_splitterSignal_splitterMoved(int pos, int index)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("ui");
    settings.setValue("table_signal_height", ui->splitterSignal->sizes()[0]);
    settings.setValue("table_signal_partition_height", ui->splitterSignal->sizes()[1]);
}

void ChipEditorView::on_splitterRegister_splitterMoved(int pos, int index)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("ui");
    settings.setValue("table_register_height", ui->splitterRegister->sizes()[0]);
    settings.setValue("table_register_partition_height", ui->splitterRegister->sizes()[1]);
}

void ChipEditorView::on_splitterBlocks_splitterMoved(int pos, int index)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("ui");
    settings.setValue("table_block_height", ui->splitterBlocks->sizes()[0]);
    settings.setValue("table_designer_height", ui->splitterBlocks->sizes()[1]);
}

void ChipEditorView::on_splitterDesignerRegPage_splitterMoved(int pos, int index)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "RegisterManager", "user_settings");
    settings.beginGroup("ui");
    settings.setValue("table_designer_width", ui->splitterDesignerRegPage->sizes()[0]);
    settings.setValue("table_register_page_width", ui->splitterDesignerRegPage->sizes()[1]);
}
