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

#include "kontodateninfodatabase.h"
#include "unterkontoeintrag.h"
#include <iostream>
#include "qsqldatabase.h"
#include "qmessagebox.h"
#include "qapplication.h"
#include "globals.h"

/**
 * Liest aus einer ODBC-Datenbank nach abtList
 */
bool KontoDatenInfoDatabase::readInto(AbteilungsListe * abtList)
{
  bool ret = true;
  // PluginDir setzen
  QApplication::addLibraryPath(execDir+"/lib");
  abtList->clear();
  QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( "QODBC3" );
  if ( defaultDB ) {
    defaultDB->setDatabaseName( "zeit" );

    if ( defaultDB->open() ) {
      QSqlQuery query( "select gb.name,konto.name,unterkonto.name,unterkonto.beschreibung from konto,unterkonto,gb WHERE konto.konto_id = unterkonto.konto_id AND gb.gb_id = konto.gb_id AND unterkonto.eintragbar=\'y\';", defaultDB);
      if ( query.isActive() ) {
        while ( query.next() ) {
          QString abt = query.value(0).toString().simplifyWhiteSpace();
          QString ko = query.value(1).toString().simplifyWhiteSpace();
          QString uko = query.value(2).toString().simplifyWhiteSpace();
          QString beschreibung = query.value(3).toString();
          abtList->insertEintrag(abt,ko,uko);
          if (beschreibung.isEmpty())
              beschreibung="";
          abtList->setBeschreibung(abt,ko,uko,beschreibung);
          abtList->setUnterKontoFlags(abt,ko,uko,IS_IN_DATABASE,FLAG_MODE_OR);
        }
      }  else std::cout<<"Kann Abfrage nicht durchfuehren"<<std::endl;
    } else {
        ret = false;
#ifdef WIN32
		QMessageBox::critical(NULL,"Error","Kann Datenbank nicht öffnen\n",
                              QMessageBox::Ok, QMessageBox::NoButton,
                              QMessageBox::NoButton);
#else
		std::cout<<"Kann Datenbank nicht öffnen"<<std::endl;
#endif
    }
  } else {
     ret = false;
     std::cout<<"Kann Datenbanktreiber nicht initialisieren"<<std::endl;
  }

  defaultDB->close();
  return ret;
}

