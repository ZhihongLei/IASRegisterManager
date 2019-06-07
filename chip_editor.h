#ifndef CHIP_EDITOR_H
#define CHIP_EDITOR_H

#include <QWidget>

namespace Ui {
class ChipEditor;
}

class ChipEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ChipEditor(QWidget *parent = nullptr);
    ~ChipEditor();

private:
    Ui::ChipEditor *ui;
};

#endif // CHIP_EDITOR_H
