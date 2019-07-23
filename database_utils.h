#ifndef DATABASE_UTILS_H
#define DATABASE_UTILS_H


#include<QVector>

class DatabaseUtils
{
public:
    DatabaseUtils();

    static bool remove_signal(const QString& sig_id, const QString& reg_sig_id);
    static bool remove_register(const QString& reg_id);
    static bool remove_block(const QString& block_id);
    static bool remove_chip(const QString& chip_id);
    static bool remove_user(const QString& my_user_id, const QString& user_id);

    static bool copy_row(const QString& tablename, const QString& id_field,
                         const QVector<QString>& data_fields,
                         const QHash<QString, QHash<QString, QString> >& in_old2news,
                         QHash<QString, QString>& out_old2new,
                         bool order, const QString& key, const QString& value);
    static bool copy_row(const QString& tablename, const QString& id_field,
                         const QVector<QString>& data_fields,
                         const QHash<QString, QHash<QString, QString> >& in_old2news,
                         QHash<QString, QString>& out_old2new,
                         bool order, const QVector<QPair<QString, QString>> &key_value_pairs);
    static bool copy_row(const QString& tablename, const QString& id_field,
                         const QVector<QString>& data_fields,
                         const QHash<QString, QHash<QString, QString> >& in_old2news,
                         QHash<QString, QString>& out_old2new,
                         bool order, const QString& constraint);
};

#endif // DATABASE_UTILS_H
