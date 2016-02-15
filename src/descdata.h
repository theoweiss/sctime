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
