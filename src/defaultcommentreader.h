/* $Id$ */

#ifndef DEFAULTCOMMENTREADER_H
#define DEFAULTCOMMENTREADER_H

#include "abteilungsliste.h"

class DefaultCommentReader
{
  public:

    DefaultCommentReader(AbteilungsListe* abtlist)
    {
      abtList = abtlist;
    }

    void read();

  private:

    AbteilungsListe* abtList;
};

#endif
