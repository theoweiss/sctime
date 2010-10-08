#include "DBConnector.h"
#include <windows.h>
#include <lmcons.h>
#include <QSqlDatabase>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QMessageBox>

DBConnector::DBConnector()
{
#if defined(UNICODE)
  if ( QSysInfo::windowsVersion() & QSysInfo::WV_NT_based )
  {
    TCHAR winUserName[UNLEN + 1];
    DWORD winUserNameSize = sizeof(winUserName);
    GetUserName( winUserName, &winUserNameSize );
    m_username = QString::fromUtf16( (ushort*)winUserName );
  } else
#endif
  {
    char winUserName[UNLEN + 1];
    DWORD winUserNameSize = sizeof(winUserName);
    GetUserNameA( winUserName, &winUserNameSize );
    m_username = QString::fromLocal8Bit( winUserName );
  }

  bool pwdfound=false;
  QFile pwdfile(QDir::homePath()+".Zeit");
  if (!pwdfile.open(QIODevice::ReadOnly))
  {
    pwdfile.setFileName("H:\\.Zeit");

    if (!pwdfile.open(QIODevice::ReadOnly))
    {
      m_password=m_username;
    }
    else pwdfound=true;
  } else pwdfound=true;

  if (pwdfound) {
    QTextStream stream( &pwdfile );
    m_password = stream.readLine();
    pwdfile.close();
  }

}

void DBConnector::configureDB(QSqlDatabase& db)
{
  db.setDatabaseName("DSN=Postgres_Zeit;SERVER=zeitdabaserv;PORT=5432;DATABASE=zeit;");
  db.setUserName(m_username);
  db.setPassword(m_password);
}
