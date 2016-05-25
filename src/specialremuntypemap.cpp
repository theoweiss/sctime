#include "specialremuntypemap.h"

SpecialRemunerationType::SpecialRemunerationType()
{
  this->description="";
  this->isValid=true;
}

SpecialRemunerationType::SpecialRemunerationType(const QString& description, bool isvalid)
{
  this->description=description;
  this->isValid=isvalid;
}