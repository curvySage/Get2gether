#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dashboard.h>
#include <connection.h>


namespace Ui {
class MainWindow;
}

// PURPOSE: This class is used to display the login screen
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    connection myconn;
    QString myuser;
    QString mypass;

private slots:
    void on_login_button_clicked();
    void on_create_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
