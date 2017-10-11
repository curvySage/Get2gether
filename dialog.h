#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>
#include <connection.h>

namespace Ui {
class Dialog;
}

// PURPOSE: This class is used to display event info window
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    connection myconn;
    QString myuser;
    void setUser(QString u);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
