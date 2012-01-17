#include <windows.h>
#include <lmcons.h> // UNLEN
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include "DBConnector.h"

DBConnector::DBConnector() {
    char winUserName[UNLEN + 1];
    DWORD winUserNameSize = sizeof(winUserName);
    if (GetUserNameA(winUserName, &winUserNameSize))
      m_username = QString::fromLocal8Bit(winUserName);
    else
      QMessageBox::warning(NULL, QObject::tr("sctime: Datenbankverbindung"), QObject::tr("kein Benutzername"));

    QFile pwdfile(QDir::homePath()+".Zeit");
    if (!pwdfile.open(QIODevice::ReadOnly)) {
      pwdfile.setFileName("H:\\.Zeit");
      if (!pwdfile.open(QIODevice::ReadOnly)) {
	m_password = m_username; // the default password is the username
	goto pwd_done;
      }
    }
    m_password = QTextStream(&pwdfile).readLine();
    pwd_done:;
}

void DBConnector::configureDB(QSqlDatabase& db)
{
  db.setDatabaseName("DSN=Postgres_Zeit;SERVER=zeitdabaserv;");
  if (!m_username.isNull()) db.setUserName(m_username);
  db.setPassword(m_password);
}
