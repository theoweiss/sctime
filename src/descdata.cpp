#include "descdata.h"

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
