#include "specialremuntypelist.h"

SpecialRemunerationType::SpecialRemunerationType()
{
  this->category="";
  this->description="";
  this->isValid=true;
}

SpecialRemunerationType::SpecialRemunerationType(const QString& category, const QString& description, bool isvalid)
{
  this->category=category;
  this->description=description;
  this->isValid=isvalid;
}