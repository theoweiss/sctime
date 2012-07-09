#include "datasource.h"

#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include "globals.h"

static bool readFile(DSResult* const result, QTextStream &ts, const QString &sep, int columns, const QString &path);

Datasource::Datasource():broken(false) {}

DatasourceManager::DatasourceManager(const QString& name):name(name) {}

DatasourceManager::~DatasourceManager() {
  Datasource *ds;
  foreach (ds, sources) delete ds;
}

void DatasourceManager::start() {
  Datasource *ds;
  DSResult result;
  foreach (ds, sources) {
    if (ds->broken) continue;
    if (ds->read(&result)) {
      emit finished(result);
      return;
    }
    result.clear();
  }
  logError(QObject::tr("%1: no data source available").arg(name));
  emit aborted();
}

FileReader::FileReader(const QString &path, const QString&  columnSeparator, int columns)
  :Datasource(), path(path), sep(columnSeparator), columns(columns) {}

bool FileReader::read(DSResult* const result) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    logError(QObject::tr("file '%1' cannot be opened: %2").arg(path, file.errorString()));
    broken = true;
    return false;
  }
  trace(QObject::tr("Reading ") + path);
  QTextStream ts(&file);
  return readFile(result, ts, sep, columns, path);
}

static bool readFile(DSResult* const result, QTextStream &ts, const QString& sep, int columns, const QString &path) {
  int lines = 0;
  while (!ts.atEnd()) {
    QString l = ts.readLine();
    lines++;
    if (l.isEmpty()) continue;
    QStringList vl;
    int start = 0;
    // Der Separator soll in der letzten Spalte als einfaches Zeichen behandelt werden soll.
    // Dafür gibt es keine Methode in QString. Deswegen hier per Hand:
    for (int i = 0; i < columns - 1; i++) {
      int end = l.indexOf(sep, start);
      if (end == -1) {
        logError(QObject::tr("Line %1 of '%2' has only %3 columns instead of %4").arg(lines).arg(path).arg(i + 1).arg(columns));
        return false;
      }
      vl << l.mid(start, end - start);
      start = end + sep.length();
    }
    vl << l.mid(start); // das letzte Element enthält den ganzen Rest
    result->append(vl);
  }
  return true;
}

SqlReader::SqlReader(QSqlDatabase db, const QString &cmd):cmd(cmd),db(db) {}

bool  SqlReader::read(DSResult* const result) {
  logError(QObject::tr("Connecting to database %1 on %2 with driver %3 as user %4")
           .arg(db.databaseName(), db.hostName(), db.driverName(), db.userName()));
  if (!db.open()) {
    logError(QObject::tr("connection failed: ") + db.lastError().databaseText());
    return false;
  }
  QSqlQuery query(cmd, db);
  if (!query.isActive()) {
    logError(QObject::tr("Error ('%1') when executing query: %2").arg(db.lastError().databaseText()));
    broken = true;
    db.close();
    return false;
  }
  int cols = query.record().count();
  while (query.next()) {
    QStringList row;
    for (int i = 0; i < cols; i++)
      row.append(query.value(i).toString());
    result->append(row);
  }
  db.close();
  return true;
}

#ifndef WIN32
#include <stdlib.h>
#include <errno.h>
#include <langinfo.h>
CommandReader::CommandReader(const QString &command, const QString& columnSeparator, int columns)
  :Datasource(), command(command), sep(columnSeparator), columns(columns) {}

bool CommandReader::read(DSResult* const result) {
  FILE *file = popen(command.toLocal8Bit(), "r");
  if (!file) {
    logError(QObject::tr("Cannot run command '%1': %s2").arg(command, strerror(errno)));
    broken = true;
    return NULL;
  }
  trace(QObject::tr("Running command: ") + command);
  QTextStream ts(file, QIODevice::ReadOnly);
  ts.setCodec(nl_langinfo(CODESET));
  bool ok = readFile(result, ts, sep, columns, command);
  int rc = pclose(file);
  if (rc == -1 || !WIFEXITED(rc) || WEXITSTATUS(rc)) {
    logError(QObject::tr("Error when executing command '%1': %2").arg(command).arg(rc == -1 ? strerror(errno) : QObject::tr("abnormal termination")));
    broken = true;
    return false;
  }
  return ok;
}
#endif
