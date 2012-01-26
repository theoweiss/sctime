#ifndef LOCK_H
#define LOCK_H
#include <QString>


class Lock {
public:
  inline void setNext(Lock *val) { this->next = val; }
  inline QString errorString() const { return errStr; }
  bool acquire();
  bool release();
protected:
  Lock();
  QString errStr;
  Lock* next;
  bool acquired;
private:
  virtual bool _acquire() = 0;
  virtual bool _release() = 0;
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
  const QString path;
  const bool localExclusionProvided;
};
