#ifndef EDIT_SIGNAL_PARTITION_LOGIC_H
#define EDIT_SIGNAL_PARTITION_LOGIC_H

#include <QHash>
#include <QVector>

typedef QPair<int, int> partition;
typedef QVector<partition> partition_list;

class EditSignalPartitionLogic
{
public:
    EditSignalPartitionLogic(int register_width, bool msb_first=true);
protected:
    virtual void make_occupied_signal_parts();
    void make_available_signal_parts(int signal_width);
    virtual void make_available_signal_starts();
    virtual void make_available_signal_ends();
    virtual void make_occupied_register_parts(const QString& reg_id);
    void make_available_register_parts(const QString& reg_id);
    void make_available_register_parts_by_length();

    const partition_list& get_available_signal_parts();
    const QVector<int>& get_available_signal_starts();
    const QVector<int>& get_available_signal_ends();
    const partition_list& get_occupied_signal_parts();
    const partition_list& get_available_register_parts();
    const partition_list& get_available_register_parts_by_length(int length);
    virtual int get_current_signal_lsb() {return -1;}
    virtual int get_current_signal_msb() {return -1;}
    int get_partition_length();

    bool add_signal_partition(const QString& sig_lsb,
                              const QString& sig_msb,
                              const QString& reg_lsb,
                              const QString& reg_msb,
                              const QString& reg_id,
                              const QString& reg_sig_id);

    const int register_width_;
    partition_list occupied_signal_parts_, available_signal_parts_, available_register_parts_;
    QHash<QString, partition_list> reg_id2occupied_register_parts_;
    QVector<int> available_signal_starts_, available_signal_ends_;
    QHash<int, partition_list > len2reg_parts_;
    const bool msb_first_;

};

#endif // EDIT_SIGNAL_PARTITION_LOGIC_H
