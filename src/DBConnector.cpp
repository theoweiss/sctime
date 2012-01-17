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
	m_password = m_username; // the username is the default password
	goto pwd_done;
      }
    }
    m_password = QTextStream(&pwdfile).readLine();
    pwd_done:;
}

QSqlDatabase DBConnector::connect(const char* driver, const char* dbname) {
  QSqlDatabase db = QSqlDatabase::addDatabase(driver);
  if (db.isValid()) {
    db.setDatabaseName(dbname);
    db.setHostName("zeitdabaserv");
    db.open(m_username, m_password);
  }
  return db;
}

QSqlDatabase DBConnector::open() {
  QSqlDatabase db(connect("QODBC", "DSN=Postgres_Zeit"));
  if (!db.isOpen()) {
    db = connect("QPSQL", "zeit");
  }
  return db;
}
