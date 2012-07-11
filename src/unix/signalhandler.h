#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <signal.h>
#include <QObject>
#include <QSocketNotifier>

// map unix signals to Qt signals
class SignalHandler : public QObject {
    Q_OBJECT
public:
  explicit SignalHandler(int sig, QObject *parent = 0);
  const int signum;

signals:
  void received();
  void received(int);  // internal

private slots:
  void handleAll();
  void handle(int sig);
};

#endif // SIGNALHANDLER_H
