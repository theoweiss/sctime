#include "setupdsm.h"

#ifdef WIN32
# include <windows.h>
# include <lmcons.h> // UNLEN
#else
# include <unistd.h>
#endif
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include "sctimexmlsettings.h"
#include "datasource.h"
#include "globals.h"

DatasourceManager* kontenDSM;
DatasourceManager* bereitDSM;

static const
QString kontenQuery(
  "Select  "
  "   gb.name, " // 0
  "   team.kostenstelle, "
  "   konto.name,  "
  "   f_username(konto.verantwortlich), " // 3
  "   f_username(coalesce(konto.stellvertreter, konto.verantwortlich)), "
  "   konto.abgerechnet_bis, "
  "   konto.zeitlimit, "  // 6
  "   u.name, "
  "   f_username(coalesce(u.verantwortlich, konto.verantwortlich)), "
  "   f_username(coalesce(u.stellvertreter, u.verantwortlich, konto.verantwortlich)), " // 9
  "   coalesce(unterkonto_art.name || ' (' || u.art || ')', u.art), "
  "   coalesce(u.beschreibung, '') || coalesce('; noch nicht abgerechnet: ' || (get_budget_saldo(u.unterkonto_id)::numeric(8,2)), ''), "
  "   coalesce(uk.kommentar, ''), " // 12
  "   coalesce(u.intercompany_id, '(keine PSP)') "
  "From "
  "  gb "
  "  join konto on (gb.gb_id = konto.gb_id) "
  "  join team on (team.team_id = konto.team_id)  "
  "  join unterkonto u on (u.konto_id = konto.konto_id) "
  "  join unterkonto_art on (u.art = unterkonto_art.art) "
  "  left join unterkonto_kommentar uk on (u.unterkonto_id = uk.unterkonto_id) "
  "Where "
  " u.eintragbar "
  "Order By gb.name, konto.name, u.name, uk.kommentar ");

static QString username() {
  static QString result;
  if (!result.isNull())
    return result;
#ifdef WIN32
    char winUserName[UNLEN + 1];
    DWORD winUserNameSize = sizeof(winUserName);
    if (GetUserNameA(winUserName, &winUserNameSize))
      result = QString::fromLocal8Bit(winUserName);
#else
    char *login = getlogin();
    if (login)
      result = QString::fromLocal8Bit(login);
#endif
    if (result.isEmpty()) {
      result = "";
      logError(QObject::tr("user name cannot be determined."));
    }
    return result;
}

static QString password() {
  static QString result;
  if (!result.isNull())
    return result;
  result = username(); // the username is the default password
  // try to read password from a file
  QList<QString> pwdfilepaths;
  pwdfilepaths << canonicalPath("~/.Zeit");
  QString p;
  foreach (p, pwdfilepaths) {
    QFile pwdfile(p);
    if (pwdfile.open(QIODevice::ReadOnly)) {
      QTextStream qs(&pwdfile);
      result = qs.readLine();
      break;
    } else
      logError(QObject::tr("Error when reading from file %1: %2").arg(p, pwdfile.errorString()));
  }
  return result;
}

static const QString bereitQuery("SELECT kategorie, beschreibung FROM v_bereitschaft_sctime");

void setupDatasources(const QStringList& datasourceNames,
                      const SCTimeXMLSettings& settings,
                      const QString &kontenPath, const QString &bereitPath)
{
  kontenDSM = new DatasourceManager(QObject::tr("Accounts"));
  bereitDSM = new DatasourceManager(QObject::tr("On-call categories"));
  trace(QObject::tr("available database drivers: %1.").arg(QSqlDatabase::drivers().join(", ")));
  if (!kontenPath.isEmpty())
    kontenDSM->sources.append(new FileReader(kontenPath, "|", 13));
  if (!bereitPath.isEmpty())
    bereitDSM->sources.append(new FileReader(bereitPath, "|", 2));
  QString dsname;
  foreach (dsname, datasourceNames) {
    if (dsname.compare("file") == 0) {
      kontenDSM->sources.append(new FileReader(configDir.filePath("zeitkonten.txt"), "|", 13));
      bereitDSM->sources.append(new FileReader(configDir.filePath("zeitbereitls.txt"), "|", 2));
    } else if (dsname.compare("command") == 0) {
#ifdef WIN32
      logError(QObject::tr("data source 'command' is not available on Windows"));
#else
      kontenDSM->sources.append(new CommandReader("zeitkonten --mikrokonten --psp --separator='|'", "|", 14));
      bereitDSM->sources.append(new CommandReader("zeitbereitls --separator='|'", "|", 2));
#endif
    } else {
      if (!QSqlDatabase::drivers().contains(dsname)) {
        logError(QObject::tr("database driver or data source not available: ") + dsname);
        continue;
      }
      QSqlDatabase db = QSqlDatabase::addDatabase(dsname, dsname);
      if (!db.isValid() || db.isOpenError()) {
        logError(QObject::tr("data source '%1' not working: %2").arg(dsname, db.lastError().driverText()));
        continue;
      }
      db.setDatabaseName(
	  dsname.startsWith("QODBC") 
	    ? "DSN=Postgres_Zeit;DRIVER=PostgreSQL UNICODE" 
	    : settings.database);
      db.setHostName(settings.databaseserver);

      QString un = settings.databaseuser;
      if (un.isEmpty())
	un = username();
      db.setUserName(un);

      QString pw = settings.databasepassword;
      if (pw.isEmpty())
	pw = password();
      db.setPassword(pw);

      kontenDSM->sources.append(new SqlReader(db, kontenQuery));
      bereitDSM->sources.append(new SqlReader(db,  bereitQuery));
    }
  }
}
