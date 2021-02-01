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

#ifndef LOCK_H
#define LOCK_H
#include <QString>

enum LockState {
  LS_OK,
  LS_CONFLICT,
  LS_UNKNOWN
};


class Lock {
public:
  virtual ~Lock();
  inline void setNext(Lock *val) { this->next = val; }
  inline QString errorString() const { return errStr; }
  bool acquire();
  bool release();
  LockState check();
protected:
  Lock();
  QString errStr;
  Lock* next;
  bool acquired;
private:
  virtual bool _acquire() = 0;
  virtual bool _release() = 0;
  virtual LockState _check() { return LS_OK; }
};

class LockLocal : public Lock {
public:
  LockLocal(const QString& name, const bool user);
  virtual bool _acquire();
  virtual bool _release();
  const bool user;
  const QString name;
  const QString path;
private:
#ifdef WIN32
    Qt::HANDLE handle;
#else
    int fd;
#endif
};
#endif

class Lockfile: public Lock {
public:
  Lockfile(const QString& path, const bool localExclusionProvided);
  virtual bool _acquire();
  virtual bool _release();
  virtual LockState _check();
  const QString path;
  const bool localExclusionProvided;
private:
  bool _update(const int);
  QString hostname;
};
