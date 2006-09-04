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

#include "bereitschaftsdateninfozeit.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include "qregexp.h"
#include "globals.h"
#include "utils.h"
#include "qmessagebox.h"
#include "abteilungsliste.h"


BereitschaftsDatenInfoZeit::BereitschaftsDatenInfoZeit()
{
    m_DatenFileName="";
}

BereitschaftsDatenInfoZeit::BereitschaftsDatenInfoZeit(QString sourcefile)
{
    m_DatenFileName=sourcefile;
}

bool BereitschaftsDatenInfoZeit::readBereitschaftsFile(FILE* file, BereitschaftsListe * berList)
{
    char zeile[800];
    int bereitschaftsCounter=0;

    while (!feof(file)) {
      // Konto, unterkonto sind eindeutig durch leerzeichen getrennt,
      // der Rest muss gesondert behandelt werden.
        if (fscanf(file,"%[^\n]",zeile)==1) {
        // Falls alle drei Strings korrekt eingelesen wurden...

            bereitschaftsCounter++;
            QString qstringzeile(zeile);
            QStringList ql = qstringzeile.split("|");

            if (ql.size()<2) {
                continue;
            }

            QString name = ql[0].simplifyWhiteSpace();

            QString beschreibung = ql[1].simplifyWhiteSpace();

            if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.

            berList->insertEintrag(name, beschreibung, IS_IN_DATABASE);

        }
        fscanf(file,"\n");
    }
    return (bereitschaftsCounter>0);
}

bool BereitschaftsDatenInfoZeit::readInto(BereitschaftsListe * berList)
{
  berList->clear();
  FILE* file;
  if (m_DatenFileName.isEmpty()) {
    file = popen("zeitbereitls --separator='|'", "r");
    if (!file) {
      std::cerr<<"Kann \"zeitbereitls\" nicht ausfuehren."<<std::endl;
      return false;
    }
  } else {
      file = fopen(m_DatenFileName, "r");
      if (!file) {
          std::cerr<<"Kann "<<m_DatenFileName.toLocal8Bit().constData()<<" nicht oeffnen."<<std::endl;
          return false;
      }
  }
  bool result=readBereitschaftsFile(file,berList);
  if (m_DatenFileName.isEmpty())
      pclose(file);
  else
      fclose(file);
  return result;
}




