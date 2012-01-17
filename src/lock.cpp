
#include <errno.h>
#include <string.h> // strerror
#include <sys/stat.h> // (_)S_IWRITE
#include <fcntl.h> // (_)O_RDWR

//#define getpid _getpid
#ifdef WIN32
#include <Windows.h>
#include <WinBase.h> // _open
#include <io.h>
#include <process.h> // _getpid

#else // !WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <QtNetwork/QHostInfo>
#include <QFile>
#include <QStringList>

#include "lock.h"

/* Ich arbeite mit Lockfiles, weil ich mit "byte range locks"
   und -- unter Windows -- "sharing modes" (_sopen_s) -- nichts gefunden habe,
   das gleichzeitig auf Windows und Linux funktioniert.

   Das Lockfile soll Hostnamen und PID durch Leerzeichen getrennt enthalten.
*/

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 300
#endif

static char _hostname[HOST_NAME_MAX + 1] = "";

static const char* lock_hostname() {
  if (!_hostname[0])
    // funktioniert leider nicht: QHostInfo::localHostName()
    if (gethostname(_hostname, HOST_NAME_MAX + 1))
	qWarning() << "Could not get the hostname:" << strerror(errno);
  return _hostname;
}

static QString lock_host_pid() {
  return QString("%1 %2\n").arg(lock_hostname()).arg(getpid());
}

// legt eine Datei an. Es ist ein Fehler, wenn diese bereits existiert.
// Der Inhalt der ersten Zeile wird dann Teil der Fehlermeldung.
// Der Rueckgabewert ist die Fehlermeldung. "isNull()" gilt, wenn alles OK ist.
// "content" wird in die Datei geschrieben.
QString lock_acquire(const QString &path, bool local_exclusion_already_provided) {
  QString status;
#ifdef WIN32
  int fd = _open(path.toLocal8Bit(), _O_WRONLY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else
  int fd = open(path.toLocal8Bit(), O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif
  if (fd == -1) {
    if (errno != EEXIST) 
      return QString("%1 konnte nicht angelegt werden (%2)").arg(path, strerror(errno));
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
      return QString("%1 existiert, kann aber nicht gelesen werden (%1).").arg(path, f.errorString());
    QTextStream in(&f);
    QString line = in.readLine();
    if (line.isEmpty())
      return QString("%1 existiert bereits.").arg(path);
    QString host(lock_hostname());
    QStringList words = line.split(" ");
    if (words.size() >= 2 && words[0].compare(host) == 0) {
      if (local_exclusion_already_provided)
	return status; // Das Lockfile ist von einer Instanz auf dem gleichen Rechner liegen geblieben.
#ifndef WIN32
      int pid = words[1].toInt();
      if (pid > 2 &&kill(pid, 0) == -1 && errno == ESRCH)
	return status; // Den (lokalen) Prozess, der das Lock hatte, gibt's nicht mehr.
#endif
    }
    return QString::fromUtf8("%1 gehört einem anderen Prozess (%2)").arg(path, line);
  }

  // fd == 0: Es hat geklappt.
  QFile f(path);
  if (!f.open(fd, QFile::WriteOnly)) {
      status = QString::fromUtf8("Konnte das Lockfile %1 nicht öffnen (%1)").arg(path, f.errorString());
  } else {
    QTextStream out(&f);
    out << lock_host_pid();
    f.close();
  }
#ifdef WIN32
  _close(fd);
#else
  close(fd);
#endif
  return status;
}

QString lock_release(const QString &path) {
  return QFile::remove(path.toUtf8())
    ? QString() // TODO: Grund abfragen
    : QString("Konnte %1 nicht entfernen").arg(path);
}


Lock::Lock(const QString& path, const QString &name)
  : path(path), name(name), acquired(false) {}

#ifdef WIN32
QString Lock::acquire() {
  if (acquired) return QString();
    local_lock = CreateEventA(NULL, false, true, name.toLocal8Bit());
    if (GetLastError () == ERROR_ALREADY_EXISTS)
	return QString::fromUtf8("%1 läuft bereits auf dieser Maschine").arg(name);
    QString err = lock_acquire(path, true);
    acquired = err.isNull();
    if (!acquired && !CloseHandle(local_lock)) 
	err += QString("; Konnte Sperre '%1' nicht aufgeben").arg(name);
    return err;
}

#else
QString Lock::acquire() {
  if (acquired) return QString();
    QString err = lock_acquire(path, false);
    acquired = err.isNull();
    return err;
}
#endif

QString Lock::release() { 
    if (!acquired) return QString();
    QString err = lock_release(path);
    acquired = false;
#ifdef WIN32
    if (!acquired && !CloseHandle(local_lock)) 
	return (err.isNull() ? "" : err + "; ") + QString("Konnte Sperre '%1' nicht aufgeben").arg(name);
#endif
    return err;
}

Lock::~Lock() { release(); }

