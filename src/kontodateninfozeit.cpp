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

// Wird unter Win32 nicht benoetigt
#ifndef WIN32

#include "kontodateninfozeit.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include "qregexp.h"


void KontoDatenInfoZeit::readInto(AbteilungsListe * abtList)
{
  abtList->clear();
  char rest[800], konto[200], unterkonto[200];

  FILE* file;

  if ((file = popen("zeitkonten --beschreibung", "r")) != NULL)
  {
    QRegExp kostenstelle("\\s.\\d\\d\\d\\d\\d\\b");
    while (!feof(file)) {
      // Konto, unterkonto sind eindeutig durch leerzeichen getrennt, 
      // der Rest muss gesondert behandelt werden.
      if (fscanf(file,"%s%s%[^\n]",konto,unterkonto,rest)==3) {
        // Falls alle drei Strings korrekt eingelesen wurden...
                
        QString qstringrest(rest), abt,beschreibung;

        // Kostenstelle trennt Abteilung von Beschreibung, also
        // dort splitten
        abt = qstringrest.section(kostenstelle,0,0).simplifyWhiteSpace();
        beschreibung = qstringrest.section(kostenstelle,1);
 
        if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.

        abtList->insertEintrag(abt,konto,unterkonto);
        abtList->setBeschreibung(abt,konto,unterkonto,beschreibung);
      }  
    }
    pclose(file);
  }
  else std::cerr<<"Kann \"zeitkonten\" nicht ausfuehren."<<std::endl;
}


#endif