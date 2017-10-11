#ifndef EVENTMANAGE_H
#define EVENTMANAGE_H

#include <QDialog>

namespace Ui {
class eventmanage;
}

class eventmanage : public QDialog
{
    Q_OBJECT

public:
    explicit eventmanage(QWidget *parent = 0);
    ~eventmanage();

private:
    Ui::eventmanage *ui;
};

#endif // EVENTMANAGE_H
