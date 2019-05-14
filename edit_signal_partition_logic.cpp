#include "edit_signal_partition_logic.h"
#include "global_variables.h"
#include "database_handler.h"
#include <iostream>
#include <QtAlgorithms>

EditSignalPartitionLogic::EditSignalPartitionLogic(int register_width, bool msb_first):
    register_width_(register_width),
    msb_first_(msb_first)
{

}

void EditSignalPartitionLogic::make_occupied_signal_parts()
{

}

void EditSignalPartitionLogic::make_available_signal_parts(int signal_width)
{
    qSort(occupied_signal_parts_);
    available_signal_parts_.clear();
    int prev_end = -1;
    for (const auto& segment : occupied_signal_parts_)
    {
        int start = segment.first;
        int end = segment.second;
        std::cout << start << " " << end << std::endl;
        if (start > prev_end + 1) available_signal_parts_.push_back({prev_end+1, start - 1});
        prev_end = end;
    }
    if (prev_end + 1 < signal_width) available_signal_parts_.push_back({prev_end+1, signal_width - 1});
}

void EditSignalPartitionLogic::make_available_signal_starts()
{
    available_signal_starts_.clear();
    if (!msb_first_)
        for (const auto & segment: available_signal_parts_)
        {
            for (int start = segment.first; start <= segment.second; start++)
            {
                available_signal_starts_.push_back(start);
            }
        }
    else
    {
        int msb = get_current_signal_msb();
        std::cout << "msb " << msb <<std::endl;
        for (const auto & segment: available_signal_parts_)
        {
            if (segment.first<= msb and segment.second >= msb)
                for (int lsb = msb; lsb >= segment.first &&  msb < lsb + register_width_ && !get_available_register_parts_by_length(msb-lsb+1).empty(); lsb--) available_signal_starts_.insert(0, lsb);
        }
    }
}

void EditSignalPartitionLogic::make_available_signal_ends()
{
    available_signal_ends_.clear();
    if (!msb_first_)
    {
        int lsb = get_current_signal_lsb();
        for (const auto & segment: available_signal_parts_)
        {
            if (segment.first<= lsb and segment.second >= lsb)
                for (int msb = lsb; msb <= segment.second && msb < lsb + register_width_ && !get_available_register_parts_by_length(msb-lsb+1).empty() ; msb++) available_signal_ends_.push_back(msb);
        }
    }
    else
    {
        for (const auto & segment: available_signal_parts_)
        {
            for (int msb = segment.first; msb <= segment.second; msb++)
            {
                std::cout << msb << std::endl;
                available_signal_ends_.push_back(msb);
            }
        }
    }
}

void EditSignalPartitionLogic::make_occupied_register_parts(const QString& reg_id)
{
    std::cout << "make occupied" << std::endl;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    QVector<QVector<QString> > items;
    if (reg_id2occupied_register_parts_.find(reg_id) != reg_id2occupied_register_parts_.end()) return;
    reg_id2occupied_register_parts_[reg_id] = partition_list();

    dbhandler.show_items("block_reg_partition", {"lsb", "msb"}, {{"reg_id", reg_id}}, items, "order by lsb");
    for (const auto& item : items)
    {
        reg_id2occupied_register_parts_[reg_id].push_back({item[0].toInt(), item[1].toInt()});
    }
}

void EditSignalPartitionLogic::make_available_register_parts(const QString& reg_id)
{
    qSort(reg_id2occupied_register_parts_[reg_id]);
    available_register_parts_.clear();
    int prev_end = -1;
    for (const auto& segment : reg_id2occupied_register_parts_[reg_id])
    {
        int start = segment.first;
        int end = segment.second;
        if (start > prev_end + 1) available_register_parts_.push_back({prev_end+1, start - 1});
        prev_end = end;
    }
    if (prev_end + 1 < register_width_) available_register_parts_.push_back({prev_end+1, register_width_ - 1});
}

void EditSignalPartitionLogic::make_available_register_parts_by_length()
{
    len2reg_parts_.clear();
    for (int len = 1; len <= register_width_; len++)
    {
        len2reg_parts_[len] = QVector<partition>();
        for (const auto& segment : available_register_parts_)
        {
            for (int lsb = segment.first; lsb + len - 1 <= segment.second; lsb ++)
            {
                len2reg_parts_[len].push_back({lsb, lsb + len - 1});
            }
        }
    }
    len2reg_parts_[-1] = QVector<partition>();

}

const partition_list& EditSignalPartitionLogic::get_available_signal_parts()
{
    return available_signal_parts_;
}
const QVector<int>& EditSignalPartitionLogic::get_available_signal_starts()
{
    return available_signal_starts_;
}
const QVector<int>& EditSignalPartitionLogic::get_available_signal_ends()
{
    return available_signal_ends_;
}

const partition_list& EditSignalPartitionLogic::get_occupied_signal_parts()
{
    return occupied_signal_parts_;
}

const partition_list& EditSignalPartitionLogic::get_available_register_parts()
{
    return available_register_parts_;
}

const QVector<partition>& EditSignalPartitionLogic::get_available_register_parts_by_length(int length)
{
    if (len2reg_parts_.find(length) != len2reg_parts_.end()) return len2reg_parts_[length];
    else return len2reg_parts_[-1];
}

int EditSignalPartitionLogic::get_partition_length()
{
    if (get_current_signal_msb() == -1 || get_current_signal_lsb() == -1) return -1;
    return get_current_signal_msb() - get_current_signal_lsb() + 1;
}

bool EditSignalPartitionLogic::add_signal_partition(const QString& sig_lsb,
                                                    const QString& sig_msb,
                                                    const QString& reg_lsb,
                                                    const QString& reg_msb,
                                                    const QString& reg_id,
                                                    const QString& reg_sig_id)
{
    QVector<QVector<QString> > items;
    DataBaseHandler dbhandler(gDBHost, gDatabase);
    if (dbhandler.insert_item("signal_reg_sig_partition", {"reg_sig_id", "lsb", "msb"}, {reg_sig_id, sig_lsb, sig_msb}) && \
            dbhandler.show_items("signal_reg_sig_partition", {"reg_sig_part_id"}, {{"reg_sig_id", reg_sig_id}, {"lsb", sig_lsb}, {"msb", sig_msb}}, items))
    {
        QString reg_sig_part_id = items[0][0];
        items.clear();
        dbhandler.insert_item("block_reg_partition", {"reg_id", "lsb", "msb", "reg_sig_part_id"}, {reg_id, reg_lsb, reg_msb, reg_sig_part_id});
        dbhandler.show_items("block_reg_partition", {"reg_part_id"}, {{"reg_id", reg_id}, {"lsb", reg_lsb}, {"msb", reg_msb}, {"reg_sig_part_id", reg_sig_part_id}}, items);
        return true;
    }
    return false;

}
