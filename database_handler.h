#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H
#include <QString>
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
    DataBaseHandler(const QString &hostname, const QString& database);
    ~DataBaseHandler();
    int create_database(const QString &dbname);
    int delete_database(const QString &dbname);

    int use_database(const QString &dbname);
    int show_databases(const QString &constraint, QVector<QString>& dbs);
    // assume the first field is always primary key and auto increment
    int create_table(const QString &tablename,
                     const QVector<QString> &fields,
                     const QVector<QString> &datatypes,
                     const QVector<QString> &additionals,
                     const QString &prikey = "");
    int create_table(const QString &tablename,
                     const QVector<QVector<QString> > &table_define,
                     const QString &primary_key = "",
                     const QVector<QVector<QString> > *foreign_keys = nullptr,
                     const QVector<QString>* unique_keys = nullptr);
    int delete_table(const QString &tablename);
    int show_tables();
    int show_items(const QString& tablename,
                   const QVector<QString>& fields,
                   const QString& key,
                   const QString& value,
                   QVector<QVector<QString> >& items,
                   const QString& additional="");
    int show_items(const QString &tablename,
                   const QVector<QString> &fields,
                   const QVector<QPair<QString, QString> >& key_value_pairs,
                   QVector<QVector<QString> >& items,
                   const QString& additional="");
    int show_items(const QString& tablename,
                   const QVector<QString> &fields,
                   QVector<QVector<QString> >& items,
                   const QString& constraint="",
                   const QString& additional="");
    int show_items_inner_join(const QVector<QPair<QString, QString> >& table_field_pairs,
                            const QVector<QPair<QString, QString> >& table_pairs,
                            const QVector<QPair<QString, QString> >& field_pairs,
                            QVector<QVector<QString> >& items,
                            const QString &constraint="",
                            const QString &additional="");
    int show_items_inner_join(const QVector<QString>& ext_fields,
                            const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& table_field_pairs,
                            QVector<QVector<QString> >& items,
                            const QString &constraint="",
                            const QString &additional="");
    int show_items_inner_join(const QVector<QString>& ext_fields,
                            const QVector<QPair<QPair<QString, QString>, QPair<QString, QString> > >& table_field_pairs,
                            QVector<QVector<QString> >& items,
                            const QVector<QPair<QString, QString> >& key_value_pairs,
                            const QString &additional="");

    int show_one_item(const QString &tablename,
                   QVector<QString>& item,
                   const QVector<QString> &fields,
                   const QString &key,
                   const QString &key_value);
    int show_one_item(const QString &tablename,
                   QVector<QString>& item,
                   const QVector<QString> &fields,
                   const QVector<QPair<QString, QString> >& key_value_pairs);
    int show_one_item(const QString &tablename,
                   QVector<QString>& item,
                   const QVector<QString> &fields,
                   const QString &constraint);

    int delete_items(const QString &tablename,
                   const QString &key,
                   const QString &key_value);
    int delete_items(const QString &tablename,
                     const QVector<QPair<QString, QString> >& key_value_pairs);
    int delete_items(const QString &tablename,
                     const QString &constraint="");
    int insert_item(const QString &tablename,
                    const QVector<QString> &fields,
                    const QVector<QString> &values);

    int update_items(const QString &tablename,
                   const QVector<QPair<QString, QString>> &key_value_pairs,
                    const QVector<QPair<QString, QString>> &field_value_pairs);
    int update_items(const QString &tablename,
                   const QString& key, const QString& value,
                    const QVector<QPair<QString, QString>> &field_value_pairs);

    QString get_error_message() {return _error_message;}

private:
    int execute(const QString &command);
    int execute_query(const QString &command);
    const QString _database;
    const QString _username;
    const QString _hostname;
    const static QString _root_password;
    QString _error_message;
    sql::Driver* _driver;
    std::unique_ptr< sql::Connection > _con;
    std::unique_ptr< sql::Statement > _stmt;
    std::unique_ptr< sql::ResultSet > _res;
    std::unique_ptr< sql::PreparedStatement > _pstmt;
};

#endif // DATABASE_HANDLER_H
