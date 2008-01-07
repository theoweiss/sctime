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
#include "q3listview.h"
#include "abteilungsliste.h"
#include "qstring.h"
#include "qpixmap.h"
#include "timecounter.h"
#include <iostream>
#include "qevent.h"
#include "globals.h"
#include "descdata.h"
#include "../pics/hi16_action_apply.xpm"
#include "../pics/hi16_action_stamp.xpm"
#include <vector>
#include <QApplication>


/**
 * Erzeugt ein neues Objekt zur Anzeige des Kontobaums. Seine Daten bezieht es aus abtlist.
 */
KontoTreeView::KontoTreeView(QWidget *parent, AbteilungsListe* abtlist, const std::vector<int>& columnwidth): Q3ListView(parent, "Konten", 0)
{

  addColumn ("Konten");
  setColumnAlignment(1,Qt::AlignCenter);
  addColumn ("Typ");
  addColumn ("Aktiv");
  addColumn ("Zeit");
  setColumnAlignment(3,Qt::AlignRight);
  addColumn ("Abzur.");
  setColumnAlignment(4,Qt::AlignRight);
  addColumn ("Kommentar");

  aktivPixmap=QPixmap((const char **)hi16_action_apply);
  bereitPixmap=QPixmap((const char **)hi16_action_stamp);

  setRootIsDecorated(true);
  viewport()->installEventFilter(this);

#ifndef MACOS
  setSelectionMode(Q3ListView::NoSelection);
#else
  setSelectionMode(Q3ListView::Single);
#endif

  load(abtlist);
  for (unsigned int i=0; i<columnwidth.size(); i++) {
      setColumnWidth(i,columnwidth[i]);
  }
}

#ifdef USE_QT4_DRAGNDROP
void KontoTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}

void KontoTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QPoint contpos(dragStartPosition);
    contpos-=QPoint(viewport()->pos());
    KontoTreeItem * item=dynamic_cast<KontoTreeItem *>(itemAt(contpos));
    if (isEintragsItem(item)) {
        QString top,uko,ko,abt;
        int idx;
        itemInfo(item,top,abt,ko,uko,idx);
        UnterKontoEintrag eintrag;
        abtList->getEintrag(eintrag,abt,ko,uko,idx);

        QString data;
        data.sprintf("%i|%i|%s",eintrag.sekunden,eintrag.sekundenAbzur,eintrag.kommentar);

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setData("application/sctime.seconds", data.toLocal8Bit());
        // mimeData->setText(data);
        drag->setMimeData(mimeData);

        Qt::DropAction dropAction = drag->start(Qt::MoveAction);
    }
}

#endif

void KontoTreeView::dropEvent(QDropEvent *event)
{
	event->setDropAction(Qt::MoveAction);
	if (event->action()!=QDropEvent::Move) {
		return;
	}
    QPoint contpos(event->pos());
    contpos-=QPoint(viewport()->pos());
    KontoTreeItem * item=dynamic_cast<KontoTreeItem *>(itemAt(contpos));
    if (isEintragsItem(item)) {
        QString top,uko,ko,abt;
        int idx;
        
        QString data;
        data=data.fromLocal8Bit(event->encodedData("application/sctime.account"));        
        QStringList datlist=data.split("|");
        UnterKontoEintrag eintrag;
        abtList->getEintrag(eintrag,datlist[1],datlist[2],datlist[3],datlist[4].toInt());

        int deltasek=eintrag.sekunden;
        int deltasekabzur=eintrag.sekundenAbzur;
        
        // clear old entry
        abtList->setSekunden(datlist[1],datlist[2],datlist[3],datlist[4].toInt(),0);
        abtList->setSekundenAbzur(datlist[1],datlist[2],datlist[3],datlist[4].toInt(),0);
        refreshItem(datlist[1],datlist[2],datlist[3],datlist[4].toInt());
        
        // add delta to new entry
        itemInfo(item,top,abt,ko,uko,idx);
        abtList->getEintrag(eintrag,abt,ko,uko,idx);
        abtList->setSekunden(abt,ko,uko,idx,deltasek+eintrag.sekunden);
        abtList->setSekundenAbzur(abt,ko,uko,idx,deltasekabzur+eintrag.sekundenAbzur);
        refreshItem(abt,ko,uko,idx);
        event->acceptAction();
    }
}


void KontoTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if ((event->action()!=QDropEvent::Move) && (event->mimeData()->hasFormat("application/sctime.account"))) {
        QPoint contpos(event->pos());
        contpos-=QPoint(viewport()->pos());
        KontoTreeItem * item=dynamic_cast<KontoTreeItem *>(itemAt(contpos));
        if (isEintragsItem(item)) {
           event->acceptAction();
        }
    }
}

void KontoTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if ((event->action()!=QDropEvent::Move) && (event->mimeData()->hasFormat("application/sctime.account"))) {
        QPoint contpos(event->pos());
        contpos-=QPoint(viewport()->pos());
        KontoTreeItem * item=dynamic_cast<KontoTreeItem *>(itemAt(contpos));
        if (isEintragsItem(item)) {
           event->acceptAction();
        }
        else {
        	event->ignore();
        }
    }
}

Q3DragObject* KontoTreeView::dragObject ()
{
	KontoTreeItem * item=dynamic_cast<KontoTreeItem *>(currentItem());
    if (isEintragsItem(item)) {
    	QString top,uko,ko,abt;
        int idx;
        itemInfo(item,top,abt,ko,uko,idx);

        QString data;
        data=top+"|"+abt+"|"+ko+"|"+uko+"|"+data.setNum(idx);

        Q3StoredDrag* dragobject=new Q3StoredDrag("application/sctime.account",this);
	    dragobject->setEncodedData(data.toLocal8Bit());	    
        bool result=dragobject->dragMove();                  
    }
	return NULL;	
}

void KontoTreeView::getColumnWidthList(std::vector<int>& columnwidth)
{
    columnwidth.clear();
    for (int i=0; i<columns(); i++) {
        columnwidth.push_back(columnWidth(i));
    }
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
  Q3ListViewItem *topi,*abti,*koi;

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
void KontoTreeView::itemInfo(Q3ListViewItem* item,QString& tops, QString& abts, QString& kos, QString& ukos, int& idx)
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

/** Versieht geschlossene, persoenliche Eintraege mit dem Flag IS_CLOSED */
void KontoTreeView::flagClosedPersoenlicheItems()
{
  Q3ListViewItem *topi, *abti, *koi, *ukoi;
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

/** Schliesst persoenliche Eintraege mit dem Flag IS_CLOSED */
void KontoTreeView::closeFlaggedPersoenlicheItems()
{
  Q3ListViewItem *topi, *abti, *koi, *ukoi;
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

bool KontoTreeView::isEintragsItem(Q3ListViewItem* item)
{
  if (!item) return false;
  int d=item->depth();
  return ((d==4)||((d==3)&&(item->firstChild()==NULL)));
}

/**
 *  True, falls das uebergebene Item ein Item ist, das ein Unterkonto repraesentiert.
 */

bool KontoTreeView::isUnterkontoItem(Q3ListViewItem* item)
{
    if (!item) return false;
    int d=item->depth();
    return (d==3);
}

/** oeffnet den Ast zum aktiven Projekt, so dass man dieses sieht*/
void KontoTreeView::showAktivesProjekt()
{
  QString uko,ko,abt,top;
  int idx;
  abtList->getAktiv(abt, ko, uko, idx);
  int flags = abtList->getEintragFlags(abt,ko,uko,idx);
  if (flags & UK_PERSOENLICH)
    top = PERSOENLICHE_KONTEN_STRING;
  else
    top = ALLE_KONTEN_STRING;
  KontoTreeItem *topi,*abti,*koi,*ukoi,*eti;
  bool itemFound=(sucheItem(top,abt,ko,uko,idx,topi,abti,koi,ukoi,eti));
  if (itemFound) {
     topi->setOpen(true);
     abti->setOpen(true);
     koi->setOpen(true);
     ukoi->setOpen(true);
  }
}

/** Laedt die uebergebene Abteilungsliste in den Kontobaum
 */
void KontoTreeView::load(AbteilungsListe* abtlist)
{
  abtList=abtlist;

  Q3ListViewItem *next, *topi;

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
      abteilungsitem->setBgColor(abtList->getBgColor(abtPos->first));
      for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
        KontoTreeItem* kontoitem= new KontoTreeItem( abteilungsitem, kontPos->first);
        UnterKontoListe* unterkontoliste=&(kontPos->second);
        kontoitem->setBgColor(abtList->getBgColor(abtPos->first,kontPos->first));
        for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
          if ((ukontPos->second.getFlags()&IS_DISABLED)!=0) {
              continue; // Deaktivierte Unterkonten nicht anzeigen
          }
          EintragsListe* eintragsliste=&(ukontPos->second);
          DescData dd=eintragsliste->description();
          for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {            
            TimeCounter tc(etPos->second.sekunden), tcAbzur(etPos->second.sekundenAbzur);
            if (etPos==eintragsliste->begin()) {
                KontoTreeItem* newItem=new KontoTreeItem( kontoitem, ukontPos->first, dd.type(), "", tc.toString(),
                                                      tcAbzur.toString() , etPos->second.kommentar);
                newItem->setGray(abtList->checkInState());
                newItem->setDragEnabled(true);
                newItem->setDropEnabled(true);
                newItem->setBgColor(abtList->getBgColor(abtPos->first,kontPos->first,ukontPos->first));
             // newItem->setBold((etPos->second.kommentar!="")||(etPos->second.sekunden!=0)||(etPos->second.sekundenAbzur!=0));
            }
            // Sorgt dafuer, dass das Konto in Persoenliche Konten kommt, bzw die Parentzeiten aktualisiert werden
            if ((etPos->second.flags&UK_PERSOENLICH)||(etPos!=eintragsliste->begin())||(etPos->second.sekunden)||(etPos->second.sekundenAbzur))
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

bool KontoTreeView::eventFilter ( QObject* obj, QEvent * e )
{
  if ((obj==viewport())&&(e->type()==QEvent::ToolTip))
  {
        QHelpEvent* qh= dynamic_cast <QHelpEvent*>(e);
        QPoint contpos(qh->pos());
        //contpos-=QPoint(viewport()->pos());
        KontoTreeItem * item=dynamic_cast<KontoTreeItem *>(itemAt(contpos));
        if (isEintragsItem(item)) {
            QString top,uko,ko,abt;
            int idx;
            itemInfo(item,top,abt,ko,uko,idx);
            QString beschreibung=abtList->getDescription(abt,ko,uko).description().simplifyWhiteSpace();
            if (beschreibung!="") {
               QToolTip::showText(qh->globalPos(),beschreibung,this);
                qh->accept();
                return true;
            }
        }
  }
  return Q3ListView::eventFilter(obj,e);
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

  QStringList bereitschaften=etl->getBereitschaft();

  KontoTreeItem *topi,*abti,*koi,*ukoi,*eti;
  bool itemFound=(sucheItem(ALLE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti));
  if ((koi)&&(!ukoi)) {
    ukoi=new KontoTreeItem(koi,uko);
    itemFound=true;
  }

  koi->setBgColor(abtList->getBgColor(abt,ko));
  abti->setBgColor(abtList->getBgColor(abt));

  ukoi->setBold((etl->getFlags())&UK_PERSOENLICH);
  ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));

  if ((etl->getFlags()&IS_DISABLED)!=0)
    return; // Deaktivierte Unterkonten nicht anfassen

  if (itemFound) {
    bool etiFound=(eti!=NULL);
    ukHasSubTree=((etl->size()+bereitschaften.size())>1);
    DescData dd=etl->description();
    if (!ukHasSubTree)
    {
      if (ukoi->firstChild())
        delete ukoi->firstChild();
      eti=ukoi;
    }
    else {
      if (ukoi->firstChild()==NULL) // Wir brauchen einen Unterbaum, der aber noch leer ist
      {
        QString qs;
        qs.setNum(etl->begin()->first);
        eti=new KontoTreeItem(ukoi,qs);
        ukoi->setPixmap(2,emptyPixmap);ukoi->setText(1,"");ukoi->setText(3,"");ukoi->setText(4,"");ukoi->setText(5,"");
        ukoi->setDragEnabled(false);
        ukoi->setDropEnabled(false);
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
      ukoi->setText(1,"");
      ukoi->setText(3,"");
      ukoi->setText(4,"");
      ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
      if (bereitschaften.size()>=1) {
        ukoi->setPixmap(2,bereitPixmap);
        ukoi->setText(5,bereitschaften.join("; "));
      }
      else {
        ukoi->setPixmap(2,emptyPixmap);
        ukoi->setText(5,"");
      }
    }

    eti->setText(5,etiter->second.kommentar);
    TimeCounter tc(etiter->second.sekunden), tcAbzur(etiter->second.sekundenAbzur);
    eti->setText(1,dd.type());
    eti->setText(3,tc.toString());
    eti->setText(4,tcAbzur.toString());
    eti->setDragEnabled(true);
    eti->setDropEnabled(true);
    eti->setBgColor(abtList->getBgColor(abt,ko,uko));
    //eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));

    eti->setBold((etiter->second.flags)&UK_PERSOENLICH);

    if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
      {
        eti->setPixmap(2,aktivPixmap);
        setSelected(eti,true);
      }
    else
      eti->setPixmap(2,emptyPixmap);
    refreshParentSumTime(ukoi,"+");
    refreshParentSumTime(koi,"++");
    bool inPersKontenGefunden=sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti);

    if ((inPersKontenGefunden)&&(!((etiter->second.flags)&UK_PERSOENLICH))) {
      if (eti!=NULL)
        delete eti;
      if (sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti)) { // Wegen moeglichen Seiteneffekten von delete
        if (!ukoi->firstChild()) {
          delete ukoi;
          if (!koi->firstChild()) {
            delete koi;
            if (!abti->firstChild())
              delete abti;
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
            eti=ukoi=new KontoTreeItem(koi,uko,dd.type(), "",tc.toString(), tcAbzur.toString(),etiter->second.kommentar);
        }
        else
          ukoi=new KontoTreeItem(koi,uko);
      }
        if (!eti) {
            eti=new KontoTreeItem(ukoi,QString().setNum(idx),dd.type(), "",tc.toString(), tcAbzur.toString(),etiter->second.kommentar);
      }
      eti->setDragEnabled(true);
      eti->setDropEnabled(true);
      topi->setOpen(true); abti->setOpen(true); koi->setOpen(true); ukoi->setOpen(true);
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
      //eti->setGray(abtList->checkInState());
      if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
        eti->setPixmap(2,aktivPixmap);
      else
        eti->setPixmap(2,emptyPixmap);
      QColor color=abtList->getBgColor(abt,ko,uko);
      eti->setBgColor(color);
      ukoi->setBgColor(color);
      koi->setBgColor(abtList->getBgColor(abt,ko));
      abti->setBgColor(abtList->getBgColor(abt));
    }
    if ((inPersKontenGefunden)&&((etiter->second.flags)&UK_PERSOENLICH)) {
      int firstEintrag=firstEintragWithFlags(etl,UK_PERSOENLICH);
      if (!ukHasSubTree) {
        eti=ukoi;
        if (ukoi->firstChild())
          delete ukoi->firstChild();
      }
      else {
        bool etiFound=(eti!=NULL);
        if (newUkSubTreeOpened)
        { /*QString qs;
          qs.setNum(firstEintrag);*/
            ukoi->setPixmap(2,emptyPixmap);ukoi->setText(1,"");ukoi->setText(3,"");ukoi->setText(4,"");ukoi->setText(5,"");
            ukoi->setDragEnabled(false);
            ukoi->setDropEnabled(false);
        }
        if (!etiFound) {
          QString qs;
          qs.setNum(idx);
          eti=new KontoTreeItem(ukoi,qs);
          ukoi->setOpen(true);
        }
        ukoi->setText(1,"");
        ukoi->setText(3,"");
        ukoi->setText(4,"");
        ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
        if (bereitschaften.size()>=1) {
          ukoi->setPixmap(2,bereitPixmap);
          ukoi->setText(5,bereitschaften.join("; "));
        }
        else {
          ukoi->setPixmap(2,emptyPixmap);
          ukoi->setText(5,"");
        }
      }

      if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
        eti->setPixmap(2,aktivPixmap);
      else
        eti->setPixmap(2,emptyPixmap);
      eti->setText(5,etiter->second.kommentar);
      eti->setText(1,dd.type());
      eti->setText(3,tc.toString());
      eti->setText(4,tcAbzur.toString());
      //eti->setGray(abtList->checkInState());
      eti->setDragEnabled(true);
      eti->setDropEnabled(true);
      eti->setBgColor(abtList->getBgColor(abt,ko,uko));
      koi->setBgColor(abtList->getBgColor(abt,ko));
      abti->setBgColor(abtList->getBgColor(abt));
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
    }
  }
}

void KontoTreeView::getSumTime(Q3ListViewItem* item, TimeCounter& sum, TimeCounter& sumAbs)
{
  for (Q3ListViewItem* it=item->firstChild(); it!=NULL ; it=it->nextSibling())
  {
    QString s=it->text(3);
    s=s.replace("+","");
    if (s.isEmpty()) {
      getSumTime(it, sum, sumAbs);
    }
    else {
      sum.addTime(TimeCounter::fromString(s));
      QString sabs=it->text(4);
      sabs=sabs.replace("+","");
      sumAbs.addTime(TimeCounter::fromString(sabs));
    }
  }
}

void KontoTreeView::refreshParentSumTime(Q3ListViewItem* item, QString prefix)
{
  Q3ListViewItem* p=item->parent();
  TimeCounter sum, sumAbs;
  getSumTime(p, sum, sumAbs);
  p->setText(3,prefix+sum.toString());
  p->setText(4,prefix+sumAbs.toString());
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

  KontoTreeItem *koi;
  UnterKontoListe* unterkontoliste=&(itKo->second);
  for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
    EintragsListe* eintragsliste=&(ukontPos->second);
    for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {
      refreshItem(abt,ko,ukontPos->first,etPos->first);
    }
  }
}

