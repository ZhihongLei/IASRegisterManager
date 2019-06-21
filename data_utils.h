#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <QRegExpValidator>
#include <QVector>
#include <QString>

class HexValueValidator: public QRegExpValidator
{
public:
    explicit HexValueValidator(int bits, QObject * parent = nullptr);
    State validate(QString &input, int &pos) const;
private:
    const int bits_;
};

QVector<QVector<QString> > sort_doubly_linked_list (const QVector<QVector<QString> >& items, int id_field = 0, int prev_id_field = -2, int next_id_field = -1);

enum DIALOG_MODE {ADD, EDIT};

class Naming
{
public:
    explicit Naming();
    void set_naming_template(const QString& naming_template);
    QString get_extended_name(const QString& shortened_name);
    QString get_shortened_name(const QString& extended_name);
    void update_key(const QString& key, const QString& value);
    void clear();
private:
    QHash<QString, QString> key2value;
    QString naming_template;
};

extern Naming REGISTER_NAMING;
extern Naming SIGNAL_NAMING;

QString decimal2hex(QString n, int address_width);
QString decimal2hex(int n, int address_width);
QString decimal2hex(long n, int address_width);
QString decimal2hex(long long n, int address_width);

QString normalize_hex(const QString& hex, int address_width);

#endif // DATA_UTILS_H
