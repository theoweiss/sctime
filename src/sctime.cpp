/*
    $Id$

    Copyright (C) 2003 Florian Schmitt, Science and Computing AG
                       f.schmitt@science-computing.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QPixmap>
#include <QSplashScreen>
#include <QString>
#include <QTranslator>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <Windows.h>
#include <Winbase.h>
#include "kontodateninfodatabase.h"
#include "bereitschaftsdateninfodatabase.h"
#include "DBConnector.h"

#else
#include <assert.h>
#include <locale.h>
#include <signal.h>
#endif

#include "GetOpt.h"
#include "lock.h"
#include "kontodateninfozeit.h"
#include "bereitschaftsdateninfozeit.h"
#include "timemainwindow.h"
#include "sctimexmlsettings.h"

#ifndef CONFIGSUBDIR 
#define CONFIGSUBDIR ".sctime"
#endif

QString configDir;
QString lockfilePath;

static void fatal(const QString& title, const QString& body) __attribute__ ((noreturn));

static void fatal(const QString& title, const QString& body) {
  QMessageBox::critical(NULL, title, body, QMessageBox::Ok);
  exit(1);
}

static const char help[] =
      " Available Options: \n"
      " --configdir=  location of the directory where your files will be placed (default: ~/.sctime)\n"
      " --zeitkontenfile=  location of zeitkontenfile (default: output of 'zeitkonten --mikrokonten --sep=\\|')\n"
      " --bereitschaftsfile=  location of bereitschaftsfile (default: output of 'zeitbereitls'.\n\n"
     "Without these options, sctime reads the necessary data from the database ('zeitdabaserv')";

#ifdef WIN32
static void setlocale() {}

static bool local_exclusion(const char* name) { // true: die Funktion garantiert, dass lokal keine weitere Instanz läuft
  Qt::HANDLE hEvent = CreateEventA(NULL, FALSE, TRUE, name);
  return  GetLastError () != ERROR_ALREADY_EXISTS);
}

static QString canonicalPath(const QString& path) { return QFileInfo(path).canonicalFilePath()); }

#else
static void setlocale() {
  if (!setlocale(LC_ALL, ""))
    fatal("sctime: Konfigurationsproblem", "Die 'locale'-Einstellungen sind nicht zulaessig (siehe 'locale -a')");
  const char *encoding = setlocale(LC_CTYPE, NULL); //query
  if (!encoding && !encoding[0])  fatal("sctime: Konfigurationsfehler", "Konnte die locale-Bibliothek nicht initialisieren.");
  if (strcmp(encoding, "POSIX") == 0 && strcmp(encoding, "C"))
    if (setenv("LC_CTYPE", "UTF-8", 1)) fatal("sctime: Konfigurationsfehler", "Konnte LC_CTYPE nicht setzen.");
}

static QString canonicalPath(QString path) {
    return path.startsWith("~/") ? path.replace(0,2,QDir::homePath()+"/") : QFileInfo(path).canonicalFilePath(); }

static bool local_exclusion(const QString& path) { return false; }
#endif

class SctimeApp: public QApplication {
public:
  SctimeApp(int &argc, char **argv):QApplication(argc, argv) {}
  void commitData ( QSessionManager & manager ) {
    QApplication::commitData(manager);
  }
};

/** main: hier wird ueberprueft, ob die Applikation ueberhaupt starten soll
 * (Lockfiles,...), und falls ja, wird SCTimeApp initialisiert und
 ausgefuehrt */
int main( int argc, char **argv ) {
  SctimeApp app(argc, argv);
  app.setObjectName("Sctime");
  
  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1],"--help") == 0 || strcmp(argv[1], "-h") == 0||strcmp(argv[1],"--help") == 0)) {
    QMessageBox::information(NULL,"sctime", help, QMessageBox::Ok);
    exit(0);
  }
  QString configdirstring, zeitkontenfile, bereitschaftsfile;
  char *envpointer;

  GetOpt opts(argc, argv);
  opts.addOption('f',"configdir", &configdirstring);
  opts.addOption('f',"zeitkontenfile", &zeitkontenfile);
  opts.addOption('f',"bereitschaftsfile", &bereitschaftsfile);
  opts.parse();

  if (configdirstring.startsWith("~/"))
      configdirstring.replace(0,2,QDir::homePath()+"/");

  if (configdirstring.isEmpty()) {
    envpointer = getenv("SCTIME_CONFIG_DIR");
    configdirstring = envpointer ? envpointer : CONFIGSUBDIR; // default Configdir
  }
  setlocale();

  QSplashScreen splash(QPixmap(":/splash.png"));
  splash.showMessage("Konfigurationsdateien suchen");
  splash.show(); // später: app.processEvents();

  QDir directory;
  if (!directory.cd(configdirstring)) {
#ifdef WIN32
    QString home("h:\\");
#else
    QString home(directory.homePath());
#endif
    if (!directory.cd(home)) fatal("sctime: Konfigurationsproblem", QString("Kann nicht auf %1 zugreifen.").arg(home));
    directory.mkdir(configdirstring);
    if (!directory.cd(configdirstring)) fatal("sctime: Konfigurationsproblem", QString("Kann nicht auf %1 zugreifen.").arg(configdirstring));
  }
  configDir=directory.path();

  splash.showMessage("Sperrdatei anlegen");
  app.processEvents();

  lockfilePath = configDir + "/LOCK";
  QString err = lock_acquire(lockfilePath, local_exclusion("sctime"));
  if (!err.isNull()) fatal("sctime: Sperrdatei", err);

  splash.showMessage("Kontenliste einlesen");
  app.processEvents();

  KontoDatenInfo* zk;
  BereitschaftsDatenInfo* bereitschaftsdatenReader;

#ifdef WIN32
    DBConnector dbconnector;
    zk = zeitkontenfile.isEmpty()
        ? new KontoDatenInfoDatabase(dbconnector)
        zk = new KontoDatenInfoZeit(canonicalPath(zeitkontenfile));
    bereitschaftsdatenREader = bereitschaftsfile.isEmpty()
      ? BereitschaftsDatenInfoDatabase(dbconnector)
      : bereitschaftsdatenReader=new BereitschaftsDatenInfoZeit(canonicalPath(bereitschaftsfile));
#else
  SCTimeXMLSettings settings;
  settings.readSettings();
  if (zeitkontenfile.isEmpty()) {
    zk = new KontoDatenInfoZeit();
    if (!settings.zeitKontenKommando().isEmpty())
      static_cast<KontoDatenInfoZeit*>(zk)->setKommando(settings.zeitKontenKommando());
  } else {
    zk = new KontoDatenInfoZeit(canonicalPath(zeitkontenfile));
  }
  bereitschaftsdatenReader = bereitschaftsfile.isEmpty()
      ? new BereitschaftsDatenInfoZeit()
      :  new BereitschaftsDatenInfoZeit(canonicalPath(bereitschaftsfile));
  app.watchUnixSignal(SIGINT, true);
  app.watchUnixSignal(SIGTERM, true);
  app.watchUnixSignal(SIGHUP, true);
#endif
  TimeMainWindow mainWindow(zk, bereitschaftsdatenReader);
  splash.finish(&mainWindow);
  mainWindow.show();
  app.exec();
  mainWindow.save();
  return 0;
}
