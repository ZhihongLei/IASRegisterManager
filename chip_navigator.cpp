#include "chip_navigator.h"
#include "ui_chip_navigator.h"
#include "global_variables.h"
#include "data_utils.h"
#include "database_handler.h"
#include <QDebug>
#include <QMessageBox>

ChipNavigator::ChipNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChipNavigator)
{
    ui->setupUi(this);
    ui->treeWidgetBlock->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidgetBlock->setColumnHidden(1, true);
}

ChipNavigator::~ChipNavigator()
{
    delete ui;
}

void ChipNavigator::login(const QString& username, const QString& user_id)
{
    username_ = username;
    user_id_ = user_id;
}

void ChipNavigator::open_chip(const QString& chip, const QString& chip_id, bool msb_first)
{
    chip_name_ = chip;
    chip_id_ = chip_id;
    msb_first_ = msb_first;
    display_nagivator();
}

void ChipNavigator::close_chip()
{
    ui->treeWidgetBlock->clear();
    ui->lineEditSearch->clear();
    block_id2abbr_.clear();
}

void ChipNavigator::set_authenticator(Authenticator* authenticator)
{
    authenticator_ = authenticator;
}

void ChipNavigator::display_nagivator()
{
    ui->treeWidgetBlock->blockSignals(true);
    QVector<QVector<QString> > items;
    ui->treeWidgetBlock->clear();
    QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui->treeWidgetBlock);
    ui->treeWidgetBlock->addTopLevelItem(topLevelItem);
    topLevelItem->setText(0, chip_name_);
    topLevelItem->setSelected(true);
    topLevelItem->setExpanded(true);

    QVector<QPair<QString, QString> > key_value_pairs;
    if (authenticator_->can_read_all_blocks()) key_value_pairs = {{"block_system_block.chip_id", chip_id_}};
    else key_value_pairs = {{"block_system_block.chip_id", chip_id_}, {"block_system_block.responsible", user_id_}};
    if (!DataBaseHandler::show_items("block_system_block", {"block_id", "block_name", "abbreviation", "responsible", "prev", "next"}, key_value_pairs, items, "order by block_id"))
    {
        QMessageBox::warning(this, "Chip Navigator", "Unable to initialize due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    if (authenticator_->can_read_all_blocks()) items = sort_doubly_linked_list(items);

    for (const auto& item : items)
    {
        QTreeWidgetItem *block_item = new QTreeWidgetItem(topLevelItem);
        block_item->setText(0, item[1]);
        block_item->setText(1, item[0]);
        block_id2abbr_[item[0]] = item[2];
        block_id2responsible_[item[0]] = item[3];
    }
    for (int i = 0; i < topLevelItem->childCount(); i++)
        refresh_block(topLevelItem->child(i));
    if (ui->treeWidgetBlock->topLevelItemCount()) ui->treeWidgetBlock->setCurrentItem(ui->treeWidgetBlock->topLevelItem(0));
    ui->treeWidgetBlock->blockSignals(false);
}

void ChipNavigator::add_block(const QString& block_id, const QString& block_name, const QString& block_abbr, const QString& responsible)
{
    ui->treeWidgetBlock->blockSignals(true);
    QTreeWidgetItem *topLevelItem = ui->treeWidgetBlock->topLevelItem(0);
    QTreeWidgetItem *block_item = new QTreeWidgetItem(topLevelItem);
    block_item->setText(0, block_name);
    block_item->setText(1, block_id);
    block_id2abbr_[block_id] = block_abbr;
    block_id2responsible_[block_id] = responsible;
    ui->treeWidgetBlock->blockSignals(false);
}

void ChipNavigator::remove_block(int row)
{
    ui->treeWidgetBlock->blockSignals(true);
    QTreeWidgetItem *block_item = ui->treeWidgetBlock->topLevelItem(0)->child(row);
    QString block_id = block_item->text(1);
    block_item->parent()->removeChild(block_item);
    block_id2abbr_.remove(block_id);
    block_id2responsible_.remove(block_id);
    ui->treeWidgetBlock->blockSignals(false);
}

void ChipNavigator::modify_block(int row, const QString& block_name, const QString& block_abbr, const QString& responsible)
{
    ui->treeWidgetBlock->blockSignals(true);
    QTreeWidgetItem *block_item = ui->treeWidgetBlock->topLevelItem(0)->child(row);
    QString block_id = block_item->text(1);
    block_item->setText(0, block_name);
    block_id2responsible_[block_id] = responsible;
    if (block_abbr != block_id2abbr_[block_id])
    {
        block_id2abbr_[block_id] = block_abbr;
        refresh_block(block_item);
    }
    ui->treeWidgetBlock->blockSignals(false);
}

void ChipNavigator::change_block_order(int from, int to)
{
    ui->treeWidgetBlock->blockSignals(true);
    QTreeWidgetItem *topLevelItem = ui->treeWidgetBlock->topLevelItem(0);
    QTreeWidgetItem* item_from = topLevelItem->child(from);
    topLevelItem->removeChild(item_from);
    topLevelItem->insertChild(to, item_from);
    ui->treeWidgetBlock->blockSignals(false);
}

void ChipNavigator::refresh_block()
{
    ui->treeWidgetBlock->blockSignals(true);
    QTreeWidgetItem* block_item = ui->treeWidgetBlock->currentItem();
    while (block_item->parent()->parent()) block_item = block_item->parent();
    refresh_block(block_item);
    ui->treeWidgetBlock->blockSignals(false);
}

void ChipNavigator::on_treeWidgetBlock_currentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous)
{
    if (!item) return;
    if (!item->parent())
    {
        emit(chip_clicked());
    }
    else {
        QString block_responsible, reg_id, sig_id;
        QTreeWidgetItem* block_item = item;
        while (block_item->parent()->parent()) block_item = block_item->parent();

        QString block_id = block_item->text(1);
        QString block_name = block_item->text(0);
        block_responsible = block_id2responsible_[block_id];

        authenticator_->set_block_permissions(block_responsible == user_id_ || authenticator_->can_fully_access_all_blocks());

        GLOBAL_REGISTER_NAMING.update_key("${BLOCK_NAME}", block_name);
        GLOBAL_REGISTER_NAMING.update_key("${BLOCK_ABBR}", block_id2abbr_[block_id]);
        GLOBAL_SIGNAL_NAMING.update_key("${BLOCK_NAME}", block_name);
        GLOBAL_SIGNAL_NAMING.update_key("${BLOCK_ABBR}", block_id2abbr_[block_id]);

        if (item == block_item) // block
        {
            emit(block_clicked(block_id));
        }
        else if (item->parent() == block_item)   // register
        {
            reg_id = item->text(1);
            emit(register_clicked(block_id, reg_id));
        }
        else if (item->parent() && item->parent()->parent() == block_item)  // signal
        {
            sig_id = item->text(1);
            emit(signal_clicked(block_id, sig_id));
        }
    }
}

void ChipNavigator::on_lineEditSearch_textChanged(const QString &pattern)
{
    search(ui->treeWidgetBlock->topLevelItem(0), ui->lineEditSearch->text().toLower(), false);
    ui->treeWidgetBlock->topLevelItem(0)->setExpanded(true);
}

void ChipNavigator::refresh_block(QTreeWidgetItem *block_item)
{
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

    if (!DataBaseHandler::show_items("block_register", {"reg_id", "reg_name", "prev", "next"}, "block_id", block_item->text(1), registers))
    {
        QMessageBox::warning(this, "Chip Navigator", "Unable to refresh due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
        return;
    }
    registers = sort_doubly_linked_list(registers);

    NamingTemplate signal_naming = GLOBAL_SIGNAL_NAMING,
            register_naming = GLOBAL_REGISTER_NAMING;
    signal_naming.update_key("${BLOCK_NAME}", block_name);
    signal_naming.update_key("${BLOCK_ABBR}", block_abbr);
    register_naming.update_key("${BLOCK_NAME}", block_name);
    register_naming.update_key("${BLOCK_ABBR}", block_abbr);
    for (const auto& reg : registers)
    {
        QTreeWidgetItem *topLevelItemReg = new QTreeWidgetItem(block_item);
        topLevelItemReg->setText(0, register_naming.get_extended_name(reg[1]));
        topLevelItemReg->setText(1, reg[0]);

        if (reg_id2expanded.contains(reg[0])) topLevelItemReg->setExpanded(reg_id2expanded[reg[0]]);

        QVector<QVector<QString> > signal_items;
        QVector<QString> extended_fields = {"block_sig_reg_partition_mapping.sig_reg_part_mapping_id",
                                       "block_sig_reg_partition_mapping.reg_lsb",
                                       "block_sig_reg_partition_mapping.reg_msb",
                                       "signal_signal.sig_name",
                                       "signal_signal.sig_id",
                                       "block_sig_reg_partition_mapping.sig_lsb",
                                       "block_sig_reg_partition_mapping.sig_msb",
                                      "signal_signal.add_port"};
        if (!DataBaseHandler::show_items_inner_join(extended_fields, {{{"block_sig_reg_partition_mapping", "reg_sig_id"}, {"signal_reg_signal", "reg_sig_id"}},
                                                     {{"signal_reg_signal", "sig_id"}, {"signal_signal", "sig_id"}}}, signal_items, {{"block_sig_reg_partition_mapping.reg_id", reg[0]}}))
        {
            QMessageBox::warning(this, "Chip Navigator", "Unable to refresh due to database connection issue.\nPlease try again.\nError message: " + DataBaseHandler::get_error_message());
            break;
        }

        if (msb_first_) qSort(signal_items.begin(), signal_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[2].toInt() > b[2].toInt();});
        else qSort(signal_items.begin(), signal_items.end(), [](const QVector<QString>& a, const QVector<QString>& b) {return a[1].toInt() < b[1].toInt();});

        QSet<QString> signal_set;
        QVector<QString> signal_ids, signal_names;

        for (const auto& signal_item : signal_items)
        {
            QString sig_id = signal_item[4], sig_name = signal_item[3];
            if (signal_set.contains(sig_id)) continue;
            signal_set.insert(sig_id);
            signal_ids.push_back(sig_id);
            signal_names.push_back(signal_item[7]=="1" ? signal_naming.get_extended_name(sig_name) : sig_name);
        }

        for (int i = 0; i < signal_ids.size(); i++)
        {
            QString sig_id = signal_ids[i], sig_name = signal_names[i];
            QTreeWidgetItem *top_level_item_signal = new QTreeWidgetItem(topLevelItemReg);
            top_level_item_signal->setText(0, sig_name);
            top_level_item_signal->setText(1, sig_id);
        }
    }
    if (!ui->treeWidgetBlock->currentItem()) ui->treeWidgetBlock->setCurrentItem(block_item ? block_item : ui->treeWidgetBlock->topLevelItem(0));
}

bool ChipNavigator::search(QTreeWidgetItem* item, const QString &s, bool visible)
{
    if (!item) return false;
    visible |= item->text(0).toLower().contains(s);
    bool res = false;
    for (int i = 0; i < item->childCount(); i++)
    {
        res |= search(item->child(i), s, visible);
    }
    item->setHidden(!res && !visible);
    item->setExpanded(res && s != "");
    QFont font = item->font(0);
    QString color;
    font.setBold(s != "" && item->text(0).toLower().contains(s));
    font.setItalic(s != "" && item->text(0).toLower().contains(s));
    color = (s != "" && item->text(0).toLower().contains(s)) ? "yellow" : "white";
    item->setFont(0, font);
    item->setBackgroundColor(0, QColor(color));
    return res || item->text(0).toLower().contains(s);
}
