/*
    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt et al.

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

#include <QTextCodec>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QString>
#include <QTranslator>
#include <QSqlDatabase>
#include <QCommandLineParser>
#include <QCommandLineOption>

#ifndef WIN32
#include <assert.h>
#include <locale.h>
#endif

#include "lock.h"
#include "timemainwindow.h"
#include "globals.h"
#include "kontotreeview.h"
#include "datasource.h"
#include "sctimeapp.h"

#ifndef CONFIGDIR
#define CONFIGDIR "~/.sctime"
#endif

#ifdef __GNUC__
static void fatal(const QString& title, const QString& body) __attribute__ ((noreturn));
#else
static void fatal(const QString& title, const QString& body);
#endif

QDir configDir;
QString lockfilePath;
QString PERSOENLICHE_KONTEN_STRING;
QString ALLE_KONTEN_STRING;

static void fatal(const QString& title, const QString& body) {
  QMessageBox::critical(NULL, title, body, QMessageBox::Ok);
  exit(1);
}

static const QString help(QObject::tr(
"Available Options:\n"
"--configdir=DIR		location of the directory where your files will be placed\n"
"			(default: ~/.sctime)\n\n"
"--datasource=		(repeatable) use these data sources\n"
"			(default: 'QPSQL'',' 'QODBC'', 'command' and 'file'');\n"
"			overrides <backends/> in settings.xml\n\n"
"--zeitkontenfile=FILE	read the accounts list from file FILE\n"
"			(default: output of 'zeitkonten'). Obsolete.\n\n"
"--bereitschaftsfile=FILE	read the 'Bereitschaftsarten'' from file FILE\n"
"			(default: output of 'zeitbereitls'). Obsolete.\n\n"
"--specialremunfile=FILE       read the types of special remunerations from file FILE\n"
"			(default: output of 'sonderzeitls'. Obsolete.\n\n"
"--offlinefile=FILE		read all needed data from FILE which must be of json format\n"
"			overides --zeitkontenfile --bereitschaftsfile and --specialremunfile\n\n"
"Please see the Help menu for further information (F1)!"));

QString absolutePath(QString path) {
    if (path == "~" || path.startsWith("~/") || path.startsWith(QString("~") +
		QDir::separator())) {
#ifdef WIN32
      
      QString homedir = getenv("SCTIME_HOMEDRIVE");
	    if (homedir.isNull()) {
          homedir = getenv("HOMEDRIVE");
      }
	    // append a separator if only the homedir is requested so that the
	    // drive's root is addressed reliably
	    if (path == "~")
		    homedir.append(QDir::separator());
#else
	    QString homedir = QDir::homePath();
#endif /* WIN32 */
	    return path.replace(0, 1, homedir);
    }
    return QFileInfo(path).absoluteFilePath();
}

/** main: hier wird ueberprueft, ob die Applikation ueberhaupt starten soll
 * (Lockfiles,...), und falls ja, wird SCTimeApp initialisiert und
 ausgefuehrt */
int main(int argc, char **argv ) {
  SctimeApp* app = new SctimeApp(argc, argv);  // Qt initialization

  // load translations
  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
          QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app->installTranslator(&qtTranslator);
  QTranslator sctimeTranslator;
  sctimeTranslator.load(":/translations/sctime");
  app->installTranslator(&sctimeTranslator);
#if QT_VERSION < 0x050000
  /* no longer necessary with Qt >= 5.0 */
  QTextCodec::setCodecForTr(QTextCodec::codecForName ("UTF-8"));
#endif

  PERSOENLICHE_KONTEN_STRING = QObject::tr("Personal accounts");
  ALLE_KONTEN_STRING = QObject::tr("All accounts");

  // FIXME: use QCommandLineParser for display of help too
  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1],"--help") == 0 || strcmp(argv[1], "/h") == 0||strcmp(argv[1],"/help") == 0)) {
      QMessageBox mb(
          QMessageBox::Information,
          QObject::tr("sctime ") + qApp->applicationVersion(),
          "<pre>"+help.toHtmlEscaped()+"</pre>",
          QMessageBox::Ok,
          NULL);
      mb.exec();
      exit(0);
  }
  
  QCommandLineParser parser;

  QCommandLineOption configdiropt("configdir","",QObject::tr("directory"));
  parser.addOption(configdiropt);
  QCommandLineOption zeitkontenfileopt("zeitkontenfile","",QObject::tr("file"));
  parser.addOption(zeitkontenfileopt);
  QCommandLineOption bereitschaftsfileopt("bereitschaftsfile","",QObject::tr("file"));
  parser.addOption(bereitschaftsfileopt);
  QCommandLineOption specialremunfileopt("specialremunfile","",QObject::tr("file"));
  parser.addOption(specialremunfileopt);
  QCommandLineOption offlinefileopt("offlinefile","",QObject::tr("file"));
  parser.addOption(offlinefileopt);
  QCommandLineOption datasourceopt("datasource","", QObject::tr("source"));
  parser.addOption(datasourceopt);
  QCommandLineOption logfileopt("logfile","",QObject::tr("file"));
  parser.addOption(logfileopt);
  parser.process(*app);
  
  QString configdirstring=parser.value(configdiropt);
  QString zeitkontenfile=parser.value(zeitkontenfileopt);
  QString bereitschaftsfile=parser.value(bereitschaftsfileopt);
  QString specialremunfile=parser.value(specialremunfileopt);
  QString offlinefile=parser.value(offlinefileopt);
  QString logfile=parser.value(logfileopt);
  QStringList dataSourceNames=parser.values(datasourceopt);

  if (configdirstring.isEmpty()) {
    char *envpointer = getenv("SCTIME_CONFIG_DIR");
    configdirstring = envpointer ? envpointer : CONFIGDIR; // default Configdir
  }

  // configdirstring can no longer be empty now but still may be relative or
  // reference home dir by ~
  configdirstring = absolutePath(configdirstring);
  QDir directory;
  if (!directory.cd(configdirstring)) {
    directory.mkdir(configdirstring);
    if (!directory.cd(configdirstring))
      fatal(QObject::tr("sctime: Configuration problem"),
        QObject::tr("Cannot access configration directory %1.").arg(configdirstring));
  }
  configDir.setPath(directory.path());

  if (!zeitkontenfile.isEmpty())
      zeitkontenfile=absolutePath(zeitkontenfile);
  if (!bereitschaftsfile.isEmpty())
      bereitschaftsfile=absolutePath(bereitschaftsfile);
  if (!specialremunfile.isEmpty())
      specialremunfile=absolutePath(specialremunfile);
  if (!offlinefile.isEmpty())
      offlinefile=absolutePath(offlinefile);
  if (!logfile.isEmpty())
      logfile=absolutePath(logfile);

      
  // Locking: nur eine Instanz von sctime soll laufen

  // Ich lege eine lokale Sperre an, die vom Betriebssystem zuverlässig auch
  // nach einem Absturz aufgegeben wird. Außerdem lege ich noch ein globales
  // Lock mit dem Rechnernamen an.
  // Gründe:
  // - Es gibt keine globalen Sperren (für SMB und NFS hinweg sichtbar), die
  //   nach einem Absturz automatisch entfernt werden.
  // - Das Programm kann ausgeben, auf welchem Rechner sctime noch läuft.
  // - Nach einem Absturz kann ich zumindest auf dem gleichen Rechner neu starten,
  //   ohne den Benutzer mit Warnungen wegen alter Sperren zu belästigen.
  LockLocal local("sctime", true);
  bool oldgloballockexists=configDir.exists("LOCK");
  Lockfile *global = new Lockfile(configDir.filePath("LOCK"), true);
  local.setNext(global);
  if (!local.acquire()) fatal(QObject::tr("sctime: Cannot start"), local.errorString());
  if (oldgloballockexists) {
      QFileInfo info(configDir,"settings.xml");
      if (info.exists()) {
        QDateTime lasttime=info.lastModified();
        QMessageBox::critical(NULL, QObject::tr("Unclean state"), QObject::tr("It looks like the last instance of sctime might have crashed, probably at %1. Please check if the recorded times of that date are correct.").arg(lasttime.toLocalTime().toString()), QMessageBox::Ok);
      }
  }
  app->init(&local, dataSourceNames, zeitkontenfile, bereitschaftsfile, specialremunfile, offlinefile, logfile);
  app->exec();
  
  // warning: dont rely on anything being executed beyond that point
  delete app;
  delete global;
  return 0;
}
