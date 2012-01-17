#ifndef LOCK_H
#define LOCK_H
#include <QString>

QString lock_acquire(const QString &path, bool local_exclusion_already_provided = false);
QString lock_release(const QString &name);

/*implementiert Lockfiles;
    sie enthalten Rechername und PID;
    UNIX: lokale Sperren werden mit "kill -0" geprueft;
    WIN32: benutzt zusaetzlich lokal WIN32-Sperren */
class Lock {
public:
    Lock(const QString &path, const QString& name); // name: fuer WIN32-Sperren
    QString acquire(); // ok: isNull; sonst: Fehlermeldung 
    QString release(); // ok: isNull: sonst: Fehlermeldung (mit undef. Zustand)
    ~Lock();
    const QString path;
    const QString name;
private:
    bool acquired;
#ifdef WIN32
    Qt::HANDLE local_lock;
#endif
};


#endif
