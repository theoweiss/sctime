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
#include <QFont>
#include <iostream>
#include "sctimeapp.h"
#include <QString>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <Windows.h>
#include <Winbase.h>

#else
#include "assert.h"
#include <locale.h>
#endif

#include "errorapp.h"
#include <stdio.h>
#include <stdlib.h>

#include <QDir>
#include <QFileInfo>
#include "GetOpt.h"
#include "lock.h"

#ifndef CONFIGSUBDIR
#define CONFIGSUBDIR ".sctime"
#endif

QString execDir;
QString configDir;
QString lockfilePath;


SCTimeApp* sctimeApp;

/** main: hier wird ueberprueft, ob die Applikation ueberhaupt starten soll
 * (Lockfiles,...), und falls ja, wird SCTimeApp initialisiert und
 ausgefuehrt */
int main( int argc, char **argv ) {
  QDir directory;
  QFileInfo executable(argv[0]);

  if (executable.isSymLink()) //Wir wollen den echten Pfad, um unsere Icons zu finden.
    executable.setFile(executable.readLink());

#ifndef WIN32
  if( argc == 2 && (strcmp(argv[1], "-h")==0||strcmp(argv[1],"--help")==0))
  {
    std::cout << "Usage: sctime [OPTION] " << std::endl << std::endl <<
    " Available Options: " << std::endl <<
    "   --configdir= \t Location of the directory where your files will be placed." << std::endl <<
    "                \t Default is /home/<USER>/.sctime" << std::endl <<
    "   --zeitkontenfile= \t Location of zeitkontenfile, not necessary if you want to connect to the Database." << std:: endl <<
    "   --bereitschaftsfile=  Location of bereitschaftsfile, not necessary if you want to connect to the Database." << std::endl;
    exit(0);
  }

#else
  if( argc == 2 && (strcmp(argv[1], "/h")==0 || strcmp(argv[1],"/?")==0))
  {
    char * text = "Usage: sctime [OPTION] \n\n" ;
    text = strcat(text, " Available Options: \n");
    text = strcat(text, " --configdir=  Location of the directory where your files will be placed.\n");
    text = strcat(text, "               Default is /home/<USER>/.sctime \n");
    text = strcat(text, " --zeitkontenfile=  Location of zeitkontenfile, not necessary if you want to connect to the Database.\n");
    text = strcat(text, " --bereitschaftsfile=  Location of bereitschaftsfile, not necessary if you want to connect to the Database.");
    ErrorApp ea(text, argc, argv, true);
    exit(0);
  }
#endif

#ifndef WIN32
  if (!setlocale(LC_ALL, "")) {
    ErrorApp("FEHLER! Die 'locale'-Einstellungen sind nicht zulaessig (siehe 'locale -a')", argc, argv);
    exit(1);
  }
  char *ctype = setlocale(LC_CTYPE, NULL); //query
  assert (ctype);
  if (strcmp(ctype, "POSIX") == 0 && strcmp(ctype, "C"))
    if (setenv("LC_CTYPE", "UTF-8", 1)) {
      ErrorApp("Konnte LC_CTYPE nicht setzen.", argc, argv);
      exit(1);
    }
#endif
  //execDir=executable.dirPath(true);
  execDir=executable.absolutePath();
  // Pruefen, ob das Configdir ueber das environment gesetzt wurde
  QString configdirstring;
  QString zeitkontenfile;
  QString bereitschaftsfile;
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

    if (envpointer)
        configdirstring = envpointer;
    else {
        configdirstring = CONFIGSUBDIR; // default Configdir
    }
  }

  if (!directory.cd(configdirstring))
  {
#ifndef WIN32
    directory.cd(directory.homePath());
#else
    if (!directory.cd("H:\\")) {
           ErrorApp ea("Kann nicht auf H:\\ zugreifen.",argc, argv );
           return 1;
    }
#endif
    // neues directory anlegen, hineinwechseln, und merken.
    directory.mkdir(configdirstring);

    if (!directory.cd(configdirstring))
    {
      ErrorApp ea("Kann in Verzeichnis "+configdirstring+" nicht wechseln!",argc, argv );
      return 1;
    }
  }
  configDir=directory.path();

#ifdef WIN32
  Qt::HANDLE hEvent = CreateEventA(NULL, FALSE, TRUE, "sctimeGuardEvent");
  if ( GetLastError () == ERROR_ALREADY_EXISTS ) {
    ErrorApp ea("Eine andere Instanz von sctime läuft bereits auf diesem Rechner.", argc, argv );
    return 1;
  }
  const bool local_excluded = true;
#else
  const bool local_excluded = false;
#endif
  lockfilePath = configDir+"/LOCK";
  QString err = lock_acquire(lockfilePath, local_excluded);
  if (!err.isNull()) {
    ErrorApp ea(err, argc, argv);
    return 1;
  }
  sctimeApp= new SCTimeApp( argc, argv , zeitkontenfile, bereitschaftsfile);
  sctimeApp->exec();
  lock_release(lockfilePath);
  delete sctimeApp;
  return 0;
}
