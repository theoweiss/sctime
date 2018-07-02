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

#ifndef BEREITSCHAFTSLISTE_H
#define BEREITSCHAFTSLISTE_H

#include <QList>
#include <QString>

struct BereitschaftsEintrag
{
  QString bezeichnung;
  QString beschreibung;
};

class BereitschaftsListe: public QList<BereitschaftsEintrag>
{
public:
  void insertEintrag(const QString& bezeichnung, const QString& beschreibung);
  static BereitschaftsListe* getInstance();
private:
  BereitschaftsListe() {};
};

#endif // BEREITSCHAFTSLISTE_H
