/*
    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KONTOLISTE_H
#define  KONTOLISTE_H

#include <map>
#include <QColor>
#include <QString>
class UnterKontoListe;
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

