#ifndef GROUPPOPUP_H
#define GROUPPOPUP_H

#include <QDialog>

namespace Ui {
class GroupPopUp;
}

class GroupPopUp : public QDialog
{
    Q_OBJECT

public:
    explicit GroupPopUp(QWidget *parent = 0);
    ~GroupPopUp();

private:
    Ui::GroupPopUp *ui;
};

#endif // GROUPPOPUP_H
