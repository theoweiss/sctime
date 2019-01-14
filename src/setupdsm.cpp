/*
    Copyright (C) 2018 science+computing ag
       Authors: Johannes Abt et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


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
#include "JSONReader.h"
#include "sctimexmlsettings.h"
#include "datasource.h"
#include "globals.h"

DatasourceManager* kontenDSM;
DatasourceManager* bereitDSM;
DatasourceManager* specialRemunDSM;

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
  "   coalesce(u.intercompany_id, '(keine PSP)'), "
  "   (select coalesce(string_list(sz.kategorie),'') from t_sonderzeiten_unterkonto szu join t_sonderzeiten sz on (szu.id_sonderzeiten=sz.id) where szu.id_unterkonto=u.unterkonto_id), "
  "   coalesce(uk.kommentar, '') " // 15
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

static const QString bereitQuery("SELECT kategorie, beschreibung FROM v_bereitschaft_sctime");

static const QString specialRemunQuery(
  "Select "
  "    sz.kategorie, "
  "    sz.beschreibung, "
  "    sz.isglobal "
  "From "
  "    v_sonderzeiten_sctime sz "
  "    order by sz.kategorie");

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

void setupDatasources(const QStringList& datasourceNames,
                      const SCTimeXMLSettings& settings,
                      const QString &kontenPath, const QString &bereitPath, const QString &specialremunPath, const QString &jsonPath)
{
  kontenDSM = new DatasourceManager(QObject::tr("Accounts"));
  bereitDSM = new DatasourceManager(QObject::tr("On-call categories"));
  specialRemunDSM = new DatasourceManager(QObject::tr("Special Remunerations"));
  trace(QObject::tr("available database drivers: %1.").arg(QSqlDatabase::drivers().join(", ")));
  JSONReader *jsonreader=NULL;
  if (!kontenPath.isEmpty())
    kontenDSM->sources.append(new FileReader(kontenPath, "|", 15));
  if (!bereitPath.isEmpty())
    bereitDSM->sources.append(new FileReader(bereitPath, "|", 2));
  if (!specialremunPath.isEmpty())
    specialRemunDSM->sources.append(new FileReader(specialremunPath, "|", 2));
  if (!jsonPath.isEmpty()) {
    trace(QObject::tr("adding jsonreader: %1.").arg(jsonPath));
    jsonreader=new JSONReader(jsonPath);
    kontenDSM->sources.append(new JSONAccountSource(jsonreader));
    bereitDSM->sources.append(new JSONOnCallSource(jsonreader));
    specialRemunDSM->sources.append(new JSONSpecialRemunSource(jsonreader));
  }
  QString dsname;
  foreach (dsname, datasourceNames) {
    if (dsname.compare("json") == 0) {
      jsonreader=new JSONReader(configDir.filePath("sctime-offline.json")); 
      kontenDSM->sources.append(new JSONAccountSource(jsonreader));
      bereitDSM->sources.append(new JSONOnCallSource(jsonreader));
      specialRemunDSM->sources.append(new JSONSpecialRemunSource(jsonreader));
    } else
    if (dsname.compare("file") == 0) {
      kontenDSM->sources.append(new FileReader(configDir.filePath("zeitkonten.txt"), "|", 15));
      bereitDSM->sources.append(new FileReader(configDir.filePath("zeitbereitls.txt"), "|", 2));
      specialRemunDSM->sources.append(new FileReader(configDir.filePath("sonderzeitls.txt"), "|", 3));
    } 
    else if (dsname.compare("command") == 0) {
#ifdef WIN32
      logError(QObject::tr("data source 'command' is not available on Windows"));
#else
      kontenDSM->sources.append(new CommandReader("zeitkonten --mikrokonten --psp --sonderzeiten --separator='|'", "|", 15));
      bereitDSM->sources.append(new CommandReader("zeitbereitls --separator='|'", "|", 2));
      specialRemunDSM->sources.append(new CommandReader("sonderzeitls --separator='|'", "|", 3));
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
      bereitDSM->sources.append(new SqlReader(db, bereitQuery));
      specialRemunDSM->sources.append(new SqlReader(db, specialRemunQuery));
    }
  }
}
