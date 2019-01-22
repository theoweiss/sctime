#include "signalhandler.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

// please look at the article "Calling Qt Functions From Unix Signal Handlers" in the Qt documentation
// http://doc.qt.nokia.com/4.7-snapshot/unix-signals.html

// contains the two ends of one anonymous pipe.
// it is used for all instances of "SignalHandler" in order to save on file handles
static int fds[2];

static void unixSignalHandler(int sig) {
  char a = sig & 255;
  ssize_t bytes = ::write(fds[1], &a, 1);
  if (bytes<=0) {
    // we could't use the signal, we ignore that for now
  }
}

static QSocketNotifier *notifier;
static SignalHandler *singleton = 0;

SignalHandler::SignalHandler(int sig, QObject *parent):QObject(parent),signum(sig) {
  if (!singleton) {
    if (::pipe(fds))
      qFatal("Couldn't create a socketpair: %s", strerror(errno));
    notifier = new QSocketNotifier(fds[0], QSocketNotifier::Read);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(handleAll()));
    singleton = this;
  }
  connect(singleton, SIGNAL(received(int)), this, SLOT(handle(int)));

  struct sigaction sa;
  sa.sa_handler = unixSignalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(sig, &sa, 0) > 0)
    qFatal("Could not install signal handler for signal %d: %s", sig, strerror(errno));
}

void SignalHandler::handleAll() {
  notifier->setEnabled(false);
  char tmp;
  ssize_t bytes=::read(fds[0], &tmp, sizeof(tmp));
  if (bytes>0) {
    emit received((int) tmp);
    notifier->setEnabled(true);
  }
}

void SignalHandler::handle(int sig) {
  if (signum == sig) {
    emit received();
  }
}
