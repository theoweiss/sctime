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

#ifndef KONTOLISTE_H
#define  KONTOLISTE_H

#include "unterkontoliste.h"

typedef std::map<QString,UnterKontoListe> Map_QString_UnterKontoListe; // Visual-C-Workaround

/** KontoListe ist eine Map zwischen den Namen des Kontos und den UnterKontoListen */
class KontoListe: public Map_QString_UnterKontoListe
{
  public:
    KontoListe(): std::map<QString,UnterKontoListe>()
    {
      flags=0;
      m_hasColor=false;
      m_bgColor=Qt::white;
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
      Map_QString_UnterKontoListe::clear();
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

