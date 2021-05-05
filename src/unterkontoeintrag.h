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

#ifndef UNTERKONTOEINTRAG_H
#define UNTERKONTOEINTRAG_H

#include "specialremuntypemap.h"

#include <QString>
#include <QSet>

#define UK_PERSOENLICH 1
#define UK_AKTIV       2

/** Die Klasse UnterKontoEintrag speichert die Daten eines Unterkontos */

class UnterKontoEintrag
{
private:
  QSet<QString> m_achievedSpecialRemunSet;
  public:
    /** Erzeugt ein Unterkonto mit Namen _name, Kommenatr _kommentar, usw */
    UnterKontoEintrag(const QString& _kommentar="", int _sekunden=0, int _sekundenabzur=0, int _flags=0)
    {
      kommentar=_kommentar;
      sekunden=_sekunden;
      sekundenAbzur=_sekundenabzur;
      flags=_flags;
    }

    QString kommentar;
    int sekunden,sekundenAbzur;
    int flags;

    /** Erzeugt eine Kopie des uebergebenen Unterkontoeintrags. */
    UnterKontoEintrag(const UnterKontoEintrag& uk)
    {
      kommentar=uk.kommentar;
      sekunden=uk.sekunden;
      sekundenAbzur=uk.sekundenAbzur;
      flags=uk.flags;
      m_achievedSpecialRemunSet=m_achievedSpecialRemunSet;
    }

    bool isEmpty()
    {
      return ((kommentar=="")&&(sekunden==0)&&(sekundenAbzur==0));
    }

    bool operator==(const UnterKontoEintrag& uk)
    {
      return ((kommentar==uk.kommentar)&&(sekunden==uk.sekunden)&&
              (sekundenAbzur==uk.sekundenAbzur)&&(flags==uk.flags));
    }
    
    const QSet<QString>& getAchievedSpecialRemunSet() const
    {
      return m_achievedSpecialRemunSet;
    }
    
    void setAchievedSpecialRemunSet(const QSet<QString>& srtl)
    {
      m_achievedSpecialRemunSet=srtl;
    }
    
};

#endif
