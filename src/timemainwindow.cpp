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

#define MAX_WORKTIME 10*60*60 // Warn if more time is spent
#define NO_CHECKIN_ACTION

#include "timemainwindow.h"
#include "qclipboard.h"
#include "qapplication.h"
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "kontotreeview.h"
#include "time.h"
#include "qtimer.h"
#include <iostream>
#include "toolbar.h"
#include "qmessagebox.h"
#include "qaction.h"
#include "qstringlist.h"
#include "statusbar.h"
#include "qdatetime.h"
#include "preferencedialog.h"
#include "defaulttagreader.h"
#ifndef HAS_NO_DATETIMEEDIT
#include "datedialog.h"
#endif
#include "qpoint.h"
#include "globals.h"
#include "qinputdialog.h"
#ifndef NO_TEXTEDIT
#include "qtextedit.h"
#endif
#include "qfile.h"
#include "findkontodialog.h"
#include "sctimehelp.h"
#include "../pics/hi22_action_player_pause.xpm"
#include "../pics/hi22_action_player_pause_half.xpm"
#include "../pics/hi22_action_filesave.xpm"
#include "../pics/hi22_action_attach.xpm"
#include "../pics/hi22_action_edit.xpm"
#include "../pics/hi22_action_queue.xpm"
#include "../pics/hi22_action_1uparrow.xpm"
#include "../pics/hi22_action_1downarrow.xpm"
#include "../pics/hi22_action_2uparrow.xpm"
#include "../pics/hi22_action_2downarrow.xpm"
#include "../pics/sc_logo.xpm"
#include "../pics/scLogo_15Farben.xpm"
#include "../pics/hi22_action_1uparrow_half.xpm"
#include "../pics/hi22_action_1downarrow_half.xpm"
#include "../pics/hi22_action_2uparrow_half.xpm"
#include "../pics/hi22_action_2downarrow_half.xpm"
#include "../pics/zero.xpm"


/** Erzeugt ein neues TimeMainWindow, das seine Daten aus abtlist bezieht. */
TimeMainWindow::TimeMainWindow(KontoDatenInfo* zk):QMainWindow(0,"sctime")
{
  QDate heute;
  abtListToday=new AbteilungsListe(heute.currentDate(),zk);
  abtList=abtListToday;
  paused=false;
  pausedAbzur=false;
  inPersoenlicheKontenAllowed=true;
  powerToolBar = NULL;
  settings=new SCTimeXMLSettings();
  settings->readSettings(abtList);
  defaultCommentReader = new DefaultCommentReader();
  defaultCommentReader->read(abtList);

  DefaultTagReader defaulttagreader;
  defaulttagreader.read(&defaultTags);
  
  // restore size+position
  QSize size;
  QPoint pos;
  settings->getMainWindowGeometry(pos,size);
  resize(size);
  move(pos);

 // setCaption("sctime "+abtList->getDatum().toString("dd.MM.yyyy"));
  kontoTree=new KontoTreeView( this, abtList );
  kontoTree->closeFlaggedPersoenlicheItems();

  mimeSourceFactory=new QMimeSourceFactory();
  mimeSourceFactory->setPixmap("/images/scLogo_15Farben.png",QPixmap((const char **)scLogo_15Farben_xpm));
  setIcon(QPixmap((const char **)sc_logo_xpm));

  setCentralWidget(kontoTree);

  statusBar = new StatusBar(this);
  toolBar   = new ToolBar(this);
    
  configClickMode(settings->singleClickActivation());

  QPopupMenu * kontomenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Konto", kontomenu );

  QPopupMenu * zeitmenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Zeit", zeitmenu );

  QPopupMenu * settingsmenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Einstellungen", settingsmenu );

  QPopupMenu * hilfemenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Hilfe", hilfemenu );

  QTimer* timer = new QTimer(this);
  connect( timer,SIGNAL(timeout()), this, SLOT(minutenUhr()));
  timer->start(60000); //Alle 60 Sekunden ticken

  QTimer* autosavetimer=new QTimer(this);
  connect( autosavetimer,SIGNAL(timeout()), this, SLOT(save()));
  autosavetimer->start(300000); //Alle 5 Minuten ticken.

  QAction* pauseAction = new QAction( "Pause", QPixmap((const char **)hi22_action_player_pause ),
                                       "&Pause", CTRL+Key_P, this, "pause" );
  connect(pauseAction, SIGNAL(activated()), this, SLOT(pause()));

  QAction* pauseAbzurAction = new QAction( "Pausiert nur die abzurechnende Zeit", QPixmap((const char **)hi22_action_player_pause_half ),
                                      "Pause der &abzur. Zeit", CTRL+Key_A, this, "pause Abzur" ,true);
  connect(pauseAbzurAction, SIGNAL(toggled(bool)), this, SLOT(pauseAbzur(bool)));

  QAction* saveAction = new QAction( "Save", QPixmap((const char **)hi22_action_filesave ),
                                      "&Save", CTRL+Key_S, this, "save" );
  connect(saveAction, SIGNAL(activated()), this, SLOT(save()));
  
  QAction* copyAction = new QAction( "Name ins Clipboard kopieren",
                                      "&Copy", CTRL+Key_C, this, "copy" );
  connect(copyAction, SIGNAL(activated()), this, SLOT(copyNameToClipboard()));

  QAction* changeDateAction = new QAction( "Datum Wählen",
                                      "&Datum wählen", CTRL+Key_D, this, "datum wählen" );
  connect(changeDateAction, SIGNAL(activated()), this, SLOT(callDateDialog()));

  QAction* resetAction = new QAction( "Zeitdifferenz auf Null setzen",
                                      "&Differenz auf Null", CTRL+Key_N, this, "differenz null" );
  connect(resetAction, SIGNAL(activated()), this, SLOT(resetDiff()));

#ifndef NO_CHECKIN_ACTION
  checkInAction = new QAction( "Aktuellen Tag einchecken",
                                      "&Tag einchecken", 0, this, "checkin" );
  connect(checkInAction, SIGNAL(activated()), this, SLOT(checkIn()));
#endif

  inPersKontAction = new QAction( "In persönliche Konten", QPixmap((const char **)hi22_action_attach),
                                      "In persönliche &Konten", CTRL+Key_K, this, "persoenliche Konten", true);

  connect(inPersKontAction, SIGNAL(toggled(bool)), this, SLOT(inPersoenlicheKonten(bool)));

  QAction* quitAction = new QAction( "Programm beenden",
                                      "&Beenden", CTRL+Key_Q, this, "beenden" );
  connect(quitAction, SIGNAL(activated()), this, SLOT(close()));

  QAction* findKontoAction = new QAction( "Konto suchen",
                                      "&Suchen", CTRL+Key_F, this, "suchen" );
  connect(findKontoAction, SIGNAL(activated()), this, SLOT(callFindKontoDialog()));
  
  QAction* refreshAction = new QAction( "Kontoliste neu laden",
                                       "&Kontoliste neu laden", CTRL+Key_R, this, "refresh" );
  connect(refreshAction, SIGNAL(activated()), this, SLOT(refreshKontoListe()));

  QAction* preferenceAction = new QAction( "sctime konfigurieren",
                                      "sctime &konfigurieren", 0, this, "configsctime" );
  connect(preferenceAction, SIGNAL(activated()), this, SLOT(callPreferenceDialog()));

  QAction* defaultCommentAction = new QAction( "Default Kommentare neu einlesen",
                                      "&Default Kommentare neu einlesen", 0, this, "reloaddefcomment" );
  connect(defaultCommentAction, SIGNAL(activated()), this, SLOT(reloadDefaultComments()));

#ifndef NO_TEXTEDIT
  QAction* helpAction = new QAction( "Anleitung",
                                      "&Anleitung", Key_F1, this, "anleitung" );
  connect(helpAction, SIGNAL(activated()), this, SLOT(callHelpDialog()));
#endif

  QAction* aboutAction = new QAction( "About sctime",
                                      "&About", 0, this, "about" );
  connect(aboutAction, SIGNAL(activated()), this, SLOT(callAboutBox()));

  editUnterKontoAction = new QAction( "Unterkonto editieren", QPixmap((const char **)hi22_action_edit ),
                                      "&Editieren", 0, this, "unterkonto editieren" );
  connect(editUnterKontoAction, SIGNAL(activated()), this, SLOT(editUnterKontoPressed()));

  QAction* eintragActivateAction = new QAction( "Eintrag aktivieren",
                                                "Eintrag a&ktivieren", CTRL+Key_X, this, "eintr aktiv" );
  connect(eintragActivateAction, SIGNAL(activated()), this, SLOT(eintragAktivieren()));

  QAction* eintragAddAction = new QAction( "Eintrag hinzufuegen", QPixmap((const char **)hi22_action_queue ),
                                      "&Eintrag hinzufügen", 0, this, "eintr hinz" );
  connect(eintragAddAction, SIGNAL(activated()), this, SLOT(eintragHinzufuegen()));

  eintragRemoveAction = new QAction( "Eintrag löschen",
                                      "&Eintrag löschen", 0, this, "eintr loeschen" );
  connect(eintragRemoveAction, SIGNAL(activated()), this, SLOT(eintragEntfernen()));

  QAction* min5PlusAction = new QAction( "Zeit incrementieren", QPixmap((const char **)hi22_action_1uparrow ),
                                      "Zeit incrementieren", 0, this, "+5Min" );
  QAction* min5MinusAction = new QAction( "Zeit decrementieren", QPixmap((const char **)hi22_action_1downarrow ),
                                      "Zeit decrementieren", 0, this, "-5Min" );

  QAction* fastPlusAction = new QAction( "Zeit schnell incrementieren", QPixmap((const char **)hi22_action_2uparrow ),
                                      "Zeit schnell incrementieren", 0, this, "+30Min" );
  QAction* fastMinusAction = new QAction( "Zeit schnell decrementieren", QPixmap((const char **)hi22_action_2downarrow ),
                                      "Zeit schnell decrementieren", 0, this, "-30Min" );

  abzurMin5PlusAction = new QAction( "Abrechenbare Zeit incrementieren", QPixmap((const char **)hi22_action_1uparrow_half ),
                                      "Abrechenbare Zeit incrementieren", 0, this, "+5Min" );
  abzurMin5MinusAction = new QAction( "Abrechenbare Zeit decrementieren", QPixmap((const char **)hi22_action_1downarrow_half ),
                                      "Abrechenbare Zeit decrementieren", 0, this, "-5Min" );

  fastAbzurPlusAction = new QAction( "Abrechenbare Zeit schnell incrementieren", QPixmap((const char **)hi22_action_2uparrow_half ),
                                             "Abrechenbare Zeit schnell incrementieren", 0, this, "+30Min" );
  fastAbzurMinusAction = new QAction( "Abrechenbare Zeit schnell decrementieren", QPixmap((const char **)hi22_action_2downarrow_half ),
                                      "Abrechenbare Zeit schnell decrementieren", 0, this, "-30Min" );

  connect(kontoTree, SIGNAL(currentChanged(QListViewItem * )), this, SLOT(changeShortCutSettings(QListViewItem * ) ));

  connect(min5PlusAction, SIGNAL(activated()), this, SLOT(addTimeInc()));
  connect(min5MinusAction, SIGNAL(activated()), this, SLOT(subTimeInc()));
  connect(fastPlusAction, SIGNAL(activated()), this, SLOT(addFastTimeInc()));
  connect(fastMinusAction, SIGNAL(activated()), this, SLOT(subFastTimeInc()));

  connect(this,SIGNAL(eintragSelected(bool)), min5PlusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), min5MinusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), fastPlusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), fastMinusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), eintragAddAction, SLOT(setEnabled(bool)));

  connect(this,SIGNAL(aktivierbarerEintragSelected(bool)), eintragActivateAction, SLOT(setEnabled(bool)));

  editUnterKontoAction->addTo(toolBar);
  saveAction->addTo(toolBar);
  inPersKontAction->addTo(toolBar);
  eintragAddAction->addTo(toolBar);
  pauseAction->addTo(toolBar);
  min5PlusAction->addTo(toolBar);
  min5MinusAction->addTo(toolBar);
  fastPlusAction->addTo(toolBar);
  fastMinusAction->addTo(toolBar);
  editUnterKontoAction->addTo(kontomenu);
  eintragActivateAction->addTo(kontomenu);
  eintragAddAction->addTo(kontomenu);
  eintragRemoveAction->addTo(kontomenu);
  saveAction->addTo(kontomenu);
  pauseAction->addTo(kontomenu);
  pauseAbzurAction->addTo(kontomenu);
  findKontoAction->addTo(kontomenu);
  refreshAction->addTo(kontomenu);
  changeDateAction->addTo(zeitmenu);
  resetAction->addTo(zeitmenu);
#ifndef NO_CHECKIN_ACTION
  checkInAction->addTo(zeitmenu);
#endif
  kontomenu->insertSeparator();
  quitAction->addTo(kontomenu);
  defaultCommentAction->addTo(settingsmenu);
  preferenceAction->addTo(settingsmenu);
  #ifndef NO_TEXTEDIT
  helpAction->addTo(hilfemenu);
  #endif
  aboutAction->addTo(hilfemenu);

  zeitChanged();

  changeShortCutSettings(NULL); // Unterkontenmenues deaktivieren...

  updateCaption();
  kontoTree->showAktivesProjekt();
  showAdditionalButtons(settings->powerUserView());
}

/** Destruktor - speichert vor dem Beenden die Einstellungen */
TimeMainWindow::~TimeMainWindow()
{
   save();
   delete settings;
   if (abtList!=abtListToday)
     delete abtListToday;
   delete abtList;
}

void TimeMainWindow::showAdditionalButtons(bool show)
{
   if (show) {
      if (powerToolBar!=NULL) return;
      powerToolBar   = new QToolBar(this);

      abzurMin5PlusAction->addTo(powerToolBar);
      abzurMin5MinusAction->addTo(powerToolBar);
      fastAbzurPlusAction->addTo(powerToolBar);
      fastAbzurMinusAction->addTo(powerToolBar);
      connect(abzurMin5PlusAction, SIGNAL(activated()), this, SLOT(addAbzurTimeInc()));
      connect(abzurMin5MinusAction, SIGNAL(activated()), this, SLOT(subAbzurTimeInc()));
      connect(fastAbzurPlusAction, SIGNAL(activated()), this, SLOT(addFastAbzurTimeInc()));
      connect(fastAbzurMinusAction, SIGNAL(activated()), this, SLOT(subFastAbzurTimeInc()));
      connect(this,SIGNAL(eintragSelected(bool)), abzurMin5PlusAction, SLOT(setEnabled(bool)));
      connect(this,SIGNAL(eintragSelected(bool)), abzurMin5MinusAction, SLOT(setEnabled(bool)));
      connect(this,SIGNAL(eintragSelected(bool)), fastAbzurPlusAction, SLOT(setEnabled(bool)));
      connect(this,SIGNAL(eintragSelected(bool)), fastAbzurMinusAction, SLOT(setEnabled(bool)));
   } else {
      if (powerToolBar==NULL) return;
      delete(powerToolBar);
      powerToolBar = NULL;
   }
}

void TimeMainWindow::configClickMode(bool singleClickActivation)
{
    if (!settings->singleClickActivation()) {
        disconnect(kontoTree, SIGNAL(mouseButtonClicked ( int, QListViewItem * , const QPoint & , int )), 
                   this, SLOT(mouseButtonInKontoTreeClicked(int, QListViewItem * , const QPoint &, int )));
        disconnect( kontoTree, SIGNAL(doubleClicked(QListViewItem *)), 
                 this, SLOT(callUnterKontoDialog(QListViewItem *)) );
        connect( kontoTree, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint& ,int)), 
                 this, SLOT(callUnterKontoDialog(QListViewItem *)) );
        connect( kontoTree, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(setAktivesProjekt(QListViewItem *)));
        }
    else {    
        disconnect(kontoTree, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(setAktivesProjekt(QListViewItem *)));
        disconnect(kontoTree, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint& ,int)),
                   this, SLOT(callUnterKontoDialog(QListViewItem *)));
        connect(kontoTree, SIGNAL(mouseButtonClicked ( int, QListViewItem * , const QPoint & , int )),
                   this, SLOT(mouseButtonInKontoTreeClicked(int, QListViewItem * , const QPoint &, int )));
        connect( kontoTree, SIGNAL(doubleClicked(QListViewItem *)),
                 this, SLOT(callUnterKontoDialog(QListViewItem *)) );
    }               
}

void TimeMainWindow::customEvent( QCustomEvent * e)
{
   if (e->type()==SIGINT_EVENT_ID) {
      close();
   }
   QMainWindow::customEvent(e);
}

void TimeMainWindow::copyNameToClipboard()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText( kontoTree->currentItem()->text(0), QClipboard::Clipboard );
}

void TimeMainWindow::mouseButtonInKontoTreeClicked(int button, QListViewItem * item, const QPoint & pos, int c)
{    
    if ((button==1) && (item)) {
        setAktivesProjekt(item);
    }    
}

/** Wird durch einen Timer einmal pro Minute aufgerufen, und sorgt fuer die
  * korrekte Aktualisierung der Objekte.
*/
void TimeMainWindow::minutenUhr()
{
  QString abt,ko,uko;
  int idx;

  if (!paused) {
    abtListToday->getAktiv(abt,ko,uko,idx);
    abtListToday->minuteVergangen(!pausedAbzur);
    kontoTree->refreshItem(abt,ko,uko,idx);
    zeitChanged();
    emit minuteTick();
    if (!pausedAbzur) emit minuteAbzurTick();
  }

  //fix-me: falls bis zu zwei Minuten nach Mitternacht das gestrige Datum
  //eingestellt ist, aufs neue Datum umstellen - Aergernis, falls jemand zw 0:00 und 0:02 tatsaechlich
  //den vorigen Tag aendern moechte.

  if ((abtList->getDatum().daysTo(QDate::currentDate())==1)&&(QTime::currentTime().secsTo ( QTime(0,2) )>0))
  {
    emit changeDate(QDate::currentDate());
  }
}


/**
  * Addiert timeIncrement auf die Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::addTimeInc()
{
  addDeltaToZeit(settings->timeIncrement());
}


/**
  * Subtrahiert timeIncrement von den Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::subTimeInc()
{
  addDeltaToZeit(-settings->timeIncrement());
}


/**
  * Addiert fastTimeIncrement auf die Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::addFastTimeInc()
{
  addDeltaToZeit(settings->fastTimeIncrement());
}


/**
  * Subtrahiert timeIncrement von den Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::subFastTimeInc()
{
  addDeltaToZeit(-settings->fastTimeIncrement());
}

/**
  * Addiert timeIncrement auf die Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::addAbzurTimeInc()
{
  addDeltaToZeit(settings->timeIncrement(), true);
}


/**
  * Subtrahiert timeIncrement von den Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::subAbzurTimeInc()
{
  addDeltaToZeit(-settings->timeIncrement(), true);
}


/**
  * Addiert fastTimeIncrement auf die Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::addFastAbzurTimeInc()
{
  addDeltaToZeit(settings->fastTimeIncrement(), true);
}

/**
  * Subtrahiert timeIncrement von den Zeiten des selktierten Unterkontos.
  */
void TimeMainWindow::subFastAbzurTimeInc()
{
  addDeltaToZeit(-settings->fastTimeIncrement(), true);
}

/**
  *  Addiert Delta Sekunden auf die Zeiten des selektierten Unterkontos.
  */
void TimeMainWindow::addDeltaToZeit(int delta, bool abzurOnly)
{
  QListViewItem * item=kontoTree->currentItem();

  if (!kontoTree->isEintragsItem(item)) return;

  QString uko,ko,abt,top;
  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  abtList->changeZeit(abt, ko, uko, idx, delta, abzurOnly);
  kontoTree->refreshItem(abt, ko, uko, idx);
  zeitChanged();
}


/**
 *  Bestimmt die veraenderte Gesamtzeit und loest die Signale gesamtZeitChanged und
 *  gesamtZeitAbzurChanged aus.
 */
void TimeMainWindow::zeitChanged()
{
  static int last=0;
  int zeitAbzur, zeit;
  abtList->getGesamtZeit(zeit, zeitAbzur);
  TimeCounter tc(zeit);
  setIconText(tc.toString());
  statusBar->setDiff(abtList->getZeitDifferenz());
  emit gesamtZeitChanged(zeit);
  emit gesamtZeitAbzurChanged(zeitAbzur);
  // Beim ersten ueberschreiten von MAX_WORKTIME
  if ((zeit>MAX_WORKTIME)&&(last<=MAX_WORKTIME)) {
    // last muss _vor_ dem oeffnen der Messagebox gesetzt werden,
    // da es andernfalls erst nach dem Schliessen der Box gesetzt wird, was bedeuten wuerde,
    // dass (falls der user nicht sofort reagiert), jede Minute eine neue Box aufpoppt
    // => nix gut am naechsten morgen, wenn man das ausloggen vergisst :-)
    last=zeit;
    // Vorlaeufig deaktiviert
    /*QMessageBox::warning(this,"Warnung","Warnung: die gesetzlich zulässige Arbeitszeit wurde überschritten.",
                       QMessageBox::Ok | QMessageBox::Default,0);*/
  }
  else
    last=zeit;
}


/** Ruft einen modalen Dialog auf, der eine Pause anzeigt, und setzt gleichzeitig
  *  paused auf true, um die Zeiten anzuhalten
  */
void TimeMainWindow::pause()
{
  paused=true;
  QMessageBox::warning(this,"Pause Dialog","Die Zeiterfassung wurde angehalten. Ende der Pause mit OK.",
                       QMessageBox::Ok | QMessageBox::Default,0);
  paused=false;
}


/**
 * Setzt, ob die abzurechnende Zeit pausiert werden soll.
 */
void TimeMainWindow::pauseAbzur(bool on)
{
  pausedAbzur=on;
}


/** 
 * Speichert die aktuellen Zeiten und Einstellungen
 */
void TimeMainWindow::save()
{ 
  kontoTree->flagClosedPersoenlicheItems();
  settings->setMainWindowGeometry(pos(),size());
  settings->writeSettings(abtListToday);
  settings->writeShellSkript(abtListToday);
  if (abtList!=abtListToday) {
    settings->writeSettings(abtList);
    settings->writeShellSkript(abtList);
  }
}



/**
 * Loest ein callUnterKontoDialog Signal mit dem selektierten Item auf
 */
void TimeMainWindow::editUnterKontoPressed()
{
  emit callUnterKontoDialog(kontoTree->currentItem());
}


/**
 * Aktiviert einen Eintrag
 */
void TimeMainWindow::eintragAktivieren()
{
  QListViewItem * item=kontoTree->currentItem();
  setAktivesProjekt(item);
}


/**
 * Fuegt einen Eintrag zum selektierten Unterkonto hinzu.
 */
void TimeMainWindow::eintragHinzufuegen()
{
  QListViewItem * item=kontoTree->currentItem();

  if (!kontoTree->isEintragsItem(item)) return;

  QString top,uko,ko,abt;
  int oldidx;

  kontoTree->itemInfo(item,top,abt,ko,uko,oldidx);

  int idx=abtList->insertEintrag(abt,ko,uko);
  abtList->setEintragFlags(abt,ko,uko,idx,abtList->getEintragFlags(abt,ko,uko,oldidx));
  kontoTree->refreshAllItemsInUnterkonto(abt,ko,uko);
  changeShortCutSettings(item);
}


/**
 * Entfernt einen Eintrag.
 */
void TimeMainWindow::eintragEntfernen()
{
  QListViewItem * item=kontoTree->currentItem();

  if ((!item)||(item->depth()!=4)) return;

  QString top,uko,ko,abt;
  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  KontoTreeItem *topi, *abti, *koi, *ukoi, *eti;

  if (abtList->isAktiv(abt,ko,uko,idx)) {
      QMessageBox::warning(NULL,"Warnung","Kann aktiven Eintrag nicht löschen\n",
                              QMessageBox::Ok, QMessageBox::NoButton,
                              QMessageBox::NoButton);
      return;
  }

  abtList->setSekunden(abt,ko,uko,idx,0); // Explizit vorher auf Null setzen, um die Gesamtzeit
                                          // nicht zu verwirren.
  abtList->deleteEintrag(abt,ko,uko,idx);

  kontoTree->sucheItem(PERSOENLICHE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti);
  delete ukoi;
  kontoTree->sucheItem(ALLE_KONTEN_STRING,abt,ko,uko,idx,topi,abti,koi,ukoi,eti);
  delete ukoi;
  kontoTree->refreshAllItemsInUnterkonto(abt,ko,uko);
  zeitChanged();
}


/**
 * Aendert das Datum: dazu werden zuerst die aktuellen Zeiten und Einstellungen gespeichert,
 * sodann die Daten fuer das angegebene Datum neu eingelesen.
 */
void TimeMainWindow::changeDate(const QDate& datum)
{
  bool currentDateSel = (datum==QDate::currentDate());

  kontoTree->flagClosedPersoenlicheItems();
  settings->writeSettings(abtList);
  settings->writeShellSkript(abtList);
  if (abtListToday!=abtList) {
    settings->writeSettings(abtListToday);
    settings->writeShellSkript(abtListToday);
    if (currentDateSel)
      delete abtList;
  }
  if (currentDateSel) {
    abtList=abtListToday;
    if (abtListToday->getDatum()!=datum)
      abtListToday->setDatum(datum);
  }
  else {
    abtList=new AbteilungsListe(datum,abtListToday);
  }

  abtList->clearKonten();
  settings->readSettings(abtList);

  kontoTree->load(abtList);
  kontoTree->closeFlaggedPersoenlicheItems();
  kontoTree->showAktivesProjekt();
  zeitChanged();
  emit (currentDateSelected(currentDateSel));
  statusBar->dateWarning(!currentDateSel, datum);
}

void TimeMainWindow::refreshKontoListe()
{
  kontoTree->flagClosedPersoenlicheItems();
  settings->writeSettings(abtList); // die Settings ueberstehen das Reload nicht
  abtList->reload();
  settings->readSettings(abtList);
  if (abtList!=abtListToday) {
    settings->writeSettings(abtListToday); // die Settings ueberstehen das Reload nicht
    abtListToday->reload();
    settings->readSettings(abtListToday);
  }
  kontoTree->load(abtList);
  kontoTree->closeFlaggedPersoenlicheItems();
}

void TimeMainWindow::reloadDefaultComments()
{
  if (abtList!=abtListToday) {
    abtListToday->clearDefaultComments();
    defaultCommentReader->read(abtListToday);
  }
  abtList->clearDefaultComments();
  defaultCommentReader->read(abtList);
}

/**
 * Fuegt das aktuell selektierte Unterkonto den Persoenlichen Konten hinzu.
 * Falls kein Unterkonto selektiert oder inPersoenlicheKonten==false ist, passiert nichts.
 */
void TimeMainWindow::inPersoenlicheKonten(bool hinzufuegen)
{

  if (!inPersoenlicheKontenAllowed) return;

  QListViewItem * item=kontoTree->currentItem();

  if (!item) return;

  QString uko,ko,abt,top;
  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  if (item->depth()==2) {
    abtList->moveKontoPersoenlich(abt,ko,hinzufuegen);
    kontoTree->refreshAllItemsInKonto(abt,ko);
    return;
  }
  else {
    if (item->depth()==3) {
      abtList->moveUnterKontoPersoenlich(abt,ko,uko,hinzufuegen);
      kontoTree->refreshAllItemsInUnterkonto(abt,ko,uko);
      return;
	}
  }

  abtList->moveEintragPersoenlich(abt,ko,uko,idx,hinzufuegen);
  kontoTree->refreshItem(abt,ko,uko,idx);
}


/**
 * Aendert die Einstellungen fuer die Menueshortcuts entsprechend dem selektierten Item
 */
void TimeMainWindow::changeShortCutSettings(QListViewItem * item)
{
  bool iseintragsitem=kontoTree->isEintragsItem(item);
  inPersoenlicheKontenAllowed=false; //Vorsorglich disablen, sonst Seiteneffekte mit flagsChanged.
  inPersKontAction->setEnabled(false);

  QString uko,ko,abt;
  QString top=""; // top wird weiter unten ausgelesen, und es ist nicht sicher, ob es initialisiert wurde.
  int idx;
  
  if (item) kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  if (iseintragsitem) {

    if (item->depth()<=3) 
       eintragRemoveAction->setEnabled(false);
    else
       eintragRemoveAction->setEnabled(true);

    flagsChanged(abt,ko,uko,idx);
    inPersKontAction->setEnabled(!abtList->checkInState());
    editUnterKontoAction->setEnabled(!abtList->checkInState());
    /* Eigentlich sollte das Signal in editierbarerEintragSelected umbenannt werden... */
    emit eintragSelected(!abtList->checkInState());
    if (abtListToday==abtList)
      emit aktivierbarerEintragSelected(!abtList->checkInState());
  }
  else {
    // Auch bei Konten und Unterkonten in Pers. Konten PersKontAction auf On stellen.
    inPersKontAction->setOn((item&&(top==PERSOENLICHE_KONTEN_STRING)&&(item->depth()>=2)&&(item->depth()<=3)));
    inPersKontAction->setEnabled((!abtList->checkInState())&&(item&&(item->depth()>=2)&&(item->depth()<=3)));
    editUnterKontoAction->setEnabled(false);
    emit eintragSelected(false);
    emit aktivierbarerEintragSelected(false);
    eintragRemoveAction->setEnabled(false);
  }
  inPersoenlicheKontenAllowed=true; // Wieder enablen.
}

void TimeMainWindow::updateCaption()
{  
   QString abt, ko, uko;
   int idx;
   abtList->getAktiv(abt,ko,uko,idx);
   setCaption("sctime - "+ abt+"/"+ko+"/"+uko);
}

void TimeMainWindow::resetDiff()
{
   abtList->resetZeitDifferenz();
   zeitChanged();
}

/**
 * Sollte aufgerufen werden, sobald sich die Einstellungen fuer ein Konto aendern.
 * Toggelt zB inPersKontAction.
 */
void TimeMainWindow::flagsChanged(const QString& abt, const QString& ko, const QString& uko, int idx)
{

  QListViewItem * item=kontoTree->currentItem();

  if (!item) return;

  QString selecteduko,selectedko,selectedabt,selectedtop;
  int selectedidx;

  kontoTree->itemInfo(item,selectedtop,selectedabt,selectedko,selecteduko,selectedidx);
  if ((selectedabt==abt)&&(selectedko==ko)&&(selecteduko==uko)&&(selectedidx==idx)) {
    inPersKontAction->setOn((abtList->getEintragFlags(abt,ko,uko,idx)&UK_PERSOENLICH)&&(!abtList->checkInState()));
  }

  updateCaption();
}

void TimeMainWindow::checkIn()
{
  if (abtList->getDatum()>=QDate::currentDate()) {
    QMessageBox::critical(this,"Fehler","Heutiges Datum kann nicht über die GUI eingecheckt werden.\nZeiten nicht eingecheckt!",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  }
  if (abtList->checkInState()) {
    QMessageBox::critical(this,"Fehler","Ausgewähltes Datum ist bereits eingecheckt.\nZeiten nicht eingecheckt!",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  }
  settings->writeSettings(abtList);
  settings->writeShellSkript(abtList);

  // do checkin
  if (!abtList->checkIn()) {
    QMessageBox::critical(this,"Fehler","Fehler beim einchecken.\nZeiten nicht eingecheckt!",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  } else {
    // move zeit* files
    abtList->setCheckInState(true);
    settings->moveToCheckedIn(abtList);
  };
  kontoTree->load(abtList);
  kontoTree->closeFlaggedPersoenlicheItems();
  kontoTree->showAktivesProjekt();
}

/**
 * Erzeugt einen UnterkontoDialog fuer item.
 */
void TimeMainWindow::callUnterKontoDialog(QListViewItem * item)
{
  if ((!kontoTree->isEintragsItem(item))||(abtList->checkInState()))
    return;

  QString top,uko,ko,abt;

  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  unterKontoDialog=new UnterKontoDialog(abt,ko,uko,idx,abtList,&defaultTags, true,this);
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int )), kontoTree,
  SLOT(refreshItem(const QString&, const QString&, const QString&,int )));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int)), this, SLOT(zeitChanged()));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int)),
           this, SLOT(flagsChanged(const QString&, const QString&, const QString&,int)));
  if (abtList->isAktiv(abt,ko,uko,idx))
    connect(this, SIGNAL(minuteTick()),unterKontoDialog->getZeitBox(),SLOT(incrMin()));
  unterKontoDialog->show();
}

/**
 * Baut den Kontosuchdialog auf, und zeigt das Such-Ergebnis an.
 */
void TimeMainWindow::callFindKontoDialog()
{

  QString konto;

  FindKontoDialog findKontoDialog(abtList,&konto,this);
  if (findKontoDialog.exec()!=QDialog::Accepted) return;


  QString abt=abtList->findAbteilungOfKonto(konto);
  if (abt=="") {
    QMessageBox::warning(this,"Konto nicht gefunden","Das Konto "+konto+" konnte nicht gefunden werden.",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  }

  QListViewItem *item = kontoTree->sucheKontoItem(ALLE_KONTEN_STRING,abt,konto);
  if (item) {
    kontoTree->setCurrentItem(item);
    kontoTree->ensureItemVisible(item);
  }
}

void TimeMainWindow::callPreferenceDialog()
{
  PreferenceDialog preferenceDialog(settings, this);
  preferenceDialog.exec();
  showAdditionalButtons(settings->powerUserView());
  configClickMode(settings->singleClickActivation());
}

/**
 * Setzt das zu Item gehoerende Unterkonto als aktiv.
 */
void TimeMainWindow::setAktivesProjekt(QListViewItem * item)
{
  if (!kontoTree->isEintragsItem(item)) return;

  QString uko,ko,abt,top ;
  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  QString oldabt, oldko, olduk;
  int oldidx;
  abtList->getAktiv(oldabt, oldko, olduk,oldidx);
  abtList->setAsAktiv(abt,ko,uko,idx);
  kontoTree->refreshItem(oldabt,oldko,olduk,oldidx);
  kontoTree->refreshItem(abt,ko,uko,idx);
  updateCaption();
}

/**
 * Erzeugt einen DatumsDialog zum aendern des aktuellen Datums.
 */
void TimeMainWindow::callDateDialog()
{
  DateDialog * dateDialog=new DateDialog(abtList->getDatum(), this);
  connect(dateDialog, SIGNAL(dateChanged(const QDate&)), this, SLOT(changeDate(const QDate&)));
  dateDialog->show();
}

#ifndef NO_TEXTEDIT
/**
 *  Baut den Hilfe-Dialog auf.
 */

void TimeMainWindow::callHelpDialog()
{
  QDialog * helpDialog = new QDialog(this);
  QVBoxLayout* layout = new QVBoxLayout(helpDialog);
  QTextEdit * helpBrowser = new QTextEdit(helpDialog,"Anleitung");

  helpBrowser->setMimeSourceFactory(mimeSourceFactory);
  helpBrowser->setTextFormat(RichText);
  helpBrowser->setText(sctimehelptext);

  helpBrowser->setReadOnly(true);
  layout->addWidget(helpBrowser);

  layout->addSpacing(7);

  QHBoxLayout* buttonlayout=new QHBoxLayout(layout,3);
  QPushButton * okbutton=new QPushButton( "OK", helpDialog);

  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addStretch(1);
  layout->addSpacing(4);

  helpDialog->resize(600,450);

  connect (okbutton, SIGNAL(clicked()), helpDialog, SLOT(close()));

  helpDialog->show();
}

#endif

/** 
 * Zeigt eine About-Box an.
 */

void TimeMainWindow::callAboutBox()
{
  QDialog * aboutBox=new QDialog(this);
  aboutBox->setPaletteBackgroundColor(white);
  QGridLayout* layout=new QGridLayout(aboutBox,7,3);
  QLabel* logo=new QLabel(aboutBox);
  logo->setPixmap(QPixmap((const char **)scLogo_15Farben_xpm));
  layout->addWidget(logo,0,0);
  QLabel versioninfo(QString("<h2>sctime</h2><nobr><b>Version:</b> ")+VERSIONSTR+"</nobr><br><nobr><b>Datum des Builds:</b> "+BUILDDATESTR+"</nobr>",aboutBox);
  versioninfo.setTextFormat(Qt::RichText);
  layout->addWidget(&versioninfo,0,1);
  layout->addRowSpacing(1,10);
  layout->addWidget(new QLabel("Core Developer:",aboutBox),2,0);
  layout->addWidget(new QLabel("Florian Schmitt <f.schmitt@science-computing.de>",aboutBox),2,1);
  layout->addWidget(new QLabel("Patches:",aboutBox),3,0);
  layout->addWidget(new QLabel("Marcus Camen <m.camen@science-computing.de>",aboutBox),3,1);
  layout->addRowSpacing(4,18);
  layout->addMultiCellWidget(new QLabel("<center>This Program is licensed under the GNU Public License.</center>",aboutBox),5,5,0,1);
  layout->addRowSpacing(6,18);

  QHBoxLayout* buttonlayout=new QHBoxLayout();
  QPushButton * okbutton=new QPushButton( "OK", aboutBox);

  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addStretch(1);
  layout->addMultiCellLayout(buttonlayout,7,7,0,1);
  connect (okbutton, SIGNAL(clicked()), aboutBox, SLOT(close()));
  layout->addRowSpacing(8,10);

  aboutBox->exec();

}
