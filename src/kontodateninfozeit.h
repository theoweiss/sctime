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

#ifndef KONTODATENINFOZEIT_H
#define  KONTODATENINFOZEIT_H

#include "kontodateninfo.h"
#include "qobject.h"

class Einchecker:QObject
{
Q_OBJECT

public:
    Einchecker(AbteilungsListe*);
    bool checkin (QDate date, const QString& konto, const QString& uko, int sek, int sekabzur, QString kommentar);

public slots:
    void onExit();
    void firstAbteilung();
    void firstKonto();
    void firstUnterkonto();
    void firstEintrag();
    void nextAbteilung();
    void nextKonto();
    void nextUnterkonto();
    void nextEintrag();

signals:
    void criticalError();
    void finishedEintrag();

private:
    AbteilungsListe* abtList;
    AbteilungsListe::iterator abtPos;
    KontoListe::iterator kontPos;
    UnterKontoListe::iterator ukontPos;
    EintragsListe::iterator etPos;
};

/**
  * Liest die Kontodaten ueber die Zeittools ein.
  */
class KontoDatenInfoZeit: public KontoDatenInfo
{
  public:
    virtual bool readInto(AbteilungsListe * abtlist);
    virtual bool checkIn(AbteilungsListe* abtlist);
  private:
    Einchecker* ec;
};

#endif
