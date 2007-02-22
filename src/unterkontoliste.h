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

#ifndef UNTERKONTOLISTE_H
#define  UNTERKONTOLISTE_H

#include <qstring.h>
#include <map>
#include "eintragsliste.h"

typedef std::map<QString,EintragsListe> Map_QString_EintragsListe; // Visual-C-Workaround

class UnterKontoListe: public std::map<QString,EintragsListe>
{
  public:
    UnterKontoListe(): std::map<QString,EintragsListe>()
    {
      flags=0;
      m_bgColor=Qt::white;
      m_hasColor=false;
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
      Map_QString_EintragsListe::clear();
    }

    void setBgColor(QColor bgColor)
    {
      m_bgColor=bgColor;
      m_hasColor=true;
    }

    QColor getBgColor()
    {
      return m_bgColor;
    }

    bool hasBgColor()
    {
      return m_hasColor;
    }

    void unsetBgColor()
    {
      m_hasColor=false;
    }

  private:
    int flags;
    QColor m_bgColor;
    bool m_hasColor;
};

#endif
