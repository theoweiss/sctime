
#include <errno.h>
#include <string.h> // strerror
#include <sys/stat.h> // (_)S_IWRITE
#include <fcntl.h> // (_)O_RDWR

#ifdef WIN32
#include <Windows.h>
#include <io.h>

#else // !WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <QtNetwork/QHostInfo>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QTextStream>

#include "lock.h"

/* Ich arbeite mit Lockfiles, weil ich mit "byte range locks"
   und -- unter Windows -- "sharing modes" (_sopen_s) -- nichts gefunden habe,
   das gleichzeitig auf Windows und Linux funktioniert.

   Das Lockfile soll Hostnamen und PID durch Leerzeichen getrennt enthalten.
*/

Lock::Lock():next(NULL), acquired(false) {}

bool Lock::acquire() {
  if (acquired) return true;
  if (!_acquire()) return false;
  if (next && !next->acquire()) {
    errStr = next->errorString();
    return false;
  }
  acquired = true;
  return true;
}

bool Lock::release() {
  bool rv = true;
  if (!acquired) return true;
  acquired = false; // wenn's beim 'release' Fehler gibt, dann ist der Zustand unsicher
  if (next &&(rv = next->_release()))
    errStr = next->errorString();
  return (_release() && rv);
}

#ifdef WIN32
LockLocal::LockLocal(const QString& name, bool user):path((user ? "Local\\" : "Global\\" ) + name),name(name),user(user) {}

bool LockLocal::_acquire() {
  handle = CreateEventA(NULL, false, true, name.toLocal8Bit());
  if (!handle) {
    err = QObject::tr("Kann die lokale Sperre %1 nicht anlegen: %2").arg(name).arg(GetLastError());
    return false;
  }
  if (GetLastError () == ERROR_ALREADY_EXISTS) {
    err = QObject::tr("Das Programm „%1“ läuft bereits (auf diesem Rechner)").arg(name);
    return false;
  }
  return true;
}

bool LockLocal::_release() {
  if (!CloseHandle(handle)) {
      err = QString("Fehler aufgetreten beim Löschen der lokalen Sperre „%1“").arg(name)
      return false;
  }
  return true;
}

#else
LockLocal::LockLocal(const QString& name, bool user):user(user),name(name), path(QFileInfo(QDir::temp(), user ? name + "." + getenv("LOGNAME") : name).absoluteFilePath()) {}

bool LockLocal::_acquire() {
  fd = open(path.toLocal8Bit(), O_WRONLY | O_CREAT, 0600);
  if (fd == -1) {
    errStr = strerror(errno);
    return false;
  }
  if (lockf(fd,F_TLOCK,0) == -1) {
    errStr = errno == EACCES || errno == EAGAIN
      ? (user
         ? QObject::tr("Das Programm „%1“ läuft bereits (auf diesem Rechner und für diesen Benutzer) (%2)").arg(name, path)
         : QObject::tr("Das Programm „%1“ läuft bereits (auf diesem Rechner ) (%2)").arg(name, path))
      : QObject::tr("Fehler beim Sperren der Datei  „%1“: %2").arg(path, strerror(errno));
    close(fd);
    return false;
  }
  return true;
}

bool LockLocal::_release() {
   if (close(fd)) {
     errStr = strerror(errno);
     return false;
   }
   return true;
}
#endif

Lockfile::Lockfile(const QString& path, bool localExclusionProvided): path(path), localExclusionProvided(localExclusionProvided) {}

bool Lockfile::_acquire() {
  int fd = -1;
  for (int tries = 0; tries < 2; tries++) {
#ifdef WIN32
    fd = _open(path.toLocal8Bit(), _O_WRONLY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else
    fd = open(path.toLocal8Bit(), O_WRONLY | O_CREAT | O_EXCL, 0644);
#endif
    if (fd != -1 || errno != EEXIST) break;
      QFile f(path);
      if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        QString line = in.readLine();
        if (!line.isEmpty()) {
          if (localExclusionProvided) {
	    QString host(QHostInfo::localHostName());
            QStringList words = line.split(" ");
            if (words.size() >= 1 && words[0].compare(host) == 0) {
              QFile p(path);
              if (p.remove())
                continue; // nächster Versuch
              errStr = QObject::tr("Kann veraltete Sperre %1 nicht löschen: %2").arg(path, p.errorString());
              return false;
            }
          }
          errStr = QObject::tr("Das Programm bereits auf einem anderen Rechner (%1: „%2“).\n").arg(path, line);
          if (localExclusionProvided)
            errStr.append(QObject::tr(
                            "\nWenn das nicht so ist, sondern das Programm sich beim letzten Mal auf einem anderen Rechner nicht regulär beenden konnte",
                            "dann starten Sie bitte das Programm nochmal auf diesem anderen Rechner!\n"
                            "Eine gefährliche Alternative ist, die Datei %1 zu löschen.").arg(1));
          return false;
        }
      }
      errStr = QString("%1 existiert bereits.").arg(path);
    return false;
  }
  if (fd == -1) {
    errStr = QString("%1 konnte nicht angelegt werden (%2)").arg(path, strerror(errno));
    return false;
  }
  // Konnte das Lockfile anlegen
  QFile f(path);
  bool rv;
  if (!f.open(fd, QFile::WriteOnly)) {
      errStr = QObject::tr("Konnte das Lockfile %1 nicht öffnen (%1)").arg(path, f.errorString());
      rv = false;
  } else {
    QTextStream out(&f);
    out << QHostInfo::localHostName() << "\n";
    rv = true;
    f.close();
  }
#ifdef WIN32
  _close(fd);
#else
  close(fd);
#endif
  return rv;
}

bool Lockfile::_release() {
  QFile p(path);
  if (p.remove()) return true;
  err = QObject::tr("Konnte %1 nicht entfernen: %2").arg(path, p.errorString());
  return false;
}
