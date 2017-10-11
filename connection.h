#ifndef CONNECTION_H
#define CONNECTION_H
#include <QSql>
#include <QSqlDatabase>

// This class is used to make connection to the database.
// When opening a new window, old connection must be closed and then open new connection.
class connection
{
public:
    QSqlDatabase db;
    connection();
    void openConn();
    void closeConn();
};

#endif // CONNECTION_H
