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

#ifndef UNTERKONTOEINTRAG_H
#define UNTERKONTOEINTRAG_H

#include <qstring.h>
#include <iostream>

#define UK_PERSOENLICH 1
#define UK_AKTIV       2

/** Die Klasse UnterKontoEintrag speichert die Daten eines Unterkontos */

class UnterKontoEintrag
{
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
} ;

#endif
