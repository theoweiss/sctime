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
#include <qmessagebox.h>
#include <iostream>
#include "timemainwindow.h"
#include "qstring.h"


/**
 * Mini-Application, die nur eine Fehlermeldung ausgibt. Sie ist dazu da, um Fehler,
 * die noch vor dem Start der Hauptapplikation auftreten, auszugeben.
 */

class ErrorApp: public QApplication
{
  private:

  public:

    ErrorApp( const QString& message, int &argc, char **argv ): QApplication (argc,argv)
    {
      QMessageBox::critical(NULL,"Error",message, QMessageBox::Ok, QMessageBox::NoButton,
                             QMessageBox::NoButton);
    }
};

