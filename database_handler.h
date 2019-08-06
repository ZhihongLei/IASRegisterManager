#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H
#include <QVector>
#include <memory>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>


class DataBaseHandler
{
public:
    static bool initialize(const QString& hostname, const QString& database, const QString& username, const QString& password);
    static bool use_database(const QString &database);
    static void close();
    static void commit();
    static void rollback();
    static bool create_database(const QString &dbname);
    static bool delete_database(const QString &dbname);

    static bool show_databases(QVector<QString>& dbs, const QString &constraint="");
    static bool create_table(const QString &tablename,
                     const QVector<QVector<QString> > &field_definitions,
                     const QString &primary_key = "");
    static bool delete_table(const QString &tablename);
    static bool add_unique_key_constraint(const QString& tablename, const QVector<QString>& fields);
    static bool add_foreign_key_constraint(const QString& from_table, const QString& from_field,
                                           const QString& to_table, const QString& to_field);
    static bool show_tables();
    static bool insert_item(const QString &tablename,
                    const QVector<QString> &fields,
                    const QVector<QString> &values);
    static bool update_items(const QString &tablename,
                   const QVector<QPair<QString, QString>> &key_value_pairs,
                    const QVector<QPair<QString, QString>> &field_value_pairs);
    static bool update_items(const QString &tablename,
                    const QString& key, const QString& value,
                    const QVector<QPair<QString, QString>> &field_value_pairs);
    static bool delete_items(const QString &tablename,
                   const QString &key,
                   const QString &value);
    static bool delete_items(const QString &tablename,
                     const QVector<QPair<QString, QString> >& key_value_pairs);
    static bool delete_items(const QString &tablename,
                     const QString &constraint="");
    static bool show_items(const QString& tablename,
                   const QVector<QString>& fields,
                   const QString& key,
                   const QString& value,
                   QVector<QVector<QString> >& items,
                   const QString& additional="");
    static bool show_items(const QString &tablename,
                   const QVector<QString> &fields,
                   const QVector<QPair<QString, QString> >& key_value_pairs,
                   QVector<QVector<QString> >& items,
                   const QString& additional="");
    static bool show_items(const QString& tablename,
                   const QVector<QString> &fields,
                   QVector<QVector<QString> >& items,
                   const QString& constraint="",
                   const QString& additional="");

    static bool show_items_inner_join(const QVector<QString>& extended_fields,
                            const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                            QVector<QVector<QString> >& items,
                            const QString& key,
                            const QString& value,
                            const QString &additional);

    static bool show_items_inner_join(const QVector<QString>& extended_fields,
                            const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                            QVector<QVector<QString> >& items,
                            const QVector<QPair<QString, QString> >& key_value_pairs,
                            const QString &additional="");

    static bool show_items_inner_join(const QVector<QString>& extended_fields,
                            const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& equal_table_field_pairs,
                            QVector<QVector<QString> >& items,
                            const QString &constraint="",
                            const QString &additional="");

    static bool show_one_item(const QString &tablename,
                   QVector<QString>& item,
                   const QVector<QString> &fields,
                   const QString &key,
                   const QString &value);
    static bool show_one_item(const QString &tablename,
                   QVector<QString>& item,
                   const QVector<QString> &fields,
                   const QVector<QPair<QString, QString> >& key_value_pairs);
    static bool show_one_item(const QString &tablename,
                   QVector<QString>& item,
                   const QVector<QString> &fields,
                   const QString &constraint);

    static bool get_next_auto_increment_id(const QString& tablename, const QString& id_field, QString& id);
    static QString get_error_message();

private:
    static bool execute(const QString &statement);
    static bool execute_query(const QString &statement);

    static QString error_message_;
    static QString database_;
    static sql::Driver* driver_;
    static std::unique_ptr< sql::Connection > con_;
    static std::unique_ptr< sql::Statement > stmt_;
    static std::unique_ptr< sql::ResultSet > res_;
};

#endif // DATABASE_HANDLER_H
