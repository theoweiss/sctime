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
#include <QSqlDatabase>
#include <QMessageBox>
#include <QApplication>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include "globals.h"
#include "descdata.h"
#include "DBConnector.h"

BereitschaftsDatenInfoDatabase::BereitschaftsDatenInfoDatabase(DBConnector* dbconnector)
{
  m_dbconnector=dbconnector;
}

/**
 * Liest aus einer ODBC-Datenbank nach abtList
 */
bool BereitschaftsDatenInfoDatabase::readInto(BereitschaftsListe * berlist)
{
  bool ret = false;
  QApplication::addLibraryPath(QApplication::applicationDirPath() + "/lib");
  QSqlDatabase defaultDB = QSqlDatabase::addDatabase( "QODBC" );
  m_dbconnector->configureDB(defaultDB);
  QString step = QObject::tr("Datenbank öffnen");
    if ( defaultDB.open() ) {
      step = QObject::tr("Bereitschaftsarten anfordern");
      QSqlQuery query(
          "SELECT kategorie, beschreibung FROM v_bereitschaft_sctime;",
            defaultDB);
      if ( query.isActive() ) {
	step = QObject::tr("Bereitschaftsarten einlesen");
        while ( query.next() ) {
	  ret = true;
          QString name = query.value(0).toString().simplified();
          QString beschreibung = query.value(1).toString().simplified();
          if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.
          berlist->insertEintrag(name, beschreibung, IS_IN_DATABASE);
        }
      }
    }
    if (!ret){
      QMessageBox::warning(NULL, QObject::tr("sctime: Bereitschaftsarten laden"),
	step + ": " + defaultDB.lastError().driverText());			     
    }
  defaultDB.close();
  return ret;
}



