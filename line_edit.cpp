#include "line_edit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QDebug>

LineEdit::LineEdit(QWidget *parent)
: QLineEdit(parent), c(nullptr)
{
}

LineEdit::~LineEdit()
{
}

void LineEdit::setCompleter(QCompleter *completer)
{
    if (c) QObject::disconnect(c, 0, this, 0);
    c = completer;
    if (!c) return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, QOverload<const QString &>::of(&QCompleter::activated),
                     this, &LineEdit::insertCompletion);
}

QCompleter *LineEdit::completer() const
{
    return c;
}

void LineEdit::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;
    static QString bow = " \n\t";
    while (cursorPosition() - 1 >= 0 && !bow.contains(text().at(cursorPosition() - 1))) backspace();
    if (inMathMode())
    {
        QString r = completion;
        //r.replace('\\', "\textbackslash ");
        r.replace('^', "\textasciicircum ");
        r.replace('~', "\textasciitilde ");
        for (QChar c : {'#', '$', '%', '&', '_', '{', '}'})
            r.replace(c, '\\' + c);
        insert(r);
    }
    else insert(completion);

    //setCursorPosition()
    //setTextCursor(tc);
}

QString LineEdit::textUnderCursor() const
{
    static QString bow = " \n\t";
    int end = cursorPosition();
    int start = end - 1;
    QString text = this->text();
    while (start >= 0 && !bow.contains(text.at(start))) start--;
    return this->text().mid(start+1, end - start-1);
}

void LineEdit::focusInEvent(QFocusEvent *e)
{
    if (c) c->setWidget(this);
    QLineEdit::focusInEvent(e);
}

void LineEdit::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
        QLineEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    //static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    static QString eow;
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr); // popup it up!
}

QString LineEdit::getPreviousKChars(int i, int k) const
{
    if (i >= k) return text().mid(i - k, k);
    else return text().mid(0, i);
}

bool LineEdit::inMathMode() const
{
    int mathMode = 0;
    QString prev;
    for (int i = 0; i <= cursorPosition(); i++)
    {
        if (i + 1 <= cursorPosition() && getPreviousKChars(i+1, 2) == "$$")
        {
            mathMode = mathMode ? 0 : 1;
            i++;
            continue;
        }

        QString prev1 = getPreviousKChars(i, 1),
                prev2 = getPreviousKChars(i, 2),
                prev4 = getPreviousKChars(i, 4),
                prev6 = getPreviousKChars(i, 6);
        if (!mathMode)
        {
            if ((prev1 == "$" && prev2 != "\\$") || prev2 == "$$" ||
                    prev2 == "\\(" || prev2 == "\\[" || prev6 == "\\begin")
                mathMode = 1;
        }
        else {
            if (prev6 == "\\begin") mathMode++;
            else if ((prev1 == "$" && prev2 != "\\$") || prev2 == "$$" ||
                    prev2 == "\\)" || prev2 == "\\]" || prev4 == "\\end")
                mathMode--;
        }
    }

    return mathMode;
}
