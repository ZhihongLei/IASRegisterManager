#ifndef LINE_EDIT_H
#define LINE_EDIT_H


#include <QLineEdit>

class QCompleter;

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent = nullptr);
    ~LineEdit() override;

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
#endif // LINE_EDIT_H
