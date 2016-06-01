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


#ifndef SETUPDSM_H
#define SETUPDSM_H

class SCTimeXMLSettings;
class DatasourceManager;
class QString;
class QStringList;
void setupDatasources(const QStringList& datasourceNames,
                      const SCTimeXMLSettings& settings,
                      const QString &kontenPath, const QString &bereitPath, const QString& specialremunfile);
extern DatasourceManager* kontenDSM;
extern DatasourceManager* bereitDSM;
extern DatasourceManager* specialRemunDSM;

#endif // SETUPDSM_H
