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
#include "add_chip_designer_dialog.h"
#include "edit_signal_dialog.h"
#include "edit_register_dialog.h"
#include "edit_signal_partition_dialog.h"
#include <assert.h>
#include <QDropEvent>
#include <QMimeData>

RegisterManager::RegisterManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RegisterManager)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->tableSystem->setColumnHidden(0, true);
    ui->tableDesigner->setColumnHidden(0, true);
    ui->tableRegPage->setColumnHidden(0, true);
    ui->tableSignal->setColumnHidden(0, true);
    ui->tableSignal->setColumnHidden(1, true);
    ui->tableSignal->setColumnHidden(2, true);
    ui->tableSigPart->setColumnHidden(0, true);
    ui->tableReg->setColumnHidden(0, true);
    ui->tableRegPart->setColumnHidden(0, true);
    //clear_db();
    //init_db();
    connect(ui->actionUser_Management, SIGNAL(triggered()), this, SLOT(on_user_management()));
    connect(ui->actionChange_Password, SIGNAL(triggered()), this, SLOT(on_change_password()));
    connect(ui->actionNew_Chip, SIGNAL(triggered()), this, SLOT(on_new_chip()));
    connect(ui->actionOpen_Chip, SIGNAL(triggered()), this, SLOT(on_open_chip()));
    msb_first_ = true;
    QVector<QWidget*> widgets = {ui->pushButtonAddSys, ui->pushButtonRemoveSys, ui->pushButtonAddReg, ui->pushButtonRemoveReg, ui->pushButtonAddSig, ui->pushButtonRemoveSig, ui->pushButtonAddSigPart,
                                ui->pushButtonRemoveSigPart, ui->pushButtonAddDesigner, ui->pushButtonRemoveDesigner};
    for (QWidget* widget : widgets) widget->setVisible(false);
    for (QAction* action : {ui->actionUser_Management, ui->actionNew_Chip}) action->setVisible(false);

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
    std::cout << from_row << " " << to_row << std::endl;
    std::cout << "here before" << std::endl;
    std::cout << "column count " << table->columnCount() << std::endl;
    for (int col = 0; col < table->columnCount(); col++) from_items.push_back(table->item(from_row, col)->text());
    std::cout << "here after" << std::endl;
    if (to_row < from_row)
        for (int row = from_row; row > to_row; row--)
        {
            std::cout << "here " <<  row << std::endl;
            for (int col = 0; col < table->columnCount(); col++) table->item(row, col)->setText(table->item(row-1, col)->text());
        }
    else {
        for (int row = from_row; row < to_row; row++)
        {
            std::cout << "here " <<  row << std::endl;
            for (int col = 0; col < table->columnCount(); col++) table->item(row, col)->setText(table->item(row+1, col)->text());
        }
    }
    std::cout << "here 3" << std::endl;
    for (int col = 0; col < table->columnCount(); col++) table->item(to_row, col)->setText(from_items[col]);
}


bool RegisterManager::eventFilter(QObject *obj, QEvent *eve)
{
    if (obj == ui->tableSystem->viewport() or obj == ui->tableReg->viewport() or obj == ui->treeWidget->viewport())
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
                QTableWidgetItem* pDropItem;
                if (obj == ui->tableSystem->viewport()) pDropItem = ui->tableSystem->itemAt((static_cast<QDropEvent*>(eve))->pos());
                if (obj == ui->tableReg->viewport()) pDropItem = ui->tableReg->itemAt((static_cast<QDropEvent*>(eve))->pos());
                //if (obj == ui->treeWidget->viewport()) pDropItem = ui->treeWidget->childAt((static_cast<QDropEvent*>(eve))->pos());
                if (!pDropItem) return true;
                if (pDropItem->row() == row) return true;

                if (obj == ui->tableReg->viewport())
                {
                    table_drop_event_handling(ui->tableReg, "block_register", "reg_id", row, pDropItem->row());
                    ui->tableReg->setCurrentCell(pDropItem->row(), 0);
                }
                if (obj == ui->tableSystem->viewport())
                {
                    table_drop_event_handling(ui->tableSystem, "block_system_block", "block_id", row, pDropItem->row());
                    ui->tableSystem->setCurrentCell(pDropItem->row(), 0);

                    QTreeWidgetItem *topLevelItem = ui->treeWidget->topLevelItem(0);
                    QString block_from = topLevelItem->child(row)->text(0);
                    if (row > pDropItem->row())
                    {
                        for (int i = row; i > pDropItem->row(); i--) topLevelItem->child(i)->setText(0, topLevelItem->child(i-1)->text(0));
                    }
                    else {
                        for (int i = row; i < pDropItem->row(); i++) topLevelItem->child(i)->setText(0, topLevelItem->child(i+1)->text(0));
                    }
                    topLevelItem->child(pDropItem->row())->setText(0, block_from);
                }


                std::cout << "From " << row << " to " << pDropItem->row() << std::endl;
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

const QString& RegisterManager::get_username() const {return username_;}
const QString& RegisterManager::get_db_role() const {return db_role_;}
const QString& RegisterManager::get_db_role_id() const {return db_role_id_;}
const QString& RegisterManager::get_user_id() const {return user_id_;}


void RegisterManager::open_chip()
{
    QAbstractItemView* widget = ui->tableSystem;
    if (chip_owner_id_ == user_id_) widget->viewport()->installEventFilter(this);
    else widget->viewport()->removeEventFilter(this);
    widget->setDragEnabled(chip_owner_id_ == user_id_);
    widget->setAcceptDrops(chip_owner_id_ == user_id_);
    widget->setDragDropMode(chip_owner_id_ == user_id_ ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
    widget->setDropIndicatorShown(true);
    ui->stackedWidget->setCurrentIndex(0);
    display_system_blocks();
    display_designers();
    display_register_pages();
}

void RegisterManager::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    if (!item->parent())
    {
        ui->stackedWidget->setCurrentIndex(0);
        block_ = "";
        block_id_ = "-1";
    }
    else {
        ui->stackedWidget->setCurrentIndex(1);
        int row = item->parent()->indexOfChild(item);
        block_id_ = ui->tableSystem->item(row, 0)->text();
        block_ = ui->tableSystem->item(row, 1)->text();

        QString block_responsible = ui->tableSystem->item(row, 3)->text();
        authenticator_.set_block_permissions(block_responsible == username_ || user_id_ == chip_owner_id_);

        ui->pushButtonAddReg->setVisible(authenticator_.can_add_register());
        ui->pushButtonRemoveReg->setVisible(authenticator_.can_remove_register());
        ui->pushButtonAddSig->setVisible(authenticator_.can_add_signal());
        ui->pushButtonRemoveSig->setVisible(authenticator_.can_remove_signal());
        ui->pushButtonAddSigPart->setVisible(authenticator_.can_add_signal());
        ui->pushButtonRemoveSigPart->setVisible(authenticator_.can_remove_signal());

        if (authenticator_.can_add_register()) ui->tableReg->viewport()->installEventFilter(this);
        else ui->tableReg->viewport()->removeEventFilter(this);
        ui->tableReg->setDragEnabled(authenticator_.can_add_register());
        ui->tableReg->setAcceptDrops(authenticator_.can_add_register());
        ui->tableReg->setDragDropMode(authenticator_.can_add_register()? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop);
        ui->tableReg->setDropIndicatorShown(true);

        if (ui->tabWidget->currentIndex() == 0) display_signals();
        else display_registers();
    }
}

void RegisterManager::on_tableSignal_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == -1 or ui->tableSignal->item(currentRow, 1)->text() == "") ui->tableSigPart->setRowCount(0);
    else display_signal_partitions(ui->tableSignal->item(currentRow, 1)->text());
}

void RegisterManager::on_tableReg_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == -1) ui->tableRegPart->setRowCount(0);
    else display_register_partitions(ui->tableReg->item(currentRow, 0)->text());
}


void RegisterManager::on_tabWidget_currentChanged(int index)
{
    if (index == 0) display_signals();
    else if (index == 1) display_registers();
}

void RegisterManager::display_system_blocks()
{
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;

    ui->treeWidget->clear();

    ui->tableSystem->setRowCount(0);

    QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui->treeWidget);
    ui->treeWidget->addTopLevelItem(topLevelItem);
    topLevelItem->setText(0, chip_);
    topLevelItem->setSelected(true);
    topLevelItem->setExpanded(true);

    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_.can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    std::cout << user_id_.toUtf8().constData() << std::endl;
    dbhandler.show_items_inner_join({"block_system_block.block_id",
                                     "block_system_block.block_name",
                                     "block_system_block.abbreviation",
                                     "global_user.username",
                                    "block_system_block.start_address", "block_system_block.prev", "block_system_block.next"},
                                    {{{"block_system_block", "responsible"}, {"global_user", "user_id"}}},
                                    items, key_value_pairs,
                                    "order by block_system_block.block_id");


    QHash<QString, int> block_id2idx;
    int start = -1;

    if (authenticator_.can_read_all_blocks())
    {
        for (int i = 0; i < items.size(); i++)
        {
            block_id2idx[items[i][0]] = i;
            if (items[i][5] == "-1") start = i;
        }
        block_id2idx["-1"] = items.size();
    }
    else start = items.size() > 0 ? 0 : -1;
    if (start == -1) return;

    int next = start;
    while (true)
    {
        const auto& item = items[next];
        int row = ui->tableSystem->rowCount();
        ui->tableSystem->insertRow(row);
        for (int i = 0; i < item.size() - 2; i++)
            ui->tableSystem->setItem(row, i, new QTableWidgetItem(item[i]));
        QTreeWidgetItem *tree_item = new QTreeWidgetItem(topLevelItem);
        tree_item->setText(0, item[1]);
        blocks_.push_back(item[1]);

        QVector<QString> count;
        dbhandler.show_one_item("block_register", count, {"count(block_register.reg_id)"}, "block_id", item[0]);
        ui->tableSystem->setItem(row, ui->tableSystem->columnCount() -1, new QTableWidgetItem(count[0]));

        if (authenticator_.can_read_all_blocks()) next = block_id2idx[item[6]];
        else next += 1;
        if (next >= items.size()) break;

    }
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
}

void RegisterManager::display_register_pages()
{
    // TODO
}

void RegisterManager::display_signals()
{
    ui->tableSignal->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    QVector<QString> fields = {"signal_signal.sig_id", "signal_signal.sig_name", "signal_signal.width", "def_signal_type.sig_type"};
    dbhandler.show_items_inner_join(fields,
                                    {{{"signal_signal", "sig_type_id"}, {"def_signal_type", "sig_type_id"}}},
                                    items,
                                    {{"signal_signal.block_id", block_id_}});
    for (const auto& item : items)
    {
        int row = ui->tableSignal->rowCount();
        ui->tableSignal->insertRow(row);
        ui->tableSignal->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableSignal->setItem(row, 3, new QTableWidgetItem(item[1]));
        ui->tableSignal->setItem(row, 4, new QTableWidgetItem(item[2]));
        ui->tableSignal->setItem(row, 5, new QTableWidgetItem(item[3]));

        QVector<QVector<QString> > items_reg_sig;
        dbhandler.show_items_inner_join({"signal_reg_signal.reg_sig_id", "signal_reg_signal.init_value", "def_register_type.reg_type_id", "def_register_type.reg_type"},
                                        {{{"signal_reg_signal", "reg_type_id"}, {"def_register_type", "reg_type_id"}}},
                                        items_reg_sig, {{"signal_reg_signal.sig_id", item[0]}});

        if (items_reg_sig.size() == 1)
        {
            ui->tableSignal->setItem(row, 1, new QTableWidgetItem(items_reg_sig[0][0]));
            ui->tableSignal->setItem(row, 2, new QTableWidgetItem(items_reg_sig[0][2]));
            ui->tableSignal->setItem(row, 6, new QTableWidgetItem(items_reg_sig[0][3]));
            ui->tableSignal->setItem(row, 7, new QTableWidgetItem(items_reg_sig[0][1]));
        }
        else
        {
            ui->tableSignal->setItem(row, 1, new QTableWidgetItem(""));
            ui->tableSignal->setItem(row, 2, new QTableWidgetItem(""));
            ui->tableSignal->setItem(row, 6, new QTableWidgetItem(""));
            ui->tableSignal->setItem(row, 7, new QTableWidgetItem(""));

        }
    }
    if (ui->tableSignal->rowCount() > 0) ui->tableSignal->setCurrentCell(0, 0);
}

void RegisterManager::display_registers()
{
    ui->tableReg->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"block_register.reg_id", "block_register.reg_name", "def_register_type.reg_type", "block_register.prev", "block_register.next"},
                                    {{{"block_register", "reg_type_id"}, {"def_register_type", "reg_type_id"}}},
                                    items, {{"block_register.block_id", block_id_}});

    QHash<QString, int> reg_id2idx;
    int start = -1;
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i][3] == "-1") start = i;
        reg_id2idx[items[i][0]] = i;
    }
    if (start == -1) return;

    int next = start;
    while (true)
    {
        const auto& item = items[next];
        int row = ui->tableReg->rowCount();
        ui->tableReg->insertRow(row);
        ui->tableReg->setItem(row, 0, new QTableWidgetItem(item[0]));
        ui->tableReg->setItem(row, 1, new QTableWidgetItem(item[1]));
        ui->tableReg->setItem(row, 2, new QTableWidgetItem(item[2]));

        if (item[4] == "-1") break;
        next = reg_id2idx[item[4]];
    }
    if (ui->tableReg->rowCount() > 0) ui->tableReg->setCurrentCell(0, 0);
}



void RegisterManager::display_signal_partitions(const QString& reg_sig_id)
{
    assert (reg_sig_id != "");
    ui->tableSigPart->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> ext_fields = {"signal_reg_sig_partition.reg_sig_part_id",
                                       "signal_reg_sig_partition.lsb",
                                       "signal_reg_sig_partition.msb",
                                       "block_register.reg_name",
                                       "block_reg_partition.lsb",
                                       "block_reg_partition.msb"};

    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join(ext_fields, {{{"signal_reg_sig_partition", "reg_sig_part_id"}, {"block_reg_partition", "reg_sig_part_id"}},
                                                 {{"block_reg_partition", "reg_id"}, {"block_register", "reg_id"}}}, items,  {{"signal_reg_sig_partition.reg_sig_id", reg_sig_id}});

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});
    for (const auto& item : items)
    {
        int row = ui->tableSigPart->rowCount();
        ui->tableSigPart->insertRow(row);
        QString reg_sig_part_id = item[0], sig_lsb = item[1], sig_msb = item[2], reg_name = item[3], reg_lsb = item[4], reg_msb = item[5];
        ui->tableSigPart->setItem(row, 0, new QTableWidgetItem(reg_sig_part_id));
        QString sig_part, reg_part;
        if (msb_first_)
        {
            sig_part = "<" + sig_msb + ":"+ sig_lsb + ">";
            reg_part = reg_name + "<" + reg_msb + ":"+ reg_lsb +">";
        }
        else
        {
            sig_part = "<" + sig_lsb + ":"+ sig_msb + ">";
            reg_part = reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
        }
        ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
        ui->tableSigPart->setItem(row, 2, new QTableWidgetItem(reg_part));

    }

}


void RegisterManager::display_register_partitions(const QString& reg_id)
{
    ui->tableRegPart->setRowCount(0);
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> ext_fields = {"block_reg_partition.reg_part_id",
                                       "block_reg_partition.lsb",
                                       "block_reg_partition.msb",
                                       "signal_signal.sig_name",
                                       "signal_reg_sig_partition.lsb",
                                       "signal_reg_sig_partition.msb"};
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join(ext_fields, {{{"block_reg_partition", "reg_sig_part_id"}, {"signal_reg_sig_partition", "reg_sig_part_id"}},
                                                 {{"signal_reg_sig_partition", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                 {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, items, {{"block_reg_partition.reg_id", reg_id}});

    if (msb_first_) qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2] > b[2];});
    else qSort(items.begin(), items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1] < b[1];});
    for (const auto& item : items)
    {
        int row = ui->tableRegPart->rowCount();
        ui->tableRegPart->insertRow(row);
        QString reg_part_id = item[0], reg_lsb = item[1], reg_msb = item[2], sig_name = item[3], sig_lsb = item[4], sig_msb = item[5];
        ui->tableRegPart->setItem(row, 0, new QTableWidgetItem(reg_part_id));
        if (!msb_first_)
        {
            ui->tableRegPart->setItem(row, 1, new QTableWidgetItem("<" + reg_lsb + ":"+ reg_msb + ">"));
            ui->tableRegPart->setItem(row, 2, new QTableWidgetItem(sig_name + "<" + sig_lsb + ":"+ sig_msb +">" ));
        }
        else {
            ui->tableRegPart->setItem(row, 1, new QTableWidgetItem("<" + reg_msb + ":"+ reg_lsb + ">"));
            ui->tableRegPart->setItem(row, 2, new QTableWidgetItem(sig_name + "<" + sig_msb + ":"+ sig_lsb +">" ));
        }

    }
}

void RegisterManager::on_loggedin(QString username)
{
    username_ = username;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    dbhandler.show_items_inner_join({"global_user.user_id", "def_db_role.db_role", "def_db_role.db_role_id"},
                                    {{{"global_user", "db_role_id"}, {"def_db_role", "db_role_id"}}}, items, {{"username", username_}});
    user_id_ = items[0][0];
    db_role_ = items[0][1];
    db_role_id_ = items[0][2];
    authenticator_.set_database_permissions(db_role_id_);

    ui->actionUser_Management->setVisible(authenticator_.is_user_manager());
    ui->actionNew_Chip->setVisible(authenticator_.can_add_project());
    setWindowTitle("IAS Register Manager - " + username);
    show();
}

void RegisterManager::on_user_management()
{
    if (!authenticator_.is_user_manager())
    {
        QMessageBox::warning(this, "User Management", "You do not have access to User Manager!");
        return;
    }
    UserManagementDialog user_management(username_, this);
    user_management.exec();
}

void RegisterManager::on_change_password()
{
    ChangePasswordDialog change_password(get_username(), this);
    if (change_password.exec() == QDialog::Accepted) change_password.change_password();
}

void RegisterManager::on_new_chip()
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

void RegisterManager::on_open_chip()
{
    OpenChipDialog open_dial(user_id_, this);
    if (open_dial.exec() == QDialog::Accepted)
    {
        chip_ = open_dial.get_chip_name();
        chip_id_ = open_dial.get_chip_id();
        address_width_ = open_dial.get_address_width();
        register_width_ = open_dial.get_register_width();
        msb_first_ = open_dial.is_msb_first();
        chip_owner_ = open_dial.get_owner();
        chip_owner_id_ = open_dial.get_owner_id();
        authenticator_.set_project_permissions(open_dial.get_project_role_id());
        ui->pushButtonAddSys->setVisible(authenticator_.can_add_block());
        ui->pushButtonRemoveSys->setVisible(authenticator_.can_remove_block());
        ui->pushButtonAddDesigner->setVisible(authenticator_.can_add_chip_designer());
        ui->pushButtonRemoveDesigner->setVisible(authenticator_.can_remove_chip_designer());
        // TODO: compile project widget

        open_chip();
    }
    //
}

void RegisterManager::on_pushButtonAddSys_clicked()
{
    if (!authenticator_.can_add_block())
    {
        QMessageBox::warning(this, "New System Block", "You are not eligible to add system blocks!");
        return;
    }
    EditSystemBlockDialog new_system(chip_id_, address_width_, this);
    new_system.setWindowTitle("New System Block");
    if (new_system.exec() == QDialog::Accepted)
    {
        if (new_system.add_system_block())
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

            QTreeWidgetItem *top_level_item = ui->treeWidget->topLevelItem(0);
            QTreeWidgetItem *tree_item = new QTreeWidgetItem(top_level_item);
            tree_item->setText(0, new_system.get_block_name());
            ui->tableSystem->setCurrentCell(row, 0);
        }
    }
}

void RegisterManager::on_pushButtonRemoveSys_clicked()
{
    if (!authenticator_.can_remove_block())
    {
        QMessageBox::warning(this, "Remove System Block", "You are not eligible to remove system blocks!");
        return;
    }
    int row = ui->tableSystem->currentRow();
    if (row == -1) return;
    if (QMessageBox::warning(this,
                         "Remove System Block",
                         "Are you sure you want to remove this blcok?\nThis operation is not reversible!",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString block_id = ui->tableSystem->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("block_system_block", item, {"prev", "next"}, "block_id", block_id);
    QString prev = item[0], next = item[1];
    if (prev != "-1") dbhandler.update_items("block_system_block", {{"block_id", prev}}, {{"next", next}});
    if (next != "-1") dbhandler.update_items("block_system_block", {{"block_id", next}}, {{"prev", prev}});
    if (dbhandler.delete_items("block_system_block", "block_id", block_id))
    {
        ui->tableSystem->removeRow(row);
        QTreeWidgetItem *top_level_item = ui->treeWidget->topLevelItem(0);
        top_level_item->removeChild(top_level_item->child(row));
    }
    else
        QMessageBox::warning(this, "Remove System Block", QString("Removing system block failed\nError message: ")+ dbhandler.get_error_message());
}

void RegisterManager::on_pushButtonAddDesigner_clicked()
{
    if (!authenticator_.can_add_chip_designer())
    {
        QMessageBox::warning(this, "Add Chip Designer", "You are not eligible to add chip designer!");
        return;
    }
    AddChipDesignerDialog add_designer(chip_id_, this);
    if (add_designer.exec() == QDialog::Accepted && add_designer.add_designer())
    {
        QString username = add_designer.get_username();
        QString user_id = add_designer.get_user_id();
        QString project_role = add_designer.get_project_role();
        QString project_role_id = add_designer.get_project_role_id();
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
    if (row == -1) return;
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
                         "Are you sure you want to remove this designer?\nAll blocks owned by this user will then belong to the admin.",
                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    if (dbhandler.delete_items("chip_designer", "chip_designer_id", designer_id))
    {
        int row = ui->tableDesigner->currentRow();
        ui->tableDesigner->removeRow(row);
    }
}

void RegisterManager::on_pushButtonAddSig_clicked()
{
    if (!authenticator_.can_add_signal())
    {
        QMessageBox::warning(this, "Add Signal", "You are not eligible to add signals!");
        return;
    }
    EditSignalDialog new_signal(block_id_, register_width_, msb_first_, this);
    if (new_signal.exec() == QDialog::Accepted && new_signal.add_signal())
    {
        int row = ui->tableSignal->rowCount();
        QVector<QVector<QString> > items;
        ui->tableSignal->insertRow(row);
        ui->tableSignal->setItem(row, 0, new QTableWidgetItem(new_signal.get_signal_id()));
        ui->tableSignal->setItem(row, 1, new QTableWidgetItem(new_signal.get_reg_sig_id()));
        ui->tableSignal->setItem(row, 2, new QTableWidgetItem(new_signal.get_register_type_id()));
        ui->tableSignal->setItem(row, 3, new QTableWidgetItem(new_signal.get_signal_name()));
        ui->tableSignal->setItem(row, 4, new QTableWidgetItem(new_signal.get_width()));
        ui->tableSignal->setItem(row, 5, new QTableWidgetItem(new_signal.get_signal_type()));
        ui->tableSignal->setItem(row, 6, new QTableWidgetItem(new_signal.get_register_type()));
        ui->tableSignal->setItem(row, 7, new QTableWidgetItem(new_signal.get_value()));

        ui->tableSignal->setCurrentCell(row, 0);
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
    if (row == -1) return;
    if (QMessageBox::warning(this,
                             "Remove Signal",
                             "Are you sure you want to remove this signal?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString sig_id = ui->tableSignal->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.delete_items("signal_signal", "sig_id", sig_id))
        ui->tableSignal->removeRow(row);
    else
        QMessageBox::warning(this, "Remove Signal", QString("Removing signal failed\nError message: ") + dbhandler.get_error_message());
}

void RegisterManager::on_pushButtonAddReg_clicked()
{
    if (!authenticator_.can_add_register())
    {
        QMessageBox::warning(this, "Add Register", "You are not eligible to add registers!");
        return;
    }
    EditRegisterDialog new_reg(block_id_, this);
    if (new_reg.exec() == QDialog::Accepted && new_reg.add_register())
    {
        int row = ui->tableReg->rowCount();
        ui->tableReg->insertRow(row);
        ui->tableReg->setItem(row, 0, new QTableWidgetItem(new_reg.get_reg_id()));
        ui->tableReg->setItem(row, 1, new QTableWidgetItem(new_reg.get_reg_name()));
        ui->tableReg->setItem(row, 2, new QTableWidgetItem(new_reg.get_reg_type()));
        ui->tableReg->setCurrentCell(row, 0);
    }
}

void RegisterManager::on_pushButtonRemoveReg_clicked()
{
    if (!authenticator_.can_remove_register())
    {
        QMessageBox::warning(this, "Remove Signal", "You are not eligible to remove signals!");
        return;
    }
    int row = ui->tableReg->currentRow();
    if (row == -1) return;
    if (QMessageBox::warning(this,
                             "Remove Register",
                             "Are you sure you want to remove this register?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QString reg_id = ui->tableReg->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QString> item;
    dbhandler.show_one_item("block_register", item, {"prev", "next"}, "reg_id", reg_id);
    QString prev = item[0], next = item[1];
    if (prev != "-1") dbhandler.update_items("block_register", {{"reg_id", prev}}, {{"next", next}});
    if (next != "-1") dbhandler.update_items("block_register", {{"reg_id", next}}, {{"prev", prev}});
    if (dbhandler.delete_items("block_register", "reg_id", reg_id))
        ui->tableReg->removeRow(row);
    else
        QMessageBox::warning(this, "Remove Register", QString("Removing register failed\nError message: ") + dbhandler.get_error_message());
}


void RegisterManager::on_pushButtonAddSigPart_clicked()
{
    int row = ui->tableSignal->currentRow();
    if (row == -1 || ui->tableSignal->item(row, 1)->text() == "") return;
    QString reg_sig_id = ui->tableSignal->item(row, 1)->text();
    QString reg_type_id = ui->tableSignal->item(row, 2)->text();
    int signal_width = ui->tableSignal->item(row, 4)->text().toInt();
    EditSignalPartitionDialog new_sig_part(block_id_, reg_sig_id, reg_type_id, signal_width, register_width_, msb_first_, this);
    if (new_sig_part.exec() == QDialog::Accepted && new_sig_part.add_signal_partition())
    {
        QString sig_lsb = new_sig_part.get_signal_lsb();
        QString sig_msb = new_sig_part.get_signal_msb();
        QString reg_lsb = new_sig_part.get_register_lsb();
        QString reg_msb = new_sig_part.get_register_msb();
        QString reg_name = new_sig_part.get_register_name();
        QString reg_id = new_sig_part.get_register_id();

        QString reg_sig_part_id = new_sig_part.get_reg_sig_part_id();
        int row = ui->tableSigPart->rowCount();
        ui->tableSigPart->insertRow(row);
        ui->tableSigPart->setItem(row, 0, new QTableWidgetItem(reg_sig_part_id));
        QString sig_part, reg_part;
        if (msb_first_)
        {
            sig_part = "<" + sig_msb + ":"+ sig_lsb + ">";
            reg_part = reg_name + "<" + reg_msb + ":"+ reg_lsb +">";
        }
        else
        {
            sig_part = "<" + sig_lsb + ":"+ sig_msb + ">";
            reg_part = reg_name + "<" + reg_lsb + ":"+ reg_msb +">";
        }
        ui->tableSigPart->setItem(row, 1, new QTableWidgetItem(sig_part));
        ui->tableSigPart->setItem(row, 2, new QTableWidgetItem(reg_part));

    }
}

void RegisterManager::on_pushButtonRemoveSigPart_clicked()
{
    int row = ui->tableSigPart->currentRow();
    if (row == -1) return;
    if (QMessageBox::warning(this,
                             "Remove Signal Partition",
                             "Are you sure you want to remove this signal partition?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    QString reg_sig_part_id = ui->tableSigPart->item(row, 0)->text();
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.delete_items("signal_reg_sig_partition", "reg_sig_part_id", reg_sig_part_id))
        ui->tableSigPart->removeRow(row);
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
                {"is_user_manager", "tinyint(1)", "not null"},
                {"add_project", "tinyint(1)", "not null"},
                {"remove_project", "tinyint(1)", "not null"}};
    primary_key = "db_role_id";
    unique_keys = {"db_role"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"db_role", "is_user_manager", "add_project", "remove_project"};
    values = {"super user", "1", "1", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"designer", "0", "1", "0"};
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
                {"remove_block", "tinyint(1)", "not null"},
                {"read_all_blocks", "tinyint(1)", "not null"},
                {"compile_project", "tinyint(1)", "not null"},
                {"add_chip_designer", "tinyint(1)", "not null"},
                {"remove_chip_designer", "tinyint(1)", "not null"}};
    primary_key = "project_role_id";
    unique_keys = {"project_role"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);

    fields = {"project_role", "add_block", "remove_block", "read_all_blocks", "compile_project", "add_chip_designer", "remove_chip_designer"};
    values = {"admin", "1", "1", "1", "1", "1", "1"};
    dbhandler.insert_item(dbname, fields, values);
    values = {"designer", "0", "0", "1", "1", "0", "0"};
    dbhandler.insert_item(dbname, fields, values);

    // def_doc_type
    dbname = "def_doc_type";
    table_define = {{"doc_type_id", "int", "not null auto_increment"},
                {"doc_type", "varchar(20)", "not null"}};
    primary_key = "doc_type_id";
    unique_keys = {"doc_type"};
    dbhandler.create_table(dbname, table_define, primary_key, nullptr, &unique_keys);
    dbhandler.insert_item(dbname, {"doc_type"}, {"text"});
    dbhandler.insert_item(dbname, {"doc_type"}, {"LaTeX"});
    dbhandler.insert_item(dbname, {"doc_type"}, {"image"});


    // global_user
    dbname = "global_user";
    table_define = {{"user_id", "int", "not null auto_increment"},
                {"username", "varchar(20)", "not null"},
                {"password", "varchar(20)", "not null"},
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
                {"abbreviation", "varchar(52)", "not null"},
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
                {"noport", "tinyint(1)"}};
    primary_key = "sig_id";
    foreign_keys = {{"block_id", "block_system_block", "block_id"},
                    {"sig_type_id", "def_signal_type", "sig_type_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);


    dbname = "chip_register_page";
    table_define = {{"page_id", "int", "not null auto_increment"},
                {"page_name", "varchar(256)", "not null"},
                {"chip_id", "int", "not null"},
                {"ctrl_sig", "int", "not null"}};
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

    dbname = "signal_reg_sig_partition";
    table_define = {{"reg_sig_part_id", "int", "not null auto_increment"},
                {"reg_sig_id", "int", "not null"},
                {"lsb", "int", "not null"},
                {"msb", "int", "not null"},
                {"bus_signal", "tinyint(1)"}};
    primary_key = "reg_sig_part_id";
    foreign_keys = {{"reg_sig_id", "signal_reg_signal", "reg_sig_id"}};
    dbhandler.create_table(dbname, table_define, primary_key, &foreign_keys, nullptr);

    dbname = "block_reg_partition";
    table_define = {{"reg_part_id", "int", "not null auto_increment"},
                {"reg_id", "int", "not null"},
                {"lsb", "int", "not null"},
                {"msb", "int", "not null"},
                {"reg_sig_part_id", "int", "not null"}};
    primary_key = "reg_sig_part_id";
    foreign_keys = {{"reg_id", "block_register", "reg_id"},
                    {"reg_sig_part_id", "signal_reg_sig_partition", "reg_sig_part_id"}};
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
    /*
    dbhandler.delete_table("block_reg_partition");
    dbhandler.delete_table("signal_reg_sig_partition");
    dbhandler.delete_table("signal_reg_signal");
    dbhandler.delete_table("signal_signal");
    dbhandler.delete_table("block_register");
    dbhandler.delete_table("block_system_block");
    dbhandler.delete_table("chip_designer");
    dbhandler.delete_table("chip_chip");
    dbhandler.delete_table("global_user");
    dbhandler.delete_table("def_project_role");
    dbhandler.delete_table("def_register_type");
    dbhandler.delete_table("def_signal_type");
    dbhandler.delete_table("def_db_role");
    */
}

void RegisterManager::on_tableSignal_cellDoubleClicked(int row, int column)
{
    if (!(authenticator_.can_add_signal() && authenticator_.can_remove_signal())) return;
    QString sig_id = ui->tableSignal->item(row, 0)->text();
    QString reg_sig_id = ui->tableSignal->item(row, 1)->text();
    EditSignalDialog edit_signal(block_id_, sig_id, reg_sig_id, register_width_, msb_first_, this);
    if (edit_signal.exec() == QDialog::Accepted && edit_signal.edit_signal())
    {
        ui->tableSignal->item(row, 1)->setText(edit_signal.get_reg_sig_id());
        ui->tableSignal->item(row, 2)->setText(edit_signal.get_register_type_id());
        ui->tableSignal->item(row, 3)->setText(edit_signal.get_signal_name());
        ui->tableSignal->item(row, 4)->setText(edit_signal.get_width());
        ui->tableSignal->item(row, 5)->setText(edit_signal.get_signal_type());
        ui->tableSignal->item(row, 6)->setText(edit_signal.get_register_type());
        ui->tableSignal->item(row, 7)->setText(edit_signal.get_value());

    }
    emit(ui->tableSignal->currentCellChanged(row, column, row, column));
}

void RegisterManager::on_tableReg_cellDoubleClicked(int row, int column)
{
    if (!(authenticator_.can_add_register() && authenticator_.can_remove_register())) return;
    QString reg_id = ui->tableReg->item(row, 0)->text();
    EditRegisterDialog edit_reg(block_id_, reg_id, this);
    if (edit_reg.exec() == QDialog::Accepted && edit_reg.edit_register())
    {
        ui->tableReg->item(row, 1)->setText(edit_reg.get_reg_name());
    }
}

void RegisterManager::on_tableSystem_cellDoubleClicked(int row, int column)
{
    if (!(authenticator_.can_add_block() && authenticator_.can_remove_block())) return;
    QString block_id = ui->tableSystem->item(row, 0)->text();
    QString old_block_name = ui->tableSystem->item(row, 1)->text();
    EditSystemBlockDialog edit_sys(chip_id_, block_id, address_width_, this);
    if (edit_sys.exec() == QDialog::Accepted && edit_sys.edit_system_block())
    {
        ui->tableSystem->item(row, 1)->setText(edit_sys.get_block_name());
        ui->tableSystem->item(row, 2)->setText(edit_sys.get_block_abbr());
        ui->tableSystem->item(row, 3)->setText(edit_sys.get_responsible());
        ui->tableSystem->item(row, 4)->setText(edit_sys.get_start_addr());
        QTreeWidgetItem* top = ui->treeWidget->topLevelItem(0);
        top->child(row)->setText(0, edit_sys.get_block_name());
    }


}
