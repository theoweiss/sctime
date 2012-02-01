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
#include <vector>
#include <QTreeWidget>
#include <QString>

#include <QKeyEvent>
#include <QEvent>
#include <QApplication>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QTextStream>
#include <QToolTip>

#include "timecounter.h"
#include "globals.h"
#include "descdata.h"
#include "abteilungsliste.h"
#include "kontotreeitem.h"
#include "unterkontoeintrag.h"


/**
 * Erzeugt ein neues Objekt zur Anzeige des Kontobaums. Seine Daten bezieht es aus abtlist.
 */
KontoTreeView::KontoTreeView(QWidget *parent, AbteilungsListe* abtlist, const std::vector<int>& columnwidth): QTreeWidget(parent)
{

  setColumnCount(6);
  QTreeWidgetItem * header = new QTreeWidgetItem;
  header->setText(0, "Konten" );
  header->setText(1,"Typ");
  header->setText(2,"Aktiv");
  header->setText(3,"Zeit");
  header->setText(4, "Abzur.");
  header->setText(5,"Kommentar");
  this->setHeaderItem(header);
  //this->header()->setResizeMode(QHeaderView::Interactive);
  for (std::vector<int>::size_type i=0; i<columnwidth.size(); i++) {
    if( !isColumnHidden(i) ){
      header->setSizeHint(i, QSize(columnwidth.at(i), 20));
    }
  }
  setRootIsDecorated(true);
  viewport()->installEventFilter(this);
  m_showPersoenlicheKontenSummenzeit=false;

  setDragEnabled( true );
  setDropIndicatorShown( true );
  setSortingEnabled( true );
  sortItems(0,Qt::AscendingOrder);
  setContextMenuPolicy(Qt::CustomContextMenu);


#ifndef MACOS
  setSelectionMode(QTreeWidget::NoSelection);
#else
  /* On Mac OS X with NoSelection the TreeView gives no visual feedback, what
   * the currently selected item is. So we have to switch on SingleSelection to
   * avoid user confusion. */
  setSelectionMode(QTreeWidget::SingleSelection);
#endif

  load(abtlist);


  this->topLevelItem(0)->setExpanded(false);

}

void KontoTreeView::keyReleaseEvent(QKeyEvent *event)
{
  keyboardModifier = Qt::NoModifier;
  QTreeWidget::keyReleaseEvent(event);
}

void KontoTreeView::keyPressEvent(QKeyEvent *event)
{
  keyboardModifier = event->modifiers();  
	QTreeWidget::keyPressEvent(event);
}

void KontoTreeView::mousePressEvent(QMouseEvent * event)
{
  //Only to detect in timemainwindow.cpp which mousebutton was clicked.
  currentButton = event->button();

  //Do only Drag n Drop if a time column is selected
  if( currentButton == Qt::LeftButton )//&& currentColumn() == 3 || currentColumn() == 4)
  {
    dragStartPosition = event->pos();
  }

  //Put event to the parentwidget
  QTreeWidget::mousePressEvent(event);
}

void KontoTreeView::mouseMoveEvent(QMouseEvent *  event )
{
  if (!(event->buttons() & Qt::LeftButton))
    return;
  if ((event->pos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance())
    return;

  //Get the current Item
  KontoTreeItem * item=(KontoTreeItem *)(currentItem());
  if( !item ) return;
  if (isEintragsItem(item)) {

      QString top,uko,ko,abt;
      int idx;
      itemInfo(item,top,abt,ko,uko,idx);

      QString data;
      data=top+"|"+abt+"|"+ko+"|"+uko+"|"+data.setNum(idx);

      UnterKontoEintrag eintrag;
      abtList->getEintrag(eintrag,abt,ko,uko,idx);

      QString secondData;
      QTextStream(&secondData) << eintrag.sekunden << "|" << eintrag.sekundenAbzur << "|" << eintrag.kommentar ;

      //std::cout << data.toStdString() << std::endl;
      //std::cout << secondData.toStdString() << std::endl;

      QMimeData *mime = new QMimeData();
      mime->setData(MIMETYPE_SECONDS, secondData.toLocal8Bit());
      mime->setData(MIMETYPE_ACCOUNT, data.toLocal8Bit());

      QDrag *drag = new QDrag(this);
      drag->setMimeData( mime ) ;
      drag->exec(Qt::MoveAction);
  }
}


void KontoTreeView::dropEvent(QDropEvent *event)
{
  if (event->dropAction()!=Qt::MoveAction) {
    return;
  }

  QPoint contpos(event->pos());

  KontoTreeItem * item=(KontoTreeItem *)(itemAt(contpos));
  if (isEintragsItem(item)) {
    QString top,uko,ko,abt;
    int idx;

    QString data;
    data=data.fromLocal8Bit(event->encodedData(MIMETYPE_ACCOUNT));
    QStringList datlist=data.split("|");

    UnterKontoEintrag eintrag;
    abtList->getEintrag(eintrag,datlist[1],datlist[2],datlist[3],datlist[4].toInt());

    int deltasek=eintrag.sekunden;
    int deltasekabzur=eintrag.sekundenAbzur;
    QString kommentar=eintrag.kommentar;

    // clear old entry
    abtList->setSekunden(datlist[1],datlist[2],datlist[3],datlist[4].toInt(),0);
    abtList->setSekundenAbzur(datlist[1],datlist[2],datlist[3],datlist[4].toInt(),0);
    //Move Kommentar on Shiftkey
    if( keyboardModifier == Qt::ShiftModifier ){
      abtList->setKommentar(datlist[1],datlist[2],datlist[3],datlist[4].toInt(), "");
    }
    refreshItem(datlist[1],datlist[2],datlist[3],datlist[4].toInt());



    // add delta to new entry
    itemInfo(item,top,abt,ko,uko,idx);
    abtList->getEintrag(eintrag,abt,ko,uko,idx);
    abtList->setSekunden(abt,ko,uko,idx,deltasek+eintrag.sekunden);
    abtList->setSekundenAbzur(abt,ko,uko,idx,deltasekabzur+eintrag.sekundenAbzur);
    //Move Kommentar on Shiftkey
    if( keyboardModifier == Qt::ShiftModifier ){
      abtList->setKommentar(abt,ko,uko,idx,kommentar);
    }
    refreshItem(abt,ko,uko,idx);

    event->accept();
  }

}


void KontoTreeView::dragEnterEvent(QDragEnterEvent *event)
{
  if ((event->pos() - dragStartPosition).manhattanLength()
          < QApplication::startDragDistance())
         return;
  if (event->mimeData()->hasFormat(MIMETYPE_ACCOUNT) &&
      event->mimeData()->hasFormat(MIMETYPE_SECONDS) &&
      event->dropAction()==Qt::MoveAction) {
    QPoint contpos(event->pos());
    KontoTreeItem * item=(KontoTreeItem *)(itemAt(contpos));
    if (isEintragsItem(item)) {
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
}

void KontoTreeView::dragMoveEvent(QDragMoveEvent *event)
{
  if ((event->pos() - dragStartPosition).manhattanLength()
          < QApplication::startDragDistance())
         return;

  if (event->mimeData()->hasFormat(MIMETYPE_ACCOUNT) &&
      event->mimeData()->hasFormat(MIMETYPE_SECONDS) &&
      event->dropAction()==Qt::MoveAction) {
    QPoint contpos(event->pos());
    KontoTreeItem * item=(KontoTreeItem *)(itemAt(contpos));
    if (isEintragsItem(item)) {
      event->accept();
    }
    else {
       event->ignore();
    }

  }
}


void KontoTreeView::getColumnWidthList(std::vector<int>& columnwidth)
{
    columnwidth.clear();
    for (int i=0; i<columnCount(); i++) {
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

  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(0)!=tops); topi=(KontoTreeItem*)(topi->nextSibling())) ;
  if (topi==NULL) return false;

  for (abti=(KontoTreeItem*)(topi->child(0)); (abti!=NULL)&&(abti->text(0)!=abts); abti=(KontoTreeItem*)(abti->nextSibling())) ;
  if (abti==NULL) return false;

  for (koi=(KontoTreeItem*)(abti->child(0)); (koi!=NULL)&&(koi->text(0)!=kos); koi=(KontoTreeItem*)(koi->nextSibling())) ;
  if (koi==NULL) return false;

  for (ukoi=(KontoTreeItem*)(koi->child(0)); (ukoi!=NULL)&&(ukoi->text(0)!=ukos); ukoi=(KontoTreeItem*)(ukoi->nextSibling())) ;
  if (ukoi==NULL) return false;

  for (eti=(KontoTreeItem*)(ukoi->child(0)); (eti!=NULL)&&(eti->text(0).toInt()!=idx); eti=(KontoTreeItem*)(eti->nextSibling())) ;
  //if ((eti==NULL)&&(idx!=0)) return false;

  return true;
}

/* Liefert im Gegensatz zu sucheItem nur ein Item zurueck, und zwar das des angegebenen kontos*/
KontoTreeItem* KontoTreeView::sucheKontoItem(const QString& tops, const QString& abts, const QString& kos)
{
  KontoTreeItem *topi,*abti,*koi;

  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(0)!=tops); topi=(KontoTreeItem*)topi->nextSibling()) ;
  if (topi==NULL) return NULL;

  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL)&&(abti->text(0)!=abts); abti=(KontoTreeItem*)abti->nextSibling()) ;
  if (abti==NULL) return NULL;

  for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL)&&(koi->text(0)!=kos); koi=(KontoTreeItem*)koi->nextSibling()) ;

  return koi;
}

KontoTreeItem* KontoTreeView::sucheUnterKontoItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos)
{
  KontoTreeItem *topi,*abti,*koi,*ukoi;

  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(0)!=tops); topi=(KontoTreeItem*)topi->nextSibling()) ;
  if (topi==NULL) return NULL;

  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL)&&(abti->text(0)!=abts); abti=(KontoTreeItem*)abti->nextSibling()) ;
  if (abti==NULL) return NULL;

  for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL)&&(koi->text(0)!=kos); koi=(KontoTreeItem*)koi->nextSibling()) ;
  if (koi==NULL) return NULL;

  for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL)&&(ukoi->text(0)!=ukos); ukoi=(KontoTreeItem*)ukoi->nextSibling()) ;
  return ukoi;
}

KontoTreeItem* KontoTreeView::sucheKommentarItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos, const QString& koms)
{
  KontoTreeItem *topi,*abti,*koi,*ukoi;

  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(0)!=tops); topi=(KontoTreeItem*)topi->nextSibling()) ;
  if (topi==NULL) return NULL;

  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL)&&(abti->text(0)!=abts); abti=(KontoTreeItem*)abti->nextSibling()) ;
  if (abti==NULL) return NULL;

  for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL)&&(koi->text(0)!=kos); koi=(KontoTreeItem*)koi->nextSibling()) ;
  if (koi==NULL) return NULL;

  for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL)&&(ukoi->text(0)!=ukos); ukoi=(KontoTreeItem*)ukoi->nextSibling()) ;
  return ukoi;
}

int KontoTreeView::getItemDepth( QTreeWidgetItem* item )
{
  int depth = -1;

  while(item!=0){
    depth++;
    item = item->parent();
  }
  return depth;
}
/**
 * Liefert in tops, abts, kos, ukos, id den String bzw Wert des Toplevel-,Abteilungs-,...-Items
 */
void KontoTreeView::itemInfo(QTreeWidgetItem* item,QString& tops, QString& abts, QString& kos, QString& ukos, int& idx)
{
  tops=abts=kos=ukos="";
  idx=-1;
  int depth = getItemDepth( item );

  for (int d=depth; d>=0; d--) {
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
    if (abtList->findUnterKonto(ituk,ukl,abts,kos,ukos)){
       idx=ituk->second.begin()->first;
    }
  }

  //updateColumnWidth();
}

void KontoTreeView::updateColumnWidth()
{
  for(int i=0; i<columnCount(); i++){
      resizeColumnToContents(i);
  }
}


/** Versieht geschlossene, persoenliche Eintraege mit dem Flag IS_CLOSED */
void KontoTreeView::flagClosedPersoenlicheItems()
{
  KontoTreeItem *topi, *abti, *koi, *ukoi;

  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(0)!=PERSOENLICHE_KONTEN_STRING); topi=(KontoTreeItem*)topi->nextSibling()) ;
  if (topi==NULL) return;

  int fm;
  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL); abti=(KontoTreeItem*)abti->nextSibling()) {
    if (abti->isExpanded()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
    abtList->setAbteilungFlags(abti->text(0),IS_CLOSED,fm);

    for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL); koi=(KontoTreeItem*)koi->nextSibling()) {
      if (koi->isExpanded()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
      abtList->setKontoFlags(abti->text(0),koi->text(0),IS_CLOSED,fm);

      for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL); ukoi=(KontoTreeItem*)ukoi->nextSibling()) {
        if (ukoi->isExpanded()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
          abtList->setUnterKontoFlags(abti->text(0),koi->text(0),ukoi->text(0),IS_CLOSED,fm);
      }
    }
  }
}

/** Schliesst persoenliche Eintraege mit dem Flag IS_CLOSED */
void KontoTreeView::closeFlaggedPersoenlicheItems()
{
  KontoTreeItem *topi, *abti, *koi, *ukoi;


  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(0)!=PERSOENLICHE_KONTEN_STRING); topi=(KontoTreeItem*)topi->nextSibling());
  if (topi==NULL) {
    return;
  }
  else
  {
    topi->setExpanded(true);
  }
  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL); abti=(KontoTreeItem*)abti->nextSibling()) {
    if (abtList->getAbteilungFlags(abti->text(0))&IS_CLOSED) {
      abti->setExpanded(false);
      continue;
    }
    for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL); koi=(KontoTreeItem*)koi->nextSibling()) {
      if (abtList->getKontoFlags(abti->text(0),koi->text(0))&IS_CLOSED) {
        koi->setExpanded(false);
        continue;
      }
      for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL); ukoi=(KontoTreeItem*)ukoi->nextSibling()) {
        if ((abtList->getUnterKontoFlags(abti->text(0),koi->text(0),ukoi->text(0))&IS_CLOSED)&&(ukoi->child(0)))
          ukoi->setExpanded(false);
        }
    }
  }
}

/**
 *  True, falls das uebergebene Item ein Item ist, das einen Eintrag repraesentiert.
 */

bool KontoTreeView::isEintragsItem(QTreeWidgetItem* item)
{
  if (!item) return false;
  int d = -1;
  d=getItemDepth( item );
  return ((d==4)||((d==3)&&(item->childCount()==0)));
}

/**
 *  True, falls das uebergebene Item ein Item ist, das ein Unterkonto repraesentiert.
 */

bool KontoTreeView::isUnterkontoItem(QTreeWidgetItem* item)
{
    if (!item) return false;
    int d= -1;
    d=getItemDepth( item );
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
     topi->setExpanded(true);
     abti->setExpanded(true);
     koi->setExpanded(true);
     ukoi->setExpanded(true);
     this->setCurrentItem(ukoi);
  }
}

/** Laedt die uebergebene Abteilungsliste in den Kontobaum
 */
void KontoTreeView::load(AbteilungsListe* abtlist)
{
  abtList=abtlist;

  KontoTreeItem *next, *topi;

  topi=(KontoTreeItem*)topLevelItem(0);
  while (topi!=NULL) {
    next=(KontoTreeItem*)topi->nextSibling();
    takeTopLevelItem(indexOfTopLevelItem(topi));
    topi=next;
  }

  KontoTreeItem* allekonten=new KontoTreeItem(this);
  allekonten->setText(0, ALLE_KONTEN_STRING);
  KontoTreeItem* perskonten=new KontoTreeItem(this);
  perskonten->setText(0, PERSOENLICHE_KONTEN_STRING);

  this->addTopLevelItem(allekonten);
  this->addTopLevelItem(perskonten);

  allekonten->setExpanded(false);
  
  perskonten->setTextAlignment(0, Qt::AlignLeft);
  if (abtList) {
    AbteilungsListe::iterator abtPos;

    for (abtPos=abtList->begin(); abtPos!=abtList->end(); ++abtPos) {
      KontoTreeItem* abteilungsitem=new KontoTreeItem( allekonten );
      abteilungsitem->setText(0, abtPos->first);
      abteilungsitem->setBgColor(abtList->getBgColor(abtPos->first));
      KontoListe* kontoliste=&(abtPos->second);
      for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
        KontoTreeItem* kontoitem= new KontoTreeItem( abteilungsitem);
        kontoitem->setText(0, kontPos->first);
        kontoitem->setBgColor(abtList->getBgColor(abtPos->first,kontPos->first));        
        UnterKontoListe* unterkontoliste=&(kontPos->second);
        for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
          if ((ukontPos->second.getFlags()&IS_DISABLED)!=0) {
              continue; // Deaktivierte Unterkonten nicht anzeigen
          }
          EintragsListe* eintragsliste=&(ukontPos->second);
          DescData dd=eintragsliste->description();
          for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {
            TimeCounter tc(etPos->second.sekunden), tcAbzur(etPos->second.sekundenAbzur);
            if (etPos==eintragsliste->begin()) {
                //KontoTreeItem* newItem=new KontoTreeItem( kontoitem, ukontPos->first, dd.type(), "", tc.toString(),
                                                      //tcAbzur.toString() , etPos->second.kommentar);
                KontoTreeItem* newItem=new KontoTreeItem( kontoitem);
                newItem->setText( 0, ukontPos->first);
                newItem->setText( 1, dd.type());
                newItem->setText( 2, "");
                newItem->setText( 3, tc.toString());
                newItem->setText( 4, tcAbzur.toString());
                newItem->setText( 5, etPos->second.kommentar);
                newItem->setGray();
                //newItem->setDragEnabled(true);
                //newItem->setDropEnabled(true);
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

  updateColumnWidth();
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
            QString beschreibung=abtList->getDescription(abt,ko,uko).description().simplified();
            if (beschreibung!="") {
               QToolTip::showText(qh->globalPos(),beschreibung,this);
                qh->accept();
                return true;
            }
        }
  }
  return QTreeWidget::eventFilter(obj,e);
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
  //std::cout << "refreshItem()" << "\n abt="<<abt.toStdString()<<" ko="<<ko.toStdString()<<" uko="<<uko.toStdString()<<" idx="<<idx<<std::endl;
  EintragsListe::iterator etiter;
  EintragsListe* etl;
  bool ukHasSubTree;
  bool newUkSubTreeOpened=false;
  bool etiFound = false;
  if (!abtList->findEintrag(etiter,etl,abt,ko,uko,idx)) return;

  QStringList bereitschaften=etl->getBereitschaft();

  KontoTreeItem *topi,*abti,*koi,*ukoi,*eti;
  bool isExpandedAlleKonten, isExpandedPersonKonten;

  for(int i=0; i<topLevelItemCount(); i++)
  {
    if(topLevelItem(i)->text(0)==ALLE_KONTEN_STRING)
    {
      isExpandedAlleKonten = topLevelItem(i)->isExpanded();
    }
    else
    {
      isExpandedPersonKonten = topLevelItem(i)->isExpanded();
    }
  }

  bool itemFound=(sucheItem(ALLE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti));

  if ((koi)&&(!ukoi)) {
    ukoi=new KontoTreeItem(koi,uko);
    itemFound=true;
  }

  koi->setBgColor(abtList->getBgColor(abt,ko));
  abti->setBgColor(abtList->getBgColor(abt));

  ukoi->setBold((etl->getFlags())&UK_PERSOENLICH);
  ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
  ukoi->setGray();
  if ((etl->getFlags()&IS_DISABLED)!=0)
    return; // Deaktivierte Unterkonten nicht anfassen

  if (itemFound) {
    etiFound=(eti!=NULL);
    ukHasSubTree=((etl->size()+bereitschaften.size())>1);
    DescData dd=etl->description();
    if (!ukHasSubTree)
    {
      if (ukoi->child(0))
      {

        ukoi->removeChild( ukoi->child(0) );
        //delete ukoi->child(0);
      }
      eti=ukoi;
    }
    else {
      //if (ukoi->child(0)==NULL) // Wir brauchen einen Unterbaum, der aber noch leer ist
      if(ukoi->childCount() == 0 )
      {

        QString qs;
        qs.setNum(etl->begin()->first);
        eti=new KontoTreeItem(ukoi,qs);
        //std::cout << "ukoi->child(0)=" << ukoi->child(0)->text(0).toStdString()<<std::endl;
        ukoi->setIcon(2,QIcon());
        ukoi->setText(1,"");ukoi->setText(3,"");ukoi->setText(4,"");ukoi->setText(5,"");
        ukoi->setGray();
        //ukoi->setDragEnabled(false);
        //ukoi->setDropEnabled(false);
        //std::cout << "abt=" << abt.toStdString() <<" ko="<<ko.toStdString()<<" uko="<<uko.toStdString()<<" etl="<< etl->begin()->first << std::endl;
        refreshItem(abt,ko,uko,etl->begin()->first);
        if (etl->begin()->first==idx){
            //std::cout<< "etl->begin()->first=" << etl->begin()->first << " == idx= " << idx << std::endl;
           etiFound=true;
         }
        newUkSubTreeOpened=true;
      }
      if (!etiFound) {
        QString qs;
        qs.setNum(idx);
        eti=new KontoTreeItem(ukoi,qs);
        //ukoi->setExpanded(true);
      }
      ukoi->setText(1,"");
      ukoi->setText(3,"");
      ukoi->setText(4,"");
      ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
      ukoi->setGray();
      if (bereitschaften.size()>=1) {
        ukoi->setIcon(2,QIcon(":/hi16_action_stamp"));
        ukoi->setText(5,bereitschaften.join("; "));
      }
      else {
        ukoi->setIcon(2,QIcon());
        ukoi->setText(5,"");
      }
    }

    eti->setText(5,etiter->second.kommentar);
    TimeCounter tc(etiter->second.sekunden), tcAbzur(etiter->second.sekundenAbzur);
    eti->setText(1,dd.type());
    eti->setText(3,tc.toString());
    eti->setText(4,tcAbzur.toString());
    //eti->setDragEnabled(true);
    //eti->setDropEnabled(true);
    eti->setGray();
    eti->setBgColor(abtList->getBgColor(abt,ko,uko));
    //eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));

    eti->setBold((etiter->second.flags)&UK_PERSOENLICH);

    if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
      {
        eti->setIcon(2,QIcon(":/hi16_action_apply"));
        //setCurrentItem(eti);
      }
    else{
      eti->setIcon(2,QIcon());
    }
    refreshParentSumTime(ukoi,"+");
    refreshParentSumTime(koi,"++");
    bool inPersKontenGefunden=sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti);

    if ((inPersKontenGefunden)&&(!((etiter->second.flags)&UK_PERSOENLICH))) {
      if (eti!=NULL)
      {
          //delete eti; //Qt3
          ukoi->removeChild( eti );
          //delete eti;
      }
      if (sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti)) { // Wegen moeglichen Seiteneffekten von delete
        if (!ukoi->child(0)) {
          //delete ukoi;
          koi->removeChild( ukoi );

          if (!koi->child(0)) {
            //delete koi;
            abti->removeChild( koi );

            if (!abti->child(0)) {
              //delete abti;
              topi->removeChild( abti );

            }
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
      //eti->setDragEnabled(true);
      //eti->setDropEnabled(true);
      topi->setExpanded(true); abti->setExpanded(true); koi->setExpanded(true); ukoi->setExpanded(true);
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
      //eti->setGray(abtList->checkInState());
      if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
        eti->setIcon(2,QIcon(":/hi16_action_apply"));
      else
        eti->setIcon(2,QIcon());
      if (m_showPersoenlicheKontenSummenzeit) {
        refreshParentSumTime(ukoi,"+");
        refreshParentSumTime(koi,"++");
      }
      QColor color=abtList->getBgColor(abt,ko,uko);
      eti->setBgColor(color);
      eti->setGray();
      ukoi->setBgColor(color);
      ukoi->setGray();
      koi->setBgColor(abtList->getBgColor(abt,ko));
      abti->setBgColor(abtList->getBgColor(abt));
      koi->setGray();
      abti->setGray();
    }
    if ((inPersKontenGefunden)&&((etiter->second.flags)&UK_PERSOENLICH)) {
      firstEintragWithFlags(etl,UK_PERSOENLICH);
      if (!ukHasSubTree) {
        eti=ukoi;
        if (ukoi->child(0))
        {
          //delete ukoi->child(0); //Qt3
          ukoi->removeChild( ukoi->child(0) );
        }
      }
      else {
        etiFound=(eti!=NULL);
        if (newUkSubTreeOpened)
        { /*QString qs;
          qs.setNum(firstEintrag);*/
            ukoi->setIcon(2,QIcon());
            ukoi->setGray();
            ukoi->setText(1,"");ukoi->setText(3,"");ukoi->setText(4,"");ukoi->setText(5,"");
            //ukoi->setDragEnabled(false);
            //ukoi->setDropEnabled(false);
        }
        if (!etiFound) {
          QString qs;
          qs.setNum(idx);
          eti=new KontoTreeItem(ukoi,qs);
          ukoi->setExpanded(true);
        }
        ukoi->setText(1,"");
        ukoi->setText(3,"");
        ukoi->setText(4,"");
        ukoi->setGray();
        ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
        if (bereitschaften.size()>=1) {
          ukoi->setIcon(2,QIcon(":/hi16_action_stamp"));
          ukoi->setText(5,bereitschaften.join("; "));
        }
        else {
          ukoi->setIcon(2,QIcon());
          ukoi->setText(5,"");
        }
      }

      if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
        eti->setIcon(2,QIcon(":/hi16_action_apply"));
      else
        eti->setIcon(2,QIcon());
      eti->setText(5,etiter->second.kommentar);
      eti->setText(1,dd.type());
      eti->setText(2, "");
      eti->setText(3,tc.toString());
      eti->setText(4,tcAbzur.toString());
      eti->setGray();

      //eti->setGray(abtList->checkInState());
      //eti->setDragEnabled(true);
      //eti->setDropEnabled(true);
      eti->setBgColor(abtList->getBgColor(abt,ko,uko));
      koi->setBgColor(abtList->getBgColor(abt,ko));
      abti->setBgColor(abtList->getBgColor(abt));
      if (m_showPersoenlicheKontenSummenzeit) {
        refreshParentSumTime(ukoi,"+");
        refreshParentSumTime(koi,"++");
      }
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
    }
  }

  for(int i=0; i<topLevelItemCount(); i++)
  {
    if(topLevelItem(i)->text(0)==ALLE_KONTEN_STRING)
    {
      topLevelItem(i)->setExpanded(isExpandedAlleKonten);
    }
    else
    {
      topLevelItem(i)->setExpanded(isExpandedPersonKonten);
    }
  }

}

void KontoTreeView::getSumTime(QTreeWidgetItem* item, TimeCounter& sum, TimeCounter& sumAbs)
{
  int childCount = item->childCount();
  //foreach (const QTreeWidgetItem it, item->takeChildren())
  for( int i=0; i<childCount; i++)
  {
    QTreeWidgetItem* it = item->child( i );
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

void KontoTreeView::refreshParentSumTime(QTreeWidgetItem* item, QString prefix)
{
  QTreeWidgetItem* p=item->parent();
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

void KontoTreeView::showPersoenlicheKontenSummenzeit(bool show)
{
    if (m_showPersoenlicheKontenSummenzeit!=show)
    {
      m_showPersoenlicheKontenSummenzeit=show;

      load(abtList);
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

Qt::MouseButton KontoTreeView::getCurrentButton()
{
  return currentButton;
}
