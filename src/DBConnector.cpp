#include "DBConnector.h"

#ifdef WIN32
# include <windows.h>
# include <lmcons.h> // UNLEN
#else
# include <unistd.h>
#endif
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

DBConnector::DBConnector() {
#ifdef WIN32
    char winUserName[UNLEN + 1];
    DWORD winUserNameSize = sizeof(winUserName);
    if (GetUserNameA(winUserName, &winUserNameSize))
      m_username = QString::fromLocal8Bit(winUserName);
#else
    char *login = getlogin();
    if (login)
      m_username = QString::fromLocal8Bit(login);
#endif

    if (m_username.isNull())
      QMessageBox::warning(NULL, QObject::tr("sctime: Datenbankverbindung"), QObject::tr("kein Benutzername"));

    m_password = m_username; // the username is the default password

    // try to read password from a file
    QList<QString> pwdfilepaths;
    pwdfilepaths << QDir::homePath() + QDir::separator() + ".Zeit";
#ifdef WIN32
    // try drive H: on Windows
    pwdfilepaths << "H:\\.Zeit";
#endif

    while (!pwdfilepaths.isEmpty()) {
      QFile pwdfile(pwdfilepaths.takeFirst());
      if (pwdfile.open(QIODevice::ReadOnly)) {
        QTextStream qs(&pwdfile);
        m_password = qs.readLine();
        pwdfile.close();

        // stop searching
        break;
      }

      if (pwdfile.exists())
        QMessageBox::warning(NULL, QObject::tr("sctime: Passwortdatei"),
	  QObject::tr("sctime: Passwortdatei") + " " + pwdfile.fileName() + " "
	  + QObject::tr("existiert aber ist nicht lesbar"));
    }
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
