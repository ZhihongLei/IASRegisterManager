#ifndef PLAIN_TEXT_EDIT_H
#define PLAIN_TEXT_EDIT_H

#include <QPlainTextEdit>

class QCompleter;
class PlainTextEdit: public QPlainTextEdit
{
public:
    PlainTextEdit(QWidget* parent);
    ~PlainTextEdit();

    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    QString textUnderCursor() const;
    QString getPreviousKChars(int i, int k) const;
    bool inMathMode() const;

private:
    QCompleter *c;
};

#endif // PLAIN_TEXT_EDIT_H
