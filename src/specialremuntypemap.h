/*

    $Id$

    Copyright (C) 2016 Florian Schmitt, Science + Computing ag
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
