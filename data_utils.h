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


#endif // DATA_UTILS_H
