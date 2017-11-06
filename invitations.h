#ifndef INVITATIONS_H
#define INVITATIONS_H

#include <QDialog>
#include "connection.h"

namespace Ui {
class invitations;
}

class invitations : public QDialog
{
    Q_OBJECT

public:
    explicit invitations(QString myuser, QWidget *parent = 0);
    ~invitations();
    connection myconn;


private:
    Ui::invitations *ui;
};

#endif // INVITATIONS_H
