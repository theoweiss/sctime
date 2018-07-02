/*
    Copyright (C) 2018 science+computing ag
       Authors: Johannes Abt et al.

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


#ifndef SETUPDSM_H
#define SETUPDSM_H

class SCTimeXMLSettings;
class DatasourceManager;
class QString;
class QStringList;
void setupDatasources(const QStringList& datasourceNames,
                      const SCTimeXMLSettings& settings,
                      const QString &kontenPath, const QString &bereitPath, const QString& specialremunfile, const QString& jsonfile);
extern DatasourceManager* kontenDSM;
extern DatasourceManager* bereitDSM;
extern DatasourceManager* specialRemunDSM;

#endif // SETUPDSM_H
