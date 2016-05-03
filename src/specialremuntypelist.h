#ifndef SPECIALREMUNERATIONLIST_H
#define SPECIALREMUNERATIONLIST_H

#include <QList>
#include <QString>

class SpecialRemunerationType
{
public:
  SpecialRemunerationType();
  SpecialRemunerationType(const QString& category, const QString& description, bool isvalid=true);
  
  QString category;
  QString description;
  bool isValid;
};

class SpecialRemunTypeList: public QList<SpecialRemunerationType>
{

};

#endif // SPECIALREMUNERATIONLIST_H
