#include "connection.h"

// PURPOSE: default constructor
connection::connection()
{
}

// PURPOSE: opens connection to database
void connection::openConn() {
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("35.203.187.201");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("get2gether");
    db.open();
}

// PURPOSE: closes current connection to database
void connection::closeConn() {
    db.close();
    db.removeDatabase("QMYSQL");
}
