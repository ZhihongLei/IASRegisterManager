#include "data_utils.h"
#include <QtMath>

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
        if ((uint64_t) qPow(2, bits_) > input.toULongLong(nullptr, 16)) return QValidator::Acceptable;
    }
    return QValidator::Invalid;

}
