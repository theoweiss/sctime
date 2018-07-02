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

#include "descdata.h"
#include <QRegularExpression>

DescData::DescData()
{
    m_description="";
    m_responsible="";
    m_type="";
    m_pspelem="";
}

DescData::DescData(const DescData& descdata)
{
    m_description=descdata.m_description;
    m_responsible=descdata.m_responsible;
    m_type=descdata.m_type;
    m_pspelem=descdata.m_pspelem;
}

DescData::DescData(const QString& description,const QString& responsible,const QString& type, const QString& pspelem)
{
    m_description=description;
    m_responsible=responsible;
    m_type=type;
    m_pspelem=pspelem;
}

QString DescData::description()
{
    return m_description;
}

QString DescData::responsible()
{
    return m_responsible;
}

QString DescData::type()
{
    return m_type;
}

QString DescData::pspElem()
{
    return m_pspelem;
}

void DescData::setDescription(const QString& description)
{
    m_description=description;
}

void DescData::setResponsible(const QString& responsible)
{
    m_responsible=responsible;
}

void DescData::setType(const QString& type)
{
    m_type=type;
}

void DescData::setPSPElem(const QString& pspelem)
{
    m_pspelem=pspelem;
}

bool DescData::supportsSpecialRemuneration()
{
    QRegularExpression re("\\((x|o)\\)$");
    QRegularExpressionMatch match = re.match(m_type);
    return ! match.hasMatch();
}
