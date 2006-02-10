#ifndef DESCDATA_H
#define DESCDATA_H

#include <QString>

class DescData
{
    public:
        DescData();
        DescData(const DescData& descdata);
        DescData(const QString& description,const QString &responsible,const QString &type);
        QString description();
        QString responsible();
        QString type();
        void setDescription(const QString& description);
        void setResponsible(const QString&);
        void setType(const QString&);
    private:
        QString m_description;
        QString m_responsible;
        QString m_type;
};

#endif
