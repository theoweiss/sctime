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


#ifndef SPECIALREMUNERATIONLIST_H
#define SPECIALREMUNERATIONLIST_H

#include <QMap>
#include <QString>

class SpecialRemunerationType
{
public:
  SpecialRemunerationType();
  SpecialRemunerationType(const QString& description, bool isvalid=true);
  
  QString description;
  bool isValid;
};

class SpecialRemunTypeMap: public QMap<QString,SpecialRemunerationType>
{

};

#endif // SPECIALREMUNERATIONLIST_H
