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

#ifndef UNTERKONTOLISTE_H
#define  UNTERKONTOLISTE_H

#include <map>
#include <QString>
#include <QColor>
class EintragsListe;

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
