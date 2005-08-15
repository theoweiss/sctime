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

#include <qapplication.h>
#include <q3listview.h>
#include <qfont.h>
#include <iostream>
#include "sctimeapp.h"
#include "qstring.h"
#include <qmessagebox.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/file.h>
#include <unistd.h>
#define LOCK_FD int
#else
#include <windows.h>
#include <io.h>
#define LOCK_FD QFile*
#endif

#include "errorapp.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef IRIX
#include <fcntl.h>
#include <errno.h>
#endif

#ifdef SUN
#include <fcntl.h>
#include <errno.h>
#endif

#if defined(__GNUC__) && (__GNUC__ >=4)
#include <errno.h>
#endif

#include "globals.h"
#include "qdir.h"
#include "qfileinfo.h"

#ifndef CONFIGSUBDIR
#define CONFIGSUBDIR ".sctime"
#endif

QString execDir;
QString configDir;

#ifdef WIN32
static LOCK_FD openlock( const QString &name )
{
   int dummy=0;

   QFileInfo info( name );

   if (info.exists()) {
      ErrorApp EA("Das Lockfile "+name+" existiert bereits.\nFalls Sie sicher sind, dass keine weitere Instanz von sctime läuft, bitte löschen.",dummy,0);
      exit(2);
   }

   // open the lockfile
   LOCK_FD fd = new QFile( name );

   if (!fd->open(QIODevice::ReadWrite)) {
     ErrorApp EA("Kann Lockfile "+fd->name()+" nicht öffnen",dummy,0);
     exit(1); /* can not open */
   }

    return fd;
}
#else
static LOCK_FD openlock(const QString &name )
{

   bool lockFileExistiert;

   QFileInfo info( name );

   lockFileExistiert=info.exists();
    // open the lockfile
    LOCK_FD fd = open( QFile::encodeName( name ),
                   O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) {
      std::cerr<<("Kann Lockfile "+name+" nicht öffnen").toStdString()<<std::endl;
      exit(1); /* can not open */
    }

    if (lockf(fd,F_TLOCK,0)<0)
    {
      switch (errno) {
#ifdef HPUX
        case EACCES:
           break;
#endif
        case EAGAIN:
          {
            std::cerr<<("Das Lockfile "+name+" wird bereits benutzt.\nFalls Sie sicher sind, dass keine weitere Instanz von sctime läuft, bitte löschen.").toStdString()<<std::endl;
            exit(3);
            break;
          }
        case ENOLCK:   // Weniger komfortables Locking auf Systemen, die es nicht unterstuetzen.
          {
            if (lockFileExistiert) {
              std::cerr<<("Das Lockfile "+name+" existiert bereits.\nFalls Sie sicher sind, dass keine weitere Instanz von sctime läuft, bitte löschen.").toStdString()<<std::endl;
              exit(2);
            }
            break;
          }
        default: {
             std::cout<<"Error "<<errno<<": "<<strerror(errno)<<(": "+name).toStdString()<<std::endl;
             exit(5);
          }
      }

    }

    char str[50];

    sprintf(str,"%d\n",getpid());
    write(fd,str,strlen(str)); /* record pid to lockfile */

    return fd;
}
#endif

/*
  Closes the lock file specified by fd.  fd is the file descriptor
  returned by the openlock() function.
*/
static void closelock(LOCK_FD fd , const QString &name)
{
#ifndef WIN32
  lockf(fd,F_ULOCK,0);
  close(fd);
#else
  fd->close();
#endif

  unlink(name);
}

SCTimeApp* sctimeApp;

/** main: hier wird ueberprueft, ob die Applikation ueberhaupt starten soll
 * (Lockfiles,...), und falls ja, wird SCTimeApp initialisiert und
 ausgefuehrt */
int main( int argc, char **argv )
{

  QDir directory;
  QFileInfo executable(argv[0]);

  if (executable.isSymLink()) //Wir wollen den echten Pfad, um unsere Icons zu finden.
    executable.setFile(executable.readLink());

  execDir=executable.dirPath(true);
  // Pruefen, ob das Configdir ueber das environment gesetzt wurde
  QString configdirstring;
  char *envpointer;
  envpointer = getenv("SCTIME_CONFIG_DIR");
  if (envpointer)
      configdirstring = envpointer;
  else {
      configdirstring = CONFIGSUBDIR; // default Configdir
#ifndef WIN32
      directory.cd(directory.homeDirPath());
#else
      if (!directory.cd("H:\\")) {
         ErrorApp ea("Kann nicht auf H:\\ zugreifen.",argc, argv );
         return false;
      }
#endif
  }

  // neues directory anlegen, hineinwechseln, und merken.
  directory.mkdir(configdirstring);
  if (!directory.cd(configdirstring))
  {
      ErrorApp ea("Kann in Verzeichnis "+configdirstring+" nicht wechseln!",argc, argv );
      return false;
  }
  configDir=directory.path();

#ifdef WIN32
  Qt::HANDLE hEvent = CreateEventA(NULL, FALSE, TRUE, "sctimeGuardEvent");
  if ( GetLastError () == ERROR_ALREADY_EXISTS )
  {
    ErrorApp ea("Eine andere Instanz von sctime läuft bereits auf diesem Rechner.",argc, argv );
    return false;
  }
#endif

  LOCK_FD lfp=openlock(configDir+"/LOCK");

  sctimeApp= new SCTimeApp( argc, argv );

  sctimeApp->exec();

  closelock(lfp, configDir+"/LOCK");

  delete sctimeApp;
  return 0;
}
