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

#ifndef BEREITSCHAFTSDATENINFOZEIT_H
#define  BEREITSCHAFTSDATENINFOZEIT_H

class QTextStream;
class QString;
class BereitschaftsListe;
#include "bereitschaftsdateninfo.h"

/**
  * Liest die Bereitschaftsdaten ueber die Zeittools ein.
  */
class BereitschaftsDatenInfoZeit: public BereitschaftsDatenInfo
{
  public:
    BereitschaftsDatenInfoZeit();
    BereitschaftsDatenInfoZeit(QString sourcefile);
    virtual bool readInto(BereitschaftsListe * berlist);
    bool readBereitschaftsFile(QTextStream &ts, BereitschaftsListe * berlist);
    virtual ~BereitschaftsDatenInfoZeit() {};
  private:
    QString m_DatenFileName;
};

#endif
