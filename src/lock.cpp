
#include "lock.h"

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


/* Ich arbeite mit Lockfiles, weil ich mit "byte range locks"
   und -- unter Windows -- "sharing modes" (_sopen_s) -- nichts gefunden habe,
   das gleichzeitig auf Windows und Linux funktioniert.

   Das Lockfile soll Hostnamen und PID durch Leerzeichen getrennt enthalten.
*/

Lock::Lock():next(NULL), acquired(false) {}

Lock::~Lock() {}

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

bool Lock::check() {
  if (!_check()) return false;
  if (next && !next->_check()) {
    errStr = next->errorString();
    return false;
  }
  return true;
}

#ifdef WIN32
LockLocal::LockLocal(const QString& name, bool user):path((user ? "Local\\" : "Global\\" ) + name),name(name),user(user) {}

bool LockLocal::_acquire() {
  handle = CreateEventA(NULL, false, true, name.toLocal8Bit());
  if (!handle) {
    errStr = QObject::tr("Cannot create lock %1: %2").arg(name).arg(GetLastError());
    return false;
  }
  if (GetLastError () == ERROR_ALREADY_EXISTS) {
    errStr = QObject::tr("This Program (%1) is already running (on this machine)").arg(name);
    return false;
  }
  return true;
}

bool LockLocal::_release() {
  if (!CloseHandle(handle)) {
      errStr = QObject::tr("Error when removing local lock for this program (%1)").arg(name);
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
         ? QObject::tr("This Program (%1) is already running (on this machine as this user) (%2)").arg(name, path)
         : QObject::tr("This Program (%1) is already running (on this machine) (%2)").arg(name, path))
      : QObject::tr("Error when locking file %1: %2").arg(path, strerror(errno));
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
            if (words.size() >= 1 && words[0].compare(host) == 0)
              return true; // Das Lockfile stammt von einem Absturz auf diesem Rechner. Wir übernehmen es.
          }
          errStr = QObject::tr("This Program is already running on another machine (%1: %2).\n").arg(path, line);
          if (localExclusionProvided)
            errStr.append(QObject::tr(
                            "\nIf this is not the case and the lock file is left over from a crash on "
			    "that other machine, then just once run the program again on that machine "
			    "to clean up the lock file.\n\n"
                            "A potentially dangerous alternative is to remove the lock file %1 manually.").arg(path));
          return false;
        }
      }
      errStr = QObject::tr("%1 already exists.").arg(path);
    return false;
  }
  if (fd == -1) {
    errStr = QObject::tr("%1 could not be created: %2").arg(path, strerror(errno));
    return false;
  }
  // Konnte das Lockfile anlegen
  QFile f(path);
  bool rv;
  if (!f.open(fd, QFile::WriteOnly)) {
      errStr = QObject::tr("lock file %1 could not be opened: %1").arg(path, f.errorString());
      rv = false;
  } else {
    QTextStream out(&f);
    out << QHostInfo::localHostName();
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
  errStr = QObject::tr("lock file %1 could not be removed: %2").arg(path, p.errorString());
  return false;
}

bool Lockfile::_check() {
  QFile f(path);
  if (f.open(QIODevice::ReadOnly)) {
    QTextStream in(&f);
    QString line = in.readLine();
    if (line.compare(QHostInfo::localHostName()) == 0)  return true;
    errStr = QObject::tr("lock file %1 has been changed by someone else: %2").arg(path, line);
  }
  else
    errStr = QObject::tr("lock file %1 has been deleted by someone else.").arg(path);
return false;
}
