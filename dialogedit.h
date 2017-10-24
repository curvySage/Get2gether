#ifndef DIALOGEDIT_H
#define DIALOGEDIT_H

#include <QDialog>
#include <QString>
#include <connection.h>

namespace Ui {
class DialogEdit;
}

class DialogEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEdit(QWidget *parent = 0);
    ~DialogEdit();

    connection myconn;
    QString myuser;

    void SetInformation(QString Owner);

private:
    Ui::DialogEdit *ui;
};

#endif // DIALOGEDIT_H
