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
  logError(QObject::tr("%1: keine Datenquelle verfügbar").arg(name));
  emit aborted();
}

FileReader::FileReader(const QString &path, const QString& sep, int columns)
  :Datasource(), path(path), sep(sep), columns(columns) {}

bool FileReader::read(DSResult* const result) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    logError(QObject::tr("kann  Datei '%1' nicht öffnen: %2").arg(path, file.errorString()));
    broken = true;
    return false;
  }
  trace(QObject::tr("Lese ") + path);
  QTextStream ts(&file);
  return readFile(result, ts, sep, columns, path);
}

static bool readFile(DSResult* const result, QTextStream &ts, const QString& sep, int columns, const QString &path) {
  int lines = 0;
  for (QString l; !(l = ts.readLine()).isNull();) {
    QStringList vl;
    lines++;
    if (l.isEmpty()) continue;
    int start = 0;
    for (int i = 0; i < columns - 1; i++) {
      int end = l.indexOf(sep, start);
      if (end == -1) {
        delete result;
        logError(QObject::tr("Zeile %1 von '%2' hat nur %3 Spalten statt %4").arg(lines).arg(path).arg(i + 1).arg(columns));
        return false;
      }
      vl.append(l.mid(start, end - start));
      start = end + sep.length();
    }
    vl.append(l.mid(start)); // das letzte Element enthält den ganzen Rest
    result->append(vl);
  }
  return true;
}

SqlReader::SqlReader(QSqlDatabase db, const QString &cmd):cmd(cmd),db(db) {}

bool  SqlReader::read(DSResult* const result) {
  logError(QObject::tr("Verbindungsaufbau zu Datenbank %1 auf %2 mit Treiber %3 als Benutzer %4")
           .arg(db.databaseName(), db.hostName(), db.driverName(), db.userName()));
  if (!db.open()) {
    logError(QObject::tr("Verbindungsaufbau fehlgeschlagen: ") + db.lastError().databaseText());
    return false;
  }
  QSqlQuery query(cmd, db);
  if (!query.isActive()) {
    logError(QObject::tr("Fehler ('%1') in Abfrage: %2").arg(db.lastError().databaseText()));
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
CommandReader::CommandReader(const QString &command, const QString& sep, int columns)
  :Datasource(), command(command), sep(sep), columns(columns) {}

bool CommandReader::read(DSResult* const result) {
  FILE *file = popen(command.toLocal8Bit(), "r");
  if (!file) {
    logError(QObject::tr("Kann Kommando '%1' nicht ausfuehren: %s2").arg(command, strerror(errno)));
    broken = true;
    return NULL;
  }
  trace(QObject::tr("Führe aus: ") + command);
  QTextStream ts(file, QIODevice::ReadOnly);
  ts.setCodec("UTF-8");
  bool ok = readFile(result, ts, sep, columns, command);
  int rc = pclose(file);
  if (rc == -1 || !WIFEXITED(rc) || WEXITSTATUS(rc)) {
    logError(QObject::tr("Fehler bei '%1': %2").arg(command).arg(rc == -1 ? strerror(errno) : "nicht normal beendet"));
    broken = true;
    return false;
  }
  return ok;
}
#endif
