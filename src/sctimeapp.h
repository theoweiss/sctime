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
#ifdef WIN32
#include "kontodateninfodatabase.h"
#include "bereitschaftsdateninfodatabase.h"
#else
#include "signal.h"
#endif
#include "kontodateninfozeit.h"
#include "bereitschaftsdateninfozeit.h"
#include <iostream>
#include "timemainwindow.h"
#include "sctimexmlsettings.h"
#include "GetOpt.h"
#include <QFileInfo>
#include <QDir>

/**
 *  In dieser Klasse wird hauptsaechlich ein TimeMainWindow erzeugt und mit Daten versorgt.
 */

class SCTimeApp: public QApplication
{
  Q_OBJECT

  TimeMainWindow* mainWindow;
  private:
    KontoDatenInfo* zk;
    BereitschaftsDatenInfo* bereitschaftsdatenReader;

  public:

    SCTimeApp( int &argc, char **argv, QString zeitkontenfile, QString bereitschaftsfile ): QApplication (argc,argv)
    {

#ifndef WIN32
      SCTimeXMLSettings settings;
      settings.readSettings();
      connect(this, SIGNAL(unixSignal(int)), this, SLOT(sighandler(int)));
      watchUnixSignal(SIGINT,true);
      watchUnixSignal(SIGTERM,true);
      watchUnixSignal(SIGHUP,true);
      if (zeitkontenfile.isEmpty()) {
          zk = new KontoDatenInfoZeit();
          if (!settings.zeitKontenKommando().isEmpty())
            static_cast<KontoDatenInfoZeit*>(zk)->setKommando(settings.zeitKontenKommando());
      } else
      {
          zk = new KontoDatenInfoZeit(canonicalPath(zeitkontenfile));
      }
      if (bereitschaftsfile.isEmpty()) {
         bereitschaftsdatenReader=new BereitschaftsDatenInfoZeit();
      } else
      {
        bereitschaftsdatenReader=new BereitschaftsDatenInfoZeit(canonicalPath(bereitschaftsfile));
      }
#else
      if (zeitkontenfile.isEmpty())
          zk = new KontoDatenInfoDatabase();
      else
          zk = new KontoDatenInfoZeit(canonicalPath(zeitkontenfile));
      if (zeitkontenfile.isEmpty())
          bereitschaftsdatenReader=new BereitschaftsDatenInfoDatabase();
      else
        bereitschaftsdatenReader=new BereitschaftsDatenInfoZeit(canonicalPath(bereitschaftsfile));
#endif
      mainWindow=new TimeMainWindow(zk,bereitschaftsdatenReader);
      setMainWidget(mainWindow);
      mainWindow->show();
    }

    virtual ~SCTimeApp()
    {
      delete mainWindow;
      delete zk;
    }

  void commitData ( QSessionManager & manager ) 
  {
  	mainWindow->save();
  	QApplication::commitData(manager);
  }

  public slots:
#ifndef WIN32
    void sighandler(int nr)
    {
        switch(nr) {
            case SIGINT:
            case SIGTERM:
                mainWindow->save();
                exit(0);
                break;
            case SIGHUP:
                break;
        }
    }
#endif
  private:
    QString canonicalPath(QString path)
    {
#ifndef WIN32
      if (path.startsWith("~/"))
         path.replace(0,2,QDir::homePath()+"/");
#endif
      return QFileInfo(path).canonicalFilePath();
    }
};

