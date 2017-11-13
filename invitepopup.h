#ifndef INVITEPOPUP_H
#define INVITEPOPUP_H

#include <QDialog>

namespace Ui {
class InvitePopUp;
}

class InvitePopUp : public QDialog
{
    Q_OBJECT

public:
    explicit InvitePopUp(QWidget *parent = 0);
    ~InvitePopUp();

private:
    Ui::InvitePopUp *ui;
};

#endif // INVITEPOPUP_H
