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

#ifndef UKATTRLIST_H
#define UKATTRLIST_H

#include <map>
#include "unterkontoeintrag.h"
#include "qstring.h"

typedef std::map<int,UnterKontoEintrag> Map_Int_UnterKontoEintrag; // Visual-C-Workaround

class EintragsListe: public Map_Int_UnterKontoEintrag
{
  public:
    EintragsListe(): std::map<int,UnterKontoEintrag>()
    {
      beschreibungString="";
      defaultKommentar = "blub";
      flags=0;
    }

    void setBeschreibung(QString _beschreibung)
    {
      beschreibungString=_beschreibung;
    }

    QString beschreibung()
    {
      return beschreibungString;
    }
    
    QString getDefaultKommentar()
    {
      return defaultKommentar;
    }
    
    void setDefaultKommentar(const QString& defaultkommentar)
    {
      defaultKommentar = defaultkommentar;
    }
    
    void setFlags(int _flags)
    {
      flags=_flags;
    }

    int getFlags()
    {
      return flags;
    }
    
    void clear()
    {
      flags=0;
      Map_Int_UnterKontoEintrag::clear();
    }

  private:
    QString beschreibungString;
    QString defaultKommentar;
    int flags;
};

#endif
