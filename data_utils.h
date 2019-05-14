#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <QRegExpValidator>

class HexValueValidator: public QRegExpValidator
{
public:
    explicit HexValueValidator(int bits, QObject * parent = nullptr);
    State validate(QString &input, int &pos) const;
private:
    const int bits_;
};

#endif // DATA_UTILS_H
