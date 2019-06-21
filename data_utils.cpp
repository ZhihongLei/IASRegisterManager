#include "data_utils.h"
#include <QtMath>
#include <QHash>
#include <QDebug>

HexValueValidator::HexValueValidator(int bits, QObject * parent):
    QRegExpValidator (QRegExp("^(0x|0X)([0-9]|[a-f]|[A-F]){1," + QString::number(qCeil(bits/4.0)) + "}$"), parent),
    bits_(bits)
{

}

QValidator::State HexValueValidator::validate(QString &input, int &pos) const
{
    if (QRegExpValidator::validate(input, pos) == QValidator::Intermediate) return QValidator::Intermediate;
    if (QRegExpValidator::validate(input, pos) == QValidator::Acceptable)
    {
        if (qRound64(qPow(2, bits_)) > input.toULongLong(nullptr, 16)) return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}


QVector<QVector<QString> > sort_doubly_linked_list (const QVector<QVector<QString> >& items, int id_field, int prev_id_field, int next_id_field)
{
    if (items.size() ==  0) return QVector<QVector<QString> >();

    QVector<QVector<QString> > ans;

    QHash<QString, int> id2idx;
    if (prev_id_field < 0) prev_id_field += items[0].size();
    if (next_id_field < 0) next_id_field += items[0].size();

    int start = -1;
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i][prev_id_field] == "-1") start = i;
        id2idx[items[i][id_field]] = i;
    }
    if (start == -1) return QVector<QVector<QString> >();

    int next = start;
    while (true)
    {
        const auto& item = items[next];
        ans.push_back(item);
        if (item[next_id_field] == "-1") break;
        next = id2idx[item[next_id_field]];
    }
    return ans;
}

Naming REGISTER_NAMING;
Naming SIGNAL_NAMING;

Naming::Naming()
{

}

QString Naming::get_extended_name(const QString &shortened_name)
{
    QString naming = naming_template;
    for (const QString& key : key2value.keys()) naming.replace(key, key2value[key]);
    naming.replace("{NAME}", shortened_name);
    return naming;
}

QString Naming::get_shortened_name(const QString &extended_name)
{
    QString ref = get_extended_name("");
    int i = 0;
    while (i < ref.size() && ref[i] == extended_name[i]) i++;
    int j = 0, m = ref.size() - 1, n = extended_name.size() - 1;
    while (j < ref.size() && ref[m - j] == extended_name[n - j]) j++;
    return extended_name.mid(i, n - i - j + 1);
}

void Naming::set_naming_template(const QString &naming_template)
{
    Naming::naming_template = naming_template;
}

void Naming::update_key(const QString &key, const QString &value)
{
    key2value[key] = value;
}

void Naming::clear()
{
    naming_template.clear();
    key2value.clear();
}

QString decimal2hex(QString n, int address_width)
{
    return decimal2hex(n.toLongLong(), address_width);
}

QString decimal2hex(int n, int address_width)
{
    return decimal2hex((long long)n, address_width);
}
QString decimal2hex(long n, int address_width)
{
    return decimal2hex((long long)n, address_width);
}
QString decimal2hex(long long n, int address_width)
{
    QString hex = QString::number(n, 16).toUpper();
    hex = "0x" + QString(qCeil(address_width/4.0) - hex.size(), '0') + hex;
    return hex;
}

QString normalize_hex(const QString& hex, int address_width)
{
    long long n = hex.toLongLong(nullptr, 16);
    return decimal2hex(n, address_width);
}
