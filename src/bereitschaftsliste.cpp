#include "bereitschaftsliste.h"
#include <assert.h>
#include <iostream>


void BereitschaftsListe::insertEintrag(QString bezeichnung, QString beschreibung, int flags)
{
   BereitschaftsEintrag eintrag;
   eintrag.bezeichnung=bezeichnung;
   eintrag.beschreibung=beschreibung;
   eintrag.flags=flags;
   append(eintrag);
}

BereitschaftsListe* BereitschaftsListe::getInstance()
{
  static BereitschaftsListe* instance=NULL;
  if (!instance)
    instance=new BereitschaftsListe();
  return instance;
}


