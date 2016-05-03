#include "bereitschaftsliste.h"

void BereitschaftsListe::insertEintrag(const QString& bezeichnung, const QString& beschreibung)
{
   BereitschaftsEintrag eintrag;
   eintrag.bezeichnung=bezeichnung;
   eintrag.beschreibung=beschreibung;
   append(eintrag);
}

BereitschaftsListe* BereitschaftsListe::getInstance()
{
  static BereitschaftsListe* instance=NULL;
  if (!instance)
    instance=new BereitschaftsListe();
  return instance;
}


