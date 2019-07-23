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

QString decimal2hex(QString n, int address_width)
{
    return decimal2hex(n.toULongLong(), address_width);
}

QString decimal2hex(quint32 n, int address_width)
{
    return decimal2hex((quint64)n, address_width);
}
QString decimal2hex(quint64 n, int address_width)
{
    QString hex = QString::number(n, 16).toUpper();
    hex = "0x" + QString(qCeil(address_width/4.0) - hex.size(), '0') + hex;
    return hex;
}

QString normalize_hex(const QString& hex, int address_width)
{
    quint64 n = hex.toULongLong(nullptr, 16);
    return decimal2hex(n, address_width);
}
