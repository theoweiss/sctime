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

#include "kontotreeview.h"
#include "qlistview.h"
#include "abteilungsliste.h"
#include "qstring.h"
#include "qpixmap.h"
#include "timecounter.h"
#include <iostream>
#include "globals.h"
#include "../pics/hi16_action_apply.xpm"
#include "kontotreetooltip.h"


/**
 * Erzeugt ein neues Objekt zur Anzeige des Kontobaums. Seine Daten bezieht es aus abtlist.
 */
KontoTreeView::KontoTreeView(QWidget *parent, AbteilungsListe* abtlist=0): QListView(parent, "Konten", 0)
{

  addColumn ("Konten");
  addColumn ("Aktiv");
  addColumn ("Zeit");
  setColumnAlignment(2,Qt::AlignRight);
  addColumn ("Abzur.");
  setColumnAlignment(3,Qt::AlignRight);
  addColumn ("Kommentar");

  aktivPixmap=new QPixmap((const char **)hi16_action_apply);

  setRootIsDecorated(TRUE);
  
  setSelectionMode(NoSelection);

  load(abtlist);
  new KontoTreeToolTip(this,abtlist);
}


/**
 * Sucht nach dem Blatt tops->abts->kos->ukos->idx im Kontobaum, und liefert die Knotenitems, die dahin
 * fuehren in topi,abti,koi,ukoi,eti zurueck.
 * Falls idx=-1 ist eti undefiniert.
 */
bool KontoTreeView::sucheItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos,
         int idx,
         KontoTreeItem* &topi, KontoTreeItem* &abti, KontoTreeItem* &koi, KontoTreeItem* &ukoi, KontoTreeItem* &eti)
{


  topi=abti=koi=ukoi=eti=NULL;

  for (topi=(KontoTreeItem*)firstChild(); (topi!=NULL)&&(topi->text(0)!=tops); topi=(KontoTreeItem*)(topi->nextSibling()));
  if (topi==NULL) return false;

  for (abti=(KontoTreeItem*)(topi->firstChild()); (abti!=NULL)&&(abti->text(0)!=abts); abti=(KontoTreeItem*)(abti->nextSibling()));
  if (abti==NULL) return false;

  for (koi=(KontoTreeItem*)(abti->firstChild()); (koi!=NULL)&&(koi->text(0)!=kos); koi=(KontoTreeItem*)(koi->nextSibling()));
  if (koi==NULL) return false;

  for (ukoi=(KontoTreeItem*)(koi->firstChild()); (ukoi!=NULL)&&(ukoi->text(0)!=ukos); ukoi=(KontoTreeItem*)(ukoi->nextSibling()));
  if (ukoi==NULL) return false;

  for (eti=(KontoTreeItem*)(ukoi->firstChild()); (eti!=NULL)&&(eti->text(0).toInt()!=idx); eti=(KontoTreeItem*)(eti->nextSibling()));
  //if ((eti==NULL)&&(idx!=0)) return false;

  return true;
}


/* Liefert im Gegensatz zu sucheItem nur ein Item zurueck, und zwar das des angegebenen kontos*/

KontoTreeItem* KontoTreeView::sucheKontoItem(const QString& tops, const QString& abts, const QString& kos)
{
  QListViewItem *topi,*abti,*koi;

  for (topi=firstChild(); (topi!=NULL)&&(topi->text(0)!=tops); topi=topi->nextSibling());
  if (topi==NULL) return NULL;

  for (abti=topi->firstChild(); (abti!=NULL)&&(abti->text(0)!=abts); abti=abti->nextSibling());
  if (abti==NULL) return NULL;

  for (koi=abti->firstChild(); (koi!=NULL)&&(koi->text(0)!=kos); koi=koi->nextSibling());

  return (KontoTreeItem*)koi;
}


/**
 * Liefert in tops, abts, kos, ukos, id den String bzw Wert des Toplevel-,Abteilungs-,...-Items
 */
void KontoTreeView::itemInfo(QListViewItem* item,QString& tops, QString& abts, QString& kos, QString& ukos, int& idx)
{
  tops=abts=kos=ukos="";
  idx=-1;

  for (int d=item->depth(); d>=0; d--) {
    switch (d) {
      case 0: tops=item->text(0); break;
      case 1: abts=item->text(0); break;
      case 2: kos=item->text(0); break;
      case 3: ukos=item->text(0); break;
      case 4: idx=item->text(0).toInt(); break;
    }
    item=item->parent();
  }
  if ((ukos!="")&&(idx==-1)) {
    UnterKontoListe::iterator ituk;
    UnterKontoListe *ukl;
    if (abtList->findUnterKonto(ituk,ukl,abts,kos,ukos)) idx=ituk->second.begin()->first;
  }
}

void KontoTreeView::flagClosedPersoenlicheItems()
{
  QListViewItem *topi, *abti, *koi, *ukoi;
  for (topi=firstChild(); (topi!=NULL)&&(topi->text(0)!=PERSOENLICHE_KONTEN_STRING); topi=topi->nextSibling());
  if (topi==NULL) return;

  int fm;
  for (abti=topi->firstChild(); (abti!=NULL); abti=abti->nextSibling()) {
    if (abti->isOpen()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
    abtList->setAbteilungFlags(abti->text(0),IS_CLOSED,fm);
    
    for (koi=abti->firstChild(); (koi!=NULL); koi=koi->nextSibling()) {
      if (koi->isOpen()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
      abtList->setKontoFlags(abti->text(0),koi->text(0),IS_CLOSED,fm);
      
      for (ukoi=koi->firstChild(); (ukoi!=NULL); ukoi=ukoi->nextSibling()) {
        if (ukoi->isOpen()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
          abtList->setUnterKontoFlags(abti->text(0),koi->text(0),ukoi->text(0),IS_CLOSED,fm);
      }
    }
  }
}

void KontoTreeView::closeFlaggedPersoenlicheItems()
{
  QListViewItem *topi, *abti, *koi, *ukoi;
  for (topi=firstChild(); (topi!=NULL)&&(topi->text(0)!=PERSOENLICHE_KONTEN_STRING); topi=topi->nextSibling());
  if (topi==NULL) return;
  for (abti=topi->firstChild(); (abti!=NULL); abti=abti->nextSibling()) {
    if (abtList->getAbteilungFlags(abti->text(0))&IS_CLOSED) {
      abti->setOpen(false);
      continue;
    }
    for (koi=abti->firstChild(); (koi!=NULL); koi=koi->nextSibling()) {
      if (abtList->getKontoFlags(abti->text(0),koi->text(0))&IS_CLOSED) {
        koi->setOpen(false);
        continue;
      }
      for (ukoi=koi->firstChild(); (ukoi!=NULL); ukoi=ukoi->nextSibling()) {
        if ((abtList->getUnterKontoFlags(abti->text(0),koi->text(0),ukoi->text(0))&IS_CLOSED)&&(ukoi->firstChild()))
          ukoi->setOpen(false);
        }
    }
  }
}

/**
 *  True, falls das uebergebene Item ein Item ist, das einen Eintrag repraesentiert.
 */

bool KontoTreeView::isEintragsItem(QListViewItem* item)
{
  if (!item) return false;
  int d=item->depth();
  return ((d==4)||((d==3)&&(item->firstChild()==NULL)));
}


/** Laedt die uebergebene Abteilungsliste in den Kontobaum
 */
void KontoTreeView::load(AbteilungsListe* abtlist)
{
  abtList=abtlist;

  QListViewItem *next, *topi;

  topi=firstChild();
  while (topi!=NULL) {
    next=topi->nextSibling();
    delete topi;
    topi=next;
  }

  KontoTreeItem* perskonten=new KontoTreeItem(this,PERSOENLICHE_KONTEN_STRING);
  KontoTreeItem* allekonten=new KontoTreeItem(this,ALLE_KONTEN_STRING);
  allekonten->setOpen(false);

  if (abtList) {

    AbteilungsListe::iterator abtPos;

    for (abtPos=abtList->begin(); abtPos!=abtList->end(); ++abtPos) {
      KontoTreeItem* abteilungsitem=new KontoTreeItem( allekonten, abtPos->first );
      KontoListe* kontoliste=&(abtPos->second);
      for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
        KontoTreeItem* kontoitem= new KontoTreeItem( abteilungsitem, kontPos->first);
        UnterKontoListe* unterkontoliste=&(kontPos->second);
        for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
          EintragsListe* eintragsliste=&(ukontPos->second);
          for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {

            TimeCounter tc(etPos->second.sekunden), tcAbzur(etPos->second.sekundenAbzur);
            if (etPos==eintragsliste->begin()) {
              KontoTreeItem* newItem=new KontoTreeItem( kontoitem, ukontPos->first, "", tc.toString(),
                                                      tcAbzur.toString() , etPos->second.kommentar);
             // newItem->setBold((etPos->second.kommentar!="")||(etPos->second.sekunden!=0)||(etPos->second.sekundenAbzur!=0));
            }
            //Unschoen, langsam! Sorgt dafuer, dass das Konto in Persoenliche Konten kommt
            if ((etPos->second.flags&UK_PERSOENLICH)||(etPos!=eintragsliste->begin()))
              refreshItem(abtPos->first,kontPos->first,ukontPos->first,etPos->first);
          }
        }
      }
    }
  }
  QString abt,ko,uko;
  int idx;
  abtList->getAktiv(abt,ko,uko,idx);
  refreshItem(abt,ko,uko,idx);
}


/**
 * Zaehlt die Eintraeg in der Eintragsliste etl mit den Flags flags.
 */

int countEintraegeWithFlags(EintragsListe* etl, int flags)
{
  int c=0;
  for (EintragsListe::iterator etiter=etl->begin(); etiter!=etl->end(); ++etiter) {
    if ((flags&((*etiter).second.flags))==flags) c++;
  }
  return c;
}


/**
 * Findet den ersten Eintrag mit den uebergebenen flags in der Eintragsliste etl, und liefert
 * die Nummer desselben zurueck, bzw -1, falls keiner gefunden wurde.
 */

int firstEintragWithFlags(EintragsListe* etl, int flags)
{

  for (EintragsListe::iterator etiter=etl->begin(); etiter!=etl->end(); ++etiter) {
    if ((flags&((*etiter).second.flags))==flags) return (*etiter).first;

  }
  return -1;
}


/**
 * Sorgt dafuer, dass die Darstellung des in abt,ko,uko uebergebenen Unterkontos aktualisiert,
 * und an die in abtList vorhandenen Daten angepasst wird.
 * Die Funktion ist relativ komplex, da immer ueberprueft werden muss, ob es in "persoenliche
 * Konten" auch geaendert werden muss. Ausserdem wird dafuer gesorgt, dass bei Bedarf
 * fuer die Eintraege jeweils ein eigener Ast benutzt wird.
 */
void KontoTreeView::refreshItem(const QString& abt, const QString& ko,const QString& uko, int idx)
{
  EintragsListe::iterator etiter;
  EintragsListe* etl;
  bool ukHasSubTree;
  bool newUkSubTreeOpened=false;

  if (!abtList->findEintrag(etiter,etl,abt,ko,uko,idx)) return;

  KontoTreeItem *topi,*abti,*koi,*ukoi,*eti;
  bool itemFound=(sucheItem(ALLE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti));
  if ((koi)&&(!ukoi)) {
    ukoi=new KontoTreeItem(koi,uko);
    itemFound=true;
  }
  
  if (itemFound) {
    bool etiFound=(eti!=NULL);
    ukHasSubTree=(etl->size()>1);
    if (!ukHasSubTree)
      eti=ukoi;
    else {
      if (ukoi->firstChild()==NULL) // Wir brauchen einen Unterbaum, der aber noch leer ist
      {
        QString qs;
        qs.setNum(etl->begin()->first);
        eti=new KontoTreeItem(ukoi,qs);
        ukoi->setPixmap(1,emptyPixmap);ukoi->setText(2,"");ukoi->setText(3,"");ukoi->setText(4,"");
        refreshItem(abt,ko,uko,etl->begin()->first);
        if (etl->begin()->first==idx) etiFound=true;
        newUkSubTreeOpened=true;
      }
      if (!etiFound) {
        QString qs;
        qs.setNum(idx);
        eti=new KontoTreeItem(ukoi,qs);
        ukoi->setOpen(true);
      }
    }

    eti->setText(4,etiter->second.kommentar);
    TimeCounter tc(etiter->second.sekunden), tcAbzur(etiter->second.sekundenAbzur);
    eti->setText(2,tc.toString());
    eti->setText(3,tcAbzur.toString());
    //eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));

    eti->setBold((etiter->second.flags)&UK_PERSOENLICH);

    if (abtList->isAktiv(abt,ko,uko,idx))
      {
        eti->setPixmap(1,*aktivPixmap);
        setSelected(eti,true);
      }
    else
      eti->setPixmap(1,emptyPixmap);

    bool inPersKontenGefunden=sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti);


    if ((inPersKontenGefunden)&&(!((etiter->second.flags)&UK_PERSOENLICH))) {
      if (eti!=NULL)
        delete eti;
      if (sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti)) { // Wegen moeglichen Seiteneffekten von delete
        if (!ukoi->firstChild()) {
          delete ukoi;
          if (!koi->firstChild()) {
            delete koi;
            if (!abti->firstChild()) delete abti;
          }
        }
      }
    }
    if ((!inPersKontenGefunden)&&((etiter->second.flags)&UK_PERSOENLICH)) {
      if (!topi) topi=new KontoTreeItem(this,PERSOENLICHE_KONTEN_STRING);
      if (!abti) abti=new KontoTreeItem(topi,abt);
      if (!koi) koi=new KontoTreeItem(abti,ko);
      if (!ukoi) {
        if (!ukHasSubTree) {
          eti=ukoi=new KontoTreeItem(koi,uko,"",tc.toString(), tcAbzur.toString(),etiter->second.kommentar);
        }
        else
          ukoi=new KontoTreeItem(koi,uko);
      }
        if (!eti) {

          eti=new KontoTreeItem(ukoi,QString().setNum(idx),"",tc.toString(), tcAbzur.toString(),etiter->second.kommentar);

      }
      topi->setOpen(true); abti->setOpen(true); koi->setOpen(true); ukoi->setOpen(true);
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
      if (abtList->isAktiv(abt,ko,uko,idx))
        eti->setPixmap(1,*aktivPixmap);
      else
        eti->setPixmap(1,emptyPixmap);
    }
    if ((inPersKontenGefunden)&&((etiter->second.flags)&UK_PERSOENLICH)) {
      int firstEintrag=firstEintragWithFlags(etl,UK_PERSOENLICH);
      if (!ukHasSubTree)
        eti=ukoi;
      else {        
        bool etiFound=(eti!=NULL);
        if (newUkSubTreeOpened)
        { /*QString qs;
          qs.setNum(firstEintrag);*/
          ukoi->setPixmap(1,emptyPixmap);ukoi->setText(2,"");ukoi->setText(3,"");ukoi->setText(4,"");
        }
        if (!etiFound) {
          QString qs;
          qs.setNum(idx);
          eti=new KontoTreeItem(ukoi,qs);
          ukoi->setOpen(true);
        }
      }

      if (abtList->isAktiv(abt,ko,uko,idx))
        eti->setPixmap(1,*aktivPixmap);
      else
        eti->setPixmap(1,emptyPixmap);
      eti->setText(4,etiter->second.kommentar);
      eti->setText(2,tc.toString());
      eti->setText(3,tcAbzur.toString());
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
    }
  }
}


/*
 * Ruft refreshItem fuer alle Eintraege im uebergebenen Unterkonto auf
 */
void KontoTreeView::refreshAllItemsInUnterkonto(const QString& abt, const QString& ko,const QString& uko)
{
  UnterKontoListe::iterator itUk;
  UnterKontoListe* unterkontoliste;

  if (!abtList->findUnterKonto(itUk,unterkontoliste,abt,ko,uko)) return;

  EintragsListe* eintragsliste=&(itUk->second);
  for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {
    refreshItem(abt,ko,uko,etPos->first);
  }
}


/*
 * Ruft refreshItem fuer alle Eintraege im uebergebenen Konto auf
 */
void KontoTreeView::refreshAllItemsInKonto(const QString& abt, const QString& ko)
{
  KontoListe::iterator itKo;
  KontoListe* kontoliste;

  if (!abtList->findKonto(itKo,kontoliste,abt,ko)) return;

  UnterKontoListe* unterkontoliste=&(itKo->second);
  for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
    EintragsListe* eintragsliste=&(ukontPos->second);
    for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {
      refreshItem(abt,ko,ukontPos->first,etPos->first);
    }
  }
}

