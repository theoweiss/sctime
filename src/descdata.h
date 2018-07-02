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

#ifndef DESCDATA_H
#define DESCDATA_H

#include <QString>

class DescData
{
    public:
        DescData();
        DescData(const DescData& descdata);
        DescData(const QString& description,const QString &responsible,const QString &type, const QString &pspelem);
        QString description();
        QString responsible();
        QString type();
        QString pspElem();
        bool supportsSpecialRemuneration();
        void setDescription(const QString& description);
        void setResponsible(const QString&);
        void setType(const QString&);
        void setPSPElem(const QString&);
    private:
        QString m_description;
        QString m_responsible;
        QString m_type;
        QString m_pspelem;
};

#endif
