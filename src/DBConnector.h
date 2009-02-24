#ifndef DBCONNECTOR_H
#define DBCONNECTOR_H

#include <QString>

class QSqlDatabase;

class DBConnector
{
   public:
	DBConnector();
	void configureDB(QSqlDatabase& db);
   private:
	QString m_username;
        QString m_password;
};

#endif
