#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <signal.h>
#include <QObject>
#include <QSocketNotifier>

class SignalHandler : public QObject {
    Q_OBJECT
public:
  explicit SignalHandler(int sig, QObject *parent = 0);
  const int signum;

signals:
  void received(int);
  void received();

public slots:
  void handleAll();
  void handle(int sig);
};

#endif // SIGNALHANDLER_H
