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

#ifndef KONTODATENINFODATBASE_H
#define  KONTODATENINFODATABASE_H

#include "kontodateninfo.h"

class DBConnector;

/**
 * Implementierung von KontoDatenInfo zum Zugriff auf die Datenbank
 */
class KontoDatenInfoDatabase: public KontoDatenInfo
{
  public:
     KontoDatenInfoDatabase(DBConnector* dbconnector);
    virtual bool readInto(AbteilungsListe * abtlist);
    virtual bool checkIn(AbteilungsListe* abtlist);  
    bool readDefaultCommentsInto (AbteilungsListe * abtList);
  private:
    DBConnector* m_dbconnector;
};

#endif
