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

#include "bereitschaftsdateninfodatabase.h"
#include "bereitschaftsliste.h"
#include "abteilungsliste.h"
#include <iostream>
#include "qsqldatabase.h"
#include "qmessagebox.h"
#include "qapplication.h"
#include "qvariant.h"
#include <QSqlQuery>
#include "globals.h"
#include "descdata.h"

/**
 * Liest aus einer ODBC-Datenbank nach abtList
 */
bool BereitschaftsDatenInfoDatabase::readInto(BereitschaftsListe * berlist)
{
  bool ret = true;
  // PluginDir setzen
  QApplication::addLibraryPath(execDir+"/lib");
  QSqlDatabase defaultDB = QSqlDatabase::addDatabase( "QODBC3" );
  defaultDB.setDatabaseName( "zeit" );

  if ( defaultDB.open() ) {
      QSqlQuery query(
          "SELECT t_bereitschaft.kategorie, t_bereitschaft.beschreibung FROM FROM t_bereit_saetze "
          "INNER JOIN t_bereitschaft ON t_bereit_saetze.kategorie_id = "
          "t_bereitschaft.id "
          "WHERE t_bereit_saetze.datum_seit < current_Date AND "
          "t_bereit_saetze.gueltig_bis Is Null;",
            defaultDB);
      if ( query.isActive() ) {
        while ( query.next() ) {
          QString name = query.value(0).toString().simplifyWhiteSpace();

          QString beschreibung = query.value(1).toString().simplifyWhiteSpace();

          if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.

          berlist->insertEintrag(name, beschreibung, IS_IN_DATABASE);
        }
      }  else std::cout<<"Kann Abfrage nicht durchfuehren"<<std::endl;
  } else {
        ret = false;
#ifdef WIN32
                QMessageBox::critical(NULL,"Error","Kann Datenbank nicht öffnen\n",
                              QMessageBox::Ok, Qt::NoButton,
                              Qt::NoButton);
#else
                std::cout<<"Kann Datenbank nicht öffnen"<<std::endl;
#endif
    }

  defaultDB.close();
  return ret;
}



