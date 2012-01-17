
#include <errno.h>
#include <string.h> // strerror
#include <sys/stat.h> // (_)S_IWRITE
#include <fcntl.h> // (_)O_RDWR

#ifdef WIN32
#define HOST_NAME_MAX 300
//#define getpid _getpid
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
QString lock_acquire(const QString &name, bool reliable_local_exclusion) {
  QString status;
#ifdef WIN32
  int fd = _open(name.toLocal8Bit(), _O_WRONLY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else
  int fd = open(name.toLocal8Bit(), O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif
  if (fd == -1) {
    if (errno != EEXIST) 
      return QString("%1 konnte nicht angelegt werden (%2)").arg(name, strerror(errno));
    QFile f(name);
    if (!f.open(QIODevice::ReadOnly))
      return QString("%1 existiert, kann aber nicht gelesen werden (%1).").arg(name, f.errorString());
    QTextStream in(&f);
    QString line = in.readLine();
    if (line.isEmpty())
      return QString("%1 existiert bereits.").arg(name);
    QString host(lock_hostname());
    QStringList words = line.split(" ");
    if (words.size() >= 2 && words[0].compare(host) == 0) {
      if (reliable_local_exclusion)
	return status; // Das Lockfile ist von einer Instanz auf dem gleichen Rechner liegen geblieben.
#ifndef WIN32
      int pid = words[1].toInt();
      qWarning() << "PID " << pid << "\n";
      if (pid > 2 &&kill(pid, 0) == -1 && errno == ESRCH)
	return status; // Den (lokalen) Prozess, der das Lock hatte, gibt's nicht mehr.
#endif
    }
    return QString::fromUtf8("%1 gehört einem anderen Prozess (%2)").arg(name, line);
  }

  // fd == 0: Es hat geklappt.
  QFile f(name);
  if (!f.open(fd, QFile::WriteOnly)) {
      status = QString::fromUtf8("Konnte das Lockfile nicht öffnen (%1)").arg(f.errorString());
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

void lock_release(const QString &name) {
  if (!QFile::remove(name.toUtf8())) {
    // TODO: Grund abfragen
    qWarning() << "Konnte" << name << "nicht löschen.";
  }
}

