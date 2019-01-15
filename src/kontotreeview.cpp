/*
    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
#include <QMimeData>
#include <QDrag>

#include "timecounter.h"
#include "globals.h"
#include "descdata.h"
#include "abteilungsliste.h"
#include "kontotreeitem.h"
#include "unterkontoeintrag.h"


/**
 * Erzeugt ein neues Objekt zur Anzeige des Kontobaums. Seine Daten bezieht es aus abtlist.
 */
KontoTreeView::KontoTreeView(QWidget *parent, AbteilungsListe* abtlist, const std::vector<int>& columnwidth, SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode): QTreeWidget(parent)
{
  this->displaymode=displaymode;
  setColumnCount(7);
  QTreeWidgetItem * header = new QTreeWidgetItem;
  header->setText(KontoTreeItem::COL_ACCOUNTS, tr("Accounts") );
  header->setText(KontoTreeItem::COL_TYPE, tr("Type"));
  header->setText(KontoTreeItem::COL_PSP, tr("PSP"));
  header->setText(KontoTreeItem::COL_ACTIVE, tr("Active"));
  header->setText(KontoTreeItem::COL_TIME, tr("Time"));
  header->setText(KontoTreeItem::COL_ACCOUNTABLE, tr("Accountable"));
  header->setText(KontoTreeItem::COL_COMMENT, tr("Comment"));
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

  // make sure we see all right mouse button events
  setContextMenuPolicy(Qt::PreventContextMenu);

#ifndef Q_OS_MAC
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
  QPoint pos = event->pos();

  if (event->button() == Qt::LeftButton)
    dragStartPosition = pos;

  // signal itemClicked() of QTreeWidget only fires on left button clicks
  // (based on QAbstractItemView's clicked() signal). Track right clicks as
  // well to implement our own itemRightClicked() signal
  if (event->button() == Qt::RightButton)
    rightPressedIndex = indexAt(pos);

  //Put event to the parentwidget
  QTreeWidget::mousePressEvent(event);
}

void KontoTreeView::mouseReleaseEvent(QMouseEvent *event) {
  QTreeWidget::mouseReleaseEvent(event);

  // emit itemRightClicked if we saw a right button press matching this release
  QModelIndex index = indexAt(event->pos());
  if (event->button() == Qt::RightButton &&
		  index == rightPressedIndex &&
		  index.isValid())
    emit itemRightClicked(itemFromIndex(index));
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
    data=data.fromLocal8Bit(event->mimeData()->data(MIMETYPE_ACCOUNT));
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
  KontoTreeItem *koi,*ukoi;

  koi = sucheKontoItem(tops, abts, kos);
  if (koi==NULL) return NULL;

  for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL)&&(ukoi->text(0)!=ukos); ukoi=(KontoTreeItem*)ukoi->nextSibling()) ;
  return ukoi;
}

KontoTreeItem* KontoTreeView::sucheKommentarItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos, const QString& koms)
{
  KontoTreeItem *ukoi,*komi;

  ukoi = sucheUnterKontoItem(tops, abts, kos, ukos);
  if (ukoi==NULL) return NULL;

  for (komi=(KontoTreeItem*)ukoi->child(0); (komi!=NULL)&&(komi->text(5)!=koms); komi=(KontoTreeItem*)komi->nextSibling());
  return komi;
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
      case 0: tops=item->text(KontoTreeItem::COL_ACCOUNTS); break;
      case 1: abts=item->text(KontoTreeItem::COL_ACCOUNTS); break;
      case 2: kos=item->text(KontoTreeItem::COL_ACCOUNTS); break;
      case 3: ukos=item->text(KontoTreeItem::COL_ACCOUNTS); break;
      case 4: idx=item->text(KontoTreeItem::COL_ACCOUNTS).toInt(); break;
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

  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(KontoTreeItem::COL_ACCOUNTS)!=PERSOENLICHE_KONTEN_STRING); topi=(KontoTreeItem*)topi->nextSibling()) ;
  if (topi==NULL) return;

  int fm;
  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL); abti=(KontoTreeItem*)abti->nextSibling()) {
    if (abti->isExpanded()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
    abtList->setAbteilungFlags(abti->text(KontoTreeItem::COL_ACCOUNTS),IS_CLOSED,fm);

    for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL); koi=(KontoTreeItem*)koi->nextSibling()) {
      if (koi->isExpanded()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
      abtList->setKontoFlags(abti->text(KontoTreeItem::COL_ACCOUNTS),koi->text(KontoTreeItem::COL_ACCOUNTS),IS_CLOSED,fm);

      for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL); ukoi=(KontoTreeItem*)ukoi->nextSibling()) {
        if (ukoi->isExpanded()) fm=FLAG_MODE_NAND; else fm=FLAG_MODE_OR;
          abtList->setUnterKontoFlags(abti->text(KontoTreeItem::COL_ACCOUNTS),koi->text(KontoTreeItem::COL_ACCOUNTS),ukoi->text(KontoTreeItem::COL_ACCOUNTS),IS_CLOSED,fm);
      }
    }
  }
}

/** Schliesst persoenliche Eintraege mit dem Flag IS_CLOSED */
void KontoTreeView::closeFlaggedPersoenlicheItems()
{
  KontoTreeItem *topi, *abti, *koi, *ukoi;


  for (topi=(KontoTreeItem*)topLevelItem(0); (topi!=NULL)&&(topi->text(KontoTreeItem::COL_ACCOUNTS)!=PERSOENLICHE_KONTEN_STRING); topi=(KontoTreeItem*)topi->nextSibling()) ;
  if (topi==NULL) {
    return;
  }
  else
  {
    topi->setExpanded(true);
  }
  for (abti=(KontoTreeItem*)topi->child(0); (abti!=NULL); abti=(KontoTreeItem*)abti->nextSibling()) {
    if (abtList->getAbteilungFlags(abti->text(KontoTreeItem::COL_ACCOUNTS))&IS_CLOSED) {
      abti->setExpanded(false);
      continue;
    }
    for (koi=(KontoTreeItem*)abti->child(0); (koi!=NULL); koi=(KontoTreeItem*)koi->nextSibling()) {
      if (abtList->getKontoFlags(abti->text(KontoTreeItem::COL_ACCOUNTS),koi->text(KontoTreeItem::COL_ACCOUNTS))&IS_CLOSED) {
        koi->setExpanded(false);
        continue;
      }
      for (ukoi=(KontoTreeItem*)koi->child(0); (ukoi!=NULL); ukoi=(KontoTreeItem*)ukoi->nextSibling()) {
        if ((abtList->getUnterKontoFlags(abti->text(KontoTreeItem::COL_ACCOUNTS),koi->text(KontoTreeItem::COL_ACCOUNTS),ukoi->text(KontoTreeItem::COL_ACCOUNTS))&IS_CLOSED)&&(ukoi->child(0)))
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

  KontoTreeItem* allekonten=new KontoTreeItem(this, displaymode);
  allekonten->setText(KontoTreeItem::COL_ACCOUNTS, ALLE_KONTEN_STRING);
  KontoTreeItem* perskonten=new KontoTreeItem(this, displaymode);
  perskonten->setText(KontoTreeItem::COL_ACCOUNTS, PERSOENLICHE_KONTEN_STRING);

  this->addTopLevelItem(allekonten);
  this->addTopLevelItem(perskonten);

  allekonten->setExpanded(false);
  
  perskonten->setTextAlignment(KontoTreeItem::COL_ACCOUNTS, Qt::AlignLeft);
  if (abtList) {
    AbteilungsListe::iterator abtPos;

    for (abtPos=abtList->begin(); abtPos!=abtList->end(); ++abtPos) {
      KontoTreeItem* abteilungsitem=new KontoTreeItem(allekonten, displaymode);
      abteilungsitem->setText(KontoTreeItem::COL_ACCOUNTS, abtPos->first);
      abteilungsitem->setBgColor(abtList->getBgColor(abtPos->first));
      KontoListe* kontoliste=&(abtPos->second);
      for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
        KontoTreeItem* kontoitem= new KontoTreeItem(abteilungsitem, displaymode);
        kontoitem->setText(KontoTreeItem::COL_ACCOUNTS, kontPos->first);
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
                KontoTreeItem* newItem=new KontoTreeItem(kontoitem, displaymode);
                newItem->setText( KontoTreeItem::COL_ACCOUNTS, ukontPos->first);
                newItem->setText( KontoTreeItem::COL_TYPE, dd.type());
                newItem->setText( KontoTreeItem::COL_PSP, dd.pspElem());
                newItem->setText( KontoTreeItem::COL_ACTIVE, "");
                newItem->setText( KontoTreeItem::COL_TIME, tc.toString());
                newItem->setText( KontoTreeItem::COL_ACCOUNTABLE, tcAbzur.toString());
                newItem->setText( KontoTreeItem::COL_COMMENT, etPos->second.kommentar);
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
            QToolTip::showText(qh->globalPos(),beschreibung,this);
            qh->accept();
            return true;
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

void KontoTreeView::refreshComment(const QString& comment, KontoTreeItem* item, EintragsListe* etl)
{
   item->setText(KontoTreeItem::COL_COMMENT,comment);
   QVector<DefaultComment>* defaultCommentList = etl->getDefaultCommentList();
   item->setHasSelectableMicroAccounts(!defaultCommentList->isEmpty());

   if (comment.isEmpty())
   {
      item->setMicroAccount(false);
      return;
   }
   QVector<DefaultComment>::iterator dclIt;
   for (dclIt = defaultCommentList->begin(); dclIt != defaultCommentList->end(); ++dclIt ) {
     QString matext=dclIt->getText();
     if ((comment.startsWith(matext))&&(!matext.isEmpty()))
     {
       item->setMicroAccount(true);
       return;
     }
   }
   item->setMicroAccount(false);
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
  QSet<QString> srlist=etiter->second.getAchievedSpecialRemunSet();

  KontoTreeItem *topi,*abti,*koi,*ukoi,*eti;
  bool isExpandedAlleKonten = false, isExpandedPersonKonten = false;

  for(int i=0; i<topLevelItemCount(); i++)
  {
    if(topLevelItem(i)->text(KontoTreeItem::COL_ACCOUNTS)==ALLE_KONTEN_STRING)
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
    ukoi=new KontoTreeItem(koi, displaymode, uko);
    itemFound=true;
  }

  koi->setBgColor(abtList->getBgColor(abt,ko));
  abti->setBgColor(abtList->getBgColor(abt));

  ukoi->setBoldAccount((etl->getFlags())&UK_PERSOENLICH);
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
        QVector<DefaultComment>* defaultCommentList = etl->getDefaultCommentList();
        ukoi->setHasSelectableMicroAccounts(!defaultCommentList->isEmpty());
        eti=new KontoTreeItem(ukoi, displaymode, qs);
        //std::cout << "ukoi->child(0)=" << ukoi->child(0)->text(0).toStdString()<<std::endl;
        ukoi->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
        ukoi->setText(KontoTreeItem::COL_PSP,"");
        ukoi->setText(KontoTreeItem::COL_TYPE,"");
        ukoi->setText(KontoTreeItem::COL_TIME,"");
        ukoi->setText(KontoTreeItem::COL_ACCOUNTABLE,"");
        ukoi->setText(KontoTreeItem::COL_COMMENT,"");
        ukoi->setMicroAccount(false);
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
        eti=new KontoTreeItem(ukoi, displaymode, qs);
        //ukoi->setExpanded(true);
      }
      ukoi->setText(KontoTreeItem::COL_TYPE,"");
      ukoi->setText(KontoTreeItem::COL_PSP,"");
      ukoi->setText(KontoTreeItem::COL_TIME,"");
      ukoi->setText(KontoTreeItem::COL_ACCOUNTABLE,"");
      ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
      ukoi->setGray();
      if (bereitschaften.size()>=1) {
        ukoi->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_action_stamp"));
        ukoi->setText(KontoTreeItem::COL_COMMENT,bereitschaften.join("; "));
      }
      else {
        ukoi->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
        ukoi->setText(KontoTreeItem::COL_COMMENT,"");
        QVector<DefaultComment>* defaultCommentList = etl->getDefaultCommentList();
        ukoi->setHasSelectableMicroAccounts(!defaultCommentList->isEmpty());
        ukoi->setMicroAccount(false);
      }
    }

    QString comment=etiter->second.kommentar;
    refreshComment(comment,eti,etl);
    TimeCounter tc(etiter->second.sekunden), tcAbzur(etiter->second.sekundenAbzur);
    eti->setText(KontoTreeItem::COL_TYPE,dd.type());
    eti->setText(KontoTreeItem::COL_PSP,dd.pspElem());
    eti->setText(KontoTreeItem::COL_TIME,tc.toString());
    eti->setText(KontoTreeItem::COL_ACCOUNTABLE,tcAbzur.toString());
    //eti->setDragEnabled(true);
    //eti->setDropEnabled(true);
    eti->setGray();
    eti->setBgColor(abtList->getBgColor(abt,ko,uko));
    //eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));

    eti->setBoldAccount((etiter->second.flags)&UK_PERSOENLICH);

    if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
      {
        if (srlist.isEmpty()) {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_action_apply"));
        } else {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_moon_apply"));
        }
          
        //setCurrentItem(eti);
      }
    else{
      if (srlist.isEmpty()) {
        eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
      } else {
        eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_moon"));
      }
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
      if (!topi) topi=new KontoTreeItem(this, displaymode, PERSOENLICHE_KONTEN_STRING);
      if (!abti) abti=new KontoTreeItem(topi,displaymode, abt);
      if (!koi) koi=new KontoTreeItem(abti, displaymode, ko);
      if (!ukoi) {
        if (!ukHasSubTree) {
            eti=new KontoTreeItem(koi, displaymode, uko);
            ukoi=eti;
        }
        else
          ukoi=new KontoTreeItem(koi, displaymode, uko);
      }
      if (!eti) {
            eti=new KontoTreeItem(ukoi, displaymode, QString().setNum(idx));
      }
      eti->setText(KontoTreeItem::COL_TYPE,dd.type());
      eti->setText(KontoTreeItem::COL_PSP,dd.pspElem());
      eti->setText(KontoTreeItem::COL_ACTIVE, "");
      eti->setText(KontoTreeItem::COL_TIME,tc.toString());
      eti->setText(KontoTreeItem::COL_ACCOUNTABLE, tcAbzur.toString());
      QString comment=etiter->second.kommentar;
      refreshComment(comment,eti,etl);
      //eti->setDragEnabled(true);
      //eti->setDropEnabled(true);
      topi->setExpanded(true); abti->setExpanded(true); koi->setExpanded(true); ukoi->setExpanded(true);
     // eti->setBold((etiter->second.kommentar!="")||(etiter->second.sekunden!=0)||(etiter->second.sekundenAbzur!=0));
      //eti->setGray(abtList->checkInState());
      if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate())) {
        if (srlist.isEmpty()) {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_action_apply"));
        } else {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_moon_apply"));
        }
      }
      else {
        if (srlist.isEmpty()) {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
        } else {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_moon"));
        }
      }
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
        QVector<DefaultComment>* defaultCommentList = etl->getDefaultCommentList();
        ukoi->setHasSelectableMicroAccounts(!defaultCommentList->isEmpty());
        if (newUkSubTreeOpened)
        { 
            ukoi->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
            ukoi->setGray();
            ukoi->setText(KontoTreeItem::COL_TYPE,"");
            ukoi->setText(KontoTreeItem::COL_PSP,"");
            ukoi->setText(KontoTreeItem::COL_TIME,"");
            ukoi->setText(KontoTreeItem::COL_ACCOUNTABLE,"");
            ukoi->setText(KontoTreeItem::COL_COMMENT,"");
            ukoi->setMicroAccount(false);
            //ukoi->setDragEnabled(false);
            //ukoi->setDropEnabled(false);
        }
        if (!etiFound) {
          QString qs;
          qs.setNum(idx);
          eti=new KontoTreeItem(ukoi, displaymode, qs);
          ukoi->setExpanded(true);
        }
        ukoi->setText(KontoTreeItem::COL_TYPE,"");
        ukoi->setText(KontoTreeItem::COL_PSP,"");
        ukoi->setText(KontoTreeItem::COL_TIME,"");
        ukoi->setText(KontoTreeItem::COL_ACCOUNTABLE,"");
        ukoi->setGray();
        ukoi->setBgColor(abtList->getBgColor(abt,ko,uko));
        if (bereitschaften.size()>=1) {
          ukoi->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_action_stamp"));
          ukoi->setText(KontoTreeItem::COL_COMMENT,bereitschaften.join("; "));
        }
        else {
          ukoi->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
          ukoi->setText(KontoTreeItem::COL_COMMENT,"");
          ukoi->setMicroAccount(false);
        }
      }

      if ((abtList->isAktiv(abt,ko,uko,idx))&&(abtList->getDatum()==QDate::currentDate()))
      {
        if (srlist.isEmpty()) {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_action_apply"));
        } else {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_moon_apply"));
        }
      }
      else {
        if (srlist.isEmpty()) {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon());
        } else {
          eti->setIcon(KontoTreeItem::COL_ACTIVE,QIcon(":/hi16_moon"));
        }
      }
      QString comment=etiter->second.kommentar;
      refreshComment(comment,eti,etl);
      eti->setText(KontoTreeItem::COL_TYPE,dd.type());
      eti->setText(KontoTreeItem::COL_PSP,dd.pspElem());
      eti->setText(KontoTreeItem::COL_ACTIVE, "");
      eti->setText(KontoTreeItem::COL_TIME,tc.toString());
      eti->setText(KontoTreeItem::COL_ACCOUNTABLE,tcAbzur.toString());
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
    if(topLevelItem(i)->text(KontoTreeItem::COL_ACCOUNTS)==ALLE_KONTEN_STRING)
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
  for( int i=0; i<childCount; i++)
  {
    QTreeWidgetItem* it = item->child( i );
    QString s=it->text(KontoTreeItem::COL_TIME);
    s=s.replace("+","");
    if (s.isEmpty()) {
      getSumTime(it, sum, sumAbs);
    }
    else {
      sum.addTime(TimeCounter::fromString(s));
      QString sabs=it->text(KontoTreeItem::COL_ACCOUNTABLE);
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
  p->setText(KontoTreeItem::COL_TIME,prefix+sum.toString());
  p->setText(KontoTreeItem::COL_ACCOUNTABLE,prefix+sumAbs.toString());
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
    refreshAllItemsInUnterkonto(abt,ko,ukontPos->first);
  }
}

void KontoTreeView::refreshAllItemsInDepartment(const QString& department)
{
	KontoListe* accountList;

	if (!abtList->findDepartment(accountList, department)) return;

	for (KontoListe::iterator subAccountPos = accountList->begin();
			subAccountPos != accountList->end();
			++subAccountPos) {
		refreshAllItemsInKonto(department, subAccountPos->first);
	}
}

void KontoTreeView::setDisplayMode(SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode)
{
  this->displaymode=displaymode;
  load(abtList);
}