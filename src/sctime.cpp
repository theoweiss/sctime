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

#include <QTextCodec>
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
#include <QSocketNotifier>
#include <QSqlDatabase>

#ifndef WIN32
#include <assert.h>
#include <locale.h>
#include <signal.h>
#include "signalhandler.h"
#define LOCALE "de_DE.UTF-8"
#endif

#include "GetOpt.h"
#include "lock.h"
#include "timemainwindow.h"
#include "sctimexmlsettings.h"
#include "globals.h"
#include "kontotreeview.h"
#include "datasource.h"
#include "setupdsm.h"


#ifndef CONFIGSUBDIR 
#define CONFIGSUBDIR ".sctime"
#endif

#ifdef __GNUC__
static void fatal(const QString& title, const QString& body) __attribute__ ((noreturn));
#else
static void fatal(const QString& title, const QString& body);
#endif

QString configDir;
QString lockfilePath;
const QString version("0.72");
QString PERSOENLICHE_KONTEN_STRING;
QString ALLE_KONTEN_STRING;
Lock *lock;


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
static QString canonicalPath(const QString& path) { return QFileInfo(path).canonicalFilePath(); }

#else
static void setlocale() {
  if (!setlocale(LC_ALL, ""))
    fatal("sctime: Konfigurationsproblem", "Die 'locale'-Einstellungen sind nicht zulaessig (siehe 'locale -a')");
  const char *encoding = setlocale(LC_CTYPE, NULL); //query
  if (!encoding && !encoding[0])  fatal("sctime: Konfigurationsfehler", "Konnte die locale-Bibliothek nicht initialisieren.");
  if (setenv("LC_ALL", LOCALE, 1)) fatal("sctime: Konfigurationsfehler", "Konnte Umgebungsvariable ZEIT_ENCODING nicht setzen.");
}

static QString canonicalPath(QString path) {
    return path.startsWith("~/") ? path.replace(0,2,QDir::homePath()+"/") : QFileInfo(path).canonicalFilePath(); }
#endif

class SctimeApp: public QApplication {
public:
  SctimeApp(int &argc, char **argv):QApplication(argc, argv) { }

  void commitData ( QSessionManager & manager ) {QApplication::commitData(manager); }
};


/** main: hier wird ueberprueft, ob die Applikation ueberhaupt starten soll
 * (Lockfiles,...), und falls ja, wird SCTimeApp initialisiert und
 ausgefuehrt */
int main( int argc, char **argv ) {
  SctimeApp app(argc, argv);
  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
          QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);
  QTextCodec::setCodecForTr(QTextCodec::codecForName ("UTF-8"));
  app.setObjectName("Sctime");

  PERSOENLICHE_KONTEN_STRING = QObject::tr("Persönliche Konten");
  ALLE_KONTEN_STRING = QObject::tr("Alle Konten");

  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1],"--help") == 0 || strcmp(argv[1], "-h") == 0||strcmp(argv[1],"--help") == 0)) {
    QMessageBox::information(NULL, "sctime", help);
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
  // Ich lege eine lokale Sperre an, die vom Betriebssystem zuverlässig auch
  // nach einem Absturz aufgegeben wird. Außerdem lege ich noch ein globales
  // Lock mit dem Rechnernamen an.
  // Gründe:
  // - Es gibt keine globalen Sperren (für SMB und NFS hinweg sichtbar), die
  //   nach einem Absturz automatisch entfernt werden.
  // - Das Programm kann ausgeben, auf welchem Rechner sctime noch läuft.
  // - Nach einem Absturz kann ich auf dem gleichen Rechner neu starten,
  //   ohne de Benutzer mit Warnungen wegen alter Sperren zu belästigen.
  LockLocal local("sctime", true);
  Lock *global = new Lockfile(configDir + "/LOCK", true);
  local.setNext(global);
  if (!local.acquire()) fatal(QObject::tr("sctime: kann nicht starten"), local.errorString());
  lock = &local;

  SCTimeXMLSettings settings;
  settings.readSettings();
  QList<QString> dataSourceNames(settings.backends.split(" "));
  setupDatasources(dataSourceNames, settings, zeitkontenfile, bereitschaftsfile);
  TimeMainWindow mainWindow;
#ifndef WIN32
  SignalHandler term(SIGTERM);
  app.connect(&term, SIGNAL(received()), &app, SLOT(closeAllWindows()));
  SignalHandler hup(SIGHUP);
  app.connect(&hup, SIGNAL(received()), &app, SLOT(closeAllWindows()));
  SignalHandler int_(SIGINT);
  app.connect(&int_, SIGNAL(received()), &app, SLOT(closeAllWindows()));
#endif  
  mainWindow.show();
  app.processEvents();
  app.exec();
  mainWindow.save();
  local.release();
  delete global;
  return 0;
}
