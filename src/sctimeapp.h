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
#else
#include "kontodateninfozeit.h"
#include "signal.h"
#endif
#include <iostream>
#include "timemainwindow.h"
#include "sctimexmlsettings.h"

/**
 *  In dieser Klasse wird hauptsaechlich ein TimeMainWindow erzeugt und mit Daten versorgt.
 */

class SCTimeApp: public QApplication
{
  TimeMainWindow* mainWindow;
  private:
    #ifdef WIN32
    KontoDatenInfoDatabase zk;
    #else
    KontoDatenInfoZeit zk;
    #endif


  public:

    SCTimeApp( int &argc, char **argv ): QApplication (argc,argv)
    {
      mainWindow=new TimeMainWindow(&zk);
      setMainWidget( mainWindow );
      mainWindow->show();
    }

    virtual ~SCTimeApp()
    {
      delete mainWindow;
    }
};

