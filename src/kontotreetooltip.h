/*

    $Id$

    Copyright (C) 2003 Florian Schmitt, Science and Computing AG
                       f.schmitt@science-computing.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef KONTOTREETOOLTIP_H
#define KONTOTREETOOLTIP_H

#include "kontotreeitem.h"
#include "kontotreeview.h"
#include <qtooltip.h>
#include "abteilungsliste.h"
#include <iostream>
#include "qdatetime.h"


/** Die Klasse KontoTreeToolTip zeigt die Beschreibung zum Unterkonto an */

class KontoTreeToolTip:public QToolTip
{
  public:
    KontoTreeToolTip(KontoTreeView* kontotree, AbteilungsListe* abtlist) :QToolTip (kontotree->viewport())
    {
      abtList=abtlist;
      kontoTree=kontotree;
    }
    
    virtual ~KontoTreeToolTip() {};
    
  protected:
    virtual void maybeTip ( const QPoint & p )
    {
      KontoTreeItem * item=(KontoTreeItem *)(kontoTree->itemAt(p));
      if (!kontoTree->isEintragsItem(item)) return;
      QString top,uko,ko,abt;
      int idx;
      kontoTree->itemInfo(item,top,abt,ko,uko,idx);
      QString beschreibung=abtList->getBeschreibung(abt,ko,uko);
      if (beschreibung.stripWhiteSpace()!="") {
        tip(kontoTree->itemRect(item),beschreibung);
      }
    }
    
    private:
      AbteilungsListe* abtList;
      KontoTreeView* kontoTree;
} ;

#endif
