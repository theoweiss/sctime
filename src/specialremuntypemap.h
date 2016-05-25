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
