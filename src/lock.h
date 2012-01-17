#ifndef LOCK_H
#define LOCK_H
#include <QString>
// reliable_local_exclusion:
// Ich gehe davon aus, dass es einen zuverlaessigen Mechanismus gibt,
// der lokal mehrere Instanzen verhindert.
// Vorgabe: false
QString lock_acquire(const QString &name, bool reliable_local_exclusion = false);
void lock_release(const QString &name);
 #endif
