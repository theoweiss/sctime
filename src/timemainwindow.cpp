/*
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

#define NO_CHECKIN_ACTION

#include "timemainwindow.h"
#include "qclipboard.h"
#include "qapplication.h"
#include <QMenu>
#include <QMenuBar>
#include <QAction>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QCustomEvent>
#include <QFont>
#include <Q3Header>
#include <QTextStream>
#include "kontotreeview.h"
#include "time.h"
#include "qtimer.h"
#include <iostream>
#include "toolbar.h"
#include "qmessagebox.h"
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
#include "q3textedit.h"
#endif
#include "bereitschaftsdateninfozeit.h"
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
#include "../pics/hi16_action_stamp.xpm"
#include "../pics/zero.xpm"


/** Erzeugt ein neues TimeMainWindow, das seine Daten aus abtlist bezieht. */
TimeMainWindow::TimeMainWindow(KontoDatenInfo* zk):QMainWindow()
{
  std::vector<QString> xmlfilelist;
  QDate heute;
  abtListToday=new AbteilungsListe(heute.currentDate(),zk);
  abtList=abtListToday;
  paused=false;
  pausedAbzur=false;
  inPersoenlicheKontenAllowed=true;
  powerToolBar = NULL;
  settings=new SCTimeXMLSettings();
  settings->readSettings(abtList);

  settings->getDefaultCommentFiles(xmlfilelist);
  qtDefaultFont=QApplication::font();
  if (settings->useCustomFont())
  {
    QString custFont=settings->customFont();
    int custFontSize=settings->customFontSize();
    QApplication::setFont(QFont(custFont,custFontSize));
  }  
  defaultCommentReader.read(abtList,xmlfilelist);

  BereitschaftsDatenInfoZeit bereitschaftsdatenReader;
  bereitschaftsdatenReader.readInto(BereitschaftsListe::getInstance());

  DefaultTagReader defaulttagreader;
  defaulttagreader.read(&defaultTags);

  // restore size+position
  QSize size;
  QPoint pos;
  settings->getMainWindowGeometry(pos,size);
  resize(size);
  move(pos);

  statusBar = new StatusBar(this);
  setStatusBar(statusBar);

  std::vector<int> columnwidthlist;

  settings->getColumnWidthList(columnwidthlist);

 // setCaption("sctime "+abtList->getDatum().toString("dd.MM.yyyy"));
  kontoTree=new KontoTreeView( this, abtList, columnwidthlist );
  kontoTree->closeFlaggedPersoenlicheItems();  

  mimeSourceFactory=new Q3MimeSourceFactory();
  mimeSourceFactory->setPixmap("/images/scLogo_15Farben.png",QPixmap((const char **)scLogo_15Farben_xpm));
  setIcon(QPixmap((const char **)sc_logo_xpm));

  setCentralWidget(kontoTree);
  
  if (!settings->showTypeColumn()) {  	
  	kontoTree->hideColumn(1);
  	kontoTree->setColumnWidthMode(1,Q3ListView::Manual);
  	kontoTree->header()->setResizeEnabled(false, 1);
  }

  toolBar   = new ToolBar("Main ToolBar", this);
  toolBar->setIconSize(QSize(22,22));

  configClickMode(settings->singleClickActivation());

  QMenu * kontomenu = menuBar()->addMenu("&Konto");
  QMenu * zeitmenu = menuBar()->addMenu("&Zeit");
  QMenu * settingsmenu = menuBar()->addMenu("&Einstellungen");
  QMenu * hilfemenu = menuBar()->addMenu("&Hilfe");

  QTimer* timer = new QTimer(this);
  connect( timer,SIGNAL(timeout()), this, SLOT(minutenUhr()));
  timer->start(60000); //Alle 60 Sekunden ticken
  lastMinuteTick=QDateTime::currentDateTime();

  QTimer* autosavetimer=new QTimer(this);
  connect( autosavetimer,SIGNAL(timeout()), this, SLOT(save()));
  autosavetimer->start(300000); //Alle 5 Minuten ticken.

  QAction* pauseAction = new QAction( QPixmap((const char **)hi22_action_player_pause ),
                                      "&Pause", this);
  pauseAction->setShortcut(Qt::CTRL+Qt::Key_P);
  connect(pauseAction, SIGNAL(triggered()), this, SLOT(pause()));

  QAction* pauseAbzurAction = new QAction( QPixmap((const char **)hi22_action_player_pause_half ),
                                           "Pause der &abzur. Zeit", this);
  pauseAbzurAction->setShortcut(Qt::CTRL+Qt::Key_A);
  pauseAbzurAction->setStatusTip("Pausiert nur die abzurechnende Zeit");
  pauseAbzurAction->setCheckable(true);
  connect(pauseAbzurAction, SIGNAL(toggled(bool)), this, SLOT(pauseAbzur(bool)));

  QAction* saveAction = new QAction( QPixmap((const char **)hi22_action_filesave ),"&Save", this);
  saveAction->setShortcut(Qt::CTRL+Qt::Key_S);
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  QAction* copyAction = new QAction("&Copy", this);
  copyAction->setShortcut(Qt::CTRL+Qt::Key_C);
  copyAction->setStatusTip("Name ins Clipboard kopieren");
  connect(copyAction, SIGNAL(triggered()), this, SLOT(copyNameToClipboard()));

  QAction* changeDateAction = new QAction( "&Datum wählen", this);
  changeDateAction->setShortcut(Qt::CTRL+Qt::Key_D);
  connect(changeDateAction, SIGNAL(triggered()), this, SLOT(callDateDialog()));

  QAction* resetAction = new QAction( "&Differenz auf Null", this);
  resetAction->setShortcut(Qt::CTRL+Qt::Key_N);
  resetAction->setStatusTip("Zeitdifferenz auf Null setzen");
  connect(resetAction, SIGNAL(triggered()), this, SLOT(resetDiff()));

#ifndef NO_CHECKIN_ACTION
  checkInAction = new QAction( "&Tag einchecken", this);
  checkInAction->setShortcut("Aktuellen Tag einchecken");
  connect(checkInAction, SIGNAL(triggered()), this, SLOT(checkIn()));
#endif

  inPersKontAction = new QAction( QPixmap((const char **)hi22_action_attach),"In persönliche &Konten", this);
  inPersKontAction->setShortcut(Qt::CTRL+Qt::Key_K);
  inPersKontAction->setCheckable(true);
  connect(inPersKontAction, SIGNAL(toggled(bool)), this, SLOT(inPersoenlicheKonten(bool)));

  QAction* quitAction = new QAction("&Beenden", this);
  quitAction->setShortcut(Qt::CTRL+Qt::Key_Q);
  quitAction->setStatusTip("Programm beenden");
  connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

  QAction* findKontoAction = new QAction("&Suchen", this);
  findKontoAction->setShortcut(Qt::CTRL+Qt::Key_F);
  findKontoAction->setStatusTip("Konto suchen");
  connect(findKontoAction, SIGNAL(triggered()), this, SLOT(callFindKontoDialog()));

  QAction* refreshAction = new QAction("&Kontoliste neu laden", this);
  refreshAction->setShortcut(Qt::CTRL+Qt::Key_R);
  connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshKontoListe()));

  QAction* preferenceAction = new QAction("sctime &konfigurieren",this);
  connect(preferenceAction, SIGNAL(triggered()), this, SLOT(callPreferenceDialog()));

  QAction* defaultCommentAction = new QAction("&Default Kommentare neu einlesen", this);
  connect(defaultCommentAction, SIGNAL(triggered()), this, SLOT(reloadDefaultComments()));

#ifndef NO_TEXTEDIT
  QAction* helpAction = new QAction("&Anleitung", this);
  helpAction->setShortcut(Qt::Key_F1);
  connect(helpAction, SIGNAL(triggered()), this, SLOT(callHelpDialog()));
#endif

  QAction* aboutAction = new QAction("&About", this);
  aboutAction->setStatusTip("About sctime");
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(callAboutBox()));

  editUnterKontoAction = new QAction(QPixmap((const char **)hi22_action_edit ), "&Editieren", this);
  editUnterKontoAction->setStatusTip("Unterkonto editieren");
  connect(editUnterKontoAction, SIGNAL(triggered()), this, SLOT(editUnterKontoPressed()));

  QAction* eintragActivateAction = new QAction("Eintrag a&ktivieren", this);
  eintragActivateAction->setShortcut(Qt::CTRL+Qt::Key_X);
  connect(eintragActivateAction, SIGNAL(triggered()), this, SLOT(eintragAktivieren()));

  QAction* eintragAddAction = new QAction(QPixmap((const char **)hi22_action_queue ),
                                             "&Eintrag hinzufügen", this);
  connect(eintragAddAction, SIGNAL(triggered()), this, SLOT(eintragHinzufuegen()));

  eintragRemoveAction = new QAction("&Eintrag löschen", this);
  eintragRemoveAction->setShortcut(Qt::Key_Delete);
  connect(eintragRemoveAction, SIGNAL(triggered()), this, SLOT(eintragEntfernen()));

  QAction* bereitschaftsAction = new QAction(QPixmap((const char **)hi16_action_stamp ),
                                             "&Bereitschaftszeiten setzen", this);
  bereitschaftsAction->setShortcut(Qt::CTRL+Qt::Key_B);
  connect(bereitschaftsAction, SIGNAL(triggered()), this, SLOT(editBereitschaftPressed()));

  QAction* min5PlusAction = new QAction(QPixmap((const char **)hi22_action_1uparrow ),
                                           "Zeit incrementieren", this);
  QAction* min5MinusAction = new QAction(QPixmap((const char **)hi22_action_1downarrow ),
                                            "Zeit decrementieren", this);

  QAction* fastPlusAction = new QAction(QPixmap((const char **)hi22_action_2uparrow ),
                                           "Zeit schnell incrementieren", this);
  QAction* fastMinusAction = new QAction(QPixmap((const char **)hi22_action_2downarrow ),
                                            "Zeit schnell decrementieren", this);

  abzurMin5PlusAction = new QAction(QPixmap((const char **)hi22_action_1uparrow_half ),
                                      "Abrechenbare Zeit incrementieren", this);
  abzurMin5MinusAction = new QAction(QPixmap((const char **)hi22_action_1downarrow_half ),
                                       "Abrechenbare Zeit decrementieren", this);

  fastAbzurPlusAction = new QAction(QPixmap((const char **)hi22_action_2uparrow_half ),
                                      "Abrechenbare Zeit schnell incrementieren", this);
  fastAbzurMinusAction = new QAction(QPixmap((const char **)hi22_action_2downarrow_half ),
                                       "Abrechenbare Zeit schnell decrementieren", this);

  connect(kontoTree, SIGNAL(currentChanged(Q3ListViewItem * )), this, SLOT(changeShortCutSettings(Q3ListViewItem * ) ));

  connect(min5PlusAction, SIGNAL(triggered()), this, SLOT(addTimeInc()));
  connect(min5MinusAction, SIGNAL(triggered()), this, SLOT(subTimeInc()));
  connect(fastPlusAction, SIGNAL(triggered()), this, SLOT(addFastTimeInc()));
  connect(fastMinusAction, SIGNAL(triggered()), this, SLOT(subFastTimeInc()));

  connect(this,SIGNAL(eintragSelected(bool)), min5PlusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), min5MinusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), fastPlusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), fastMinusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(augmentableItemSelected(bool)), eintragAddAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(unterkontoSelected(bool)), bereitschaftsAction, SLOT(setEnabled(bool)));

  connect(this,SIGNAL(aktivierbarerEintragSelected(bool)), eintragActivateAction, SLOT(setEnabled(bool)));

  toolBar->addAction(editUnterKontoAction);
  toolBar->addAction(saveAction);
  toolBar->addAction(inPersKontAction);
  toolBar->addAction(eintragAddAction);
  toolBar->addAction(bereitschaftsAction);
  toolBar->addAction(pauseAction);
  toolBar->addAction(min5PlusAction);
  toolBar->addAction(min5MinusAction);
  toolBar->addAction(fastPlusAction);
  toolBar->addAction(fastMinusAction);
  kontomenu->addAction(editUnterKontoAction);
  kontomenu->addAction(eintragActivateAction);
  kontomenu->addAction(eintragAddAction);
  kontomenu->addAction(eintragRemoveAction);
  kontomenu->addAction(saveAction);
  kontomenu->addAction(pauseAction);
  kontomenu->addAction(pauseAbzurAction);
  kontomenu->addAction(findKontoAction);
  kontomenu->addAction(refreshAction);
  kontomenu->addAction(bereitschaftsAction);
  kontomenu->insertSeparator();
  kontomenu->addAction(quitAction);
  zeitmenu->addAction(changeDateAction);
  zeitmenu->addAction(resetAction);
#ifndef NO_CHECKIN_ACTION
  zeitmenu->addAction(checkInAction);
#endif
  settingsmenu->addAction(defaultCommentAction);
  settingsmenu->addAction(preferenceAction);
  #ifndef NO_TEXTEDIT
  hilfemenu->addAction(helpAction);
  #endif
  hilfemenu->addAction(aboutAction);

  addToolBar(toolBar);

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
      powerToolBar = addToolBar("Power Buttons");
      powerToolBar->setIconSize(toolBar->iconSize());
      powerToolBar->addAction(abzurMin5PlusAction);
      powerToolBar->addAction(abzurMin5MinusAction);
      powerToolBar->addAction(fastAbzurPlusAction);
      powerToolBar->addAction(fastAbzurMinusAction);
      connect(abzurMin5PlusAction, SIGNAL(triggered()), this, SLOT(addAbzurTimeInc()));
      connect(abzurMin5MinusAction, SIGNAL(triggered()), this, SLOT(subAbzurTimeInc()));
      connect(fastAbzurPlusAction, SIGNAL(triggered()), this, SLOT(addFastAbzurTimeInc()));
      connect(fastAbzurMinusAction, SIGNAL(triggered()), this, SLOT(subFastAbzurTimeInc()));
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
    disconnect(kontoTree, SIGNAL(mouseButtonClicked ( int, Q3ListViewItem * , const QPoint & , int )),
               this, SLOT(mouseButtonInKontoTreeClicked(int, Q3ListViewItem * , const QPoint &, int )));
    disconnect(kontoTree, SIGNAL(doubleClicked(Q3ListViewItem *)),
               this, SLOT(callUnterKontoDialog(Q3ListViewItem *)) );
    disconnect(kontoTree, SIGNAL(doubleClicked(Q3ListViewItem *)),
               this, SLOT(setAktivesProjekt(Q3ListViewItem *)));
    disconnect(kontoTree, SIGNAL(contextMenuRequested(Q3ListViewItem *, const QPoint& ,int)),
               this, SLOT(callUnterKontoDialog(Q3ListViewItem *)));

    if (!singleClickActivation) {
        connect(kontoTree, SIGNAL(contextMenuRequested(Q3ListViewItem *, const QPoint& ,int)),
                this, SLOT(callUnterKontoDialog(Q3ListViewItem *)) );
        connect(kontoTree, SIGNAL(doubleClicked(Q3ListViewItem *)),
                this, SLOT(setAktivesProjekt(Q3ListViewItem *)));
        }
    else {
        connect(kontoTree, SIGNAL(mouseButtonClicked ( int, Q3ListViewItem * , const QPoint & , int )),
                   this, SLOT(mouseButtonInKontoTreeClicked(int, Q3ListViewItem * , const QPoint &, int )));
        connect(kontoTree, SIGNAL(doubleClicked(Q3ListViewItem *)),
                this, SLOT(callUnterKontoDialog(Q3ListViewItem *)) );
    }
}

void TimeMainWindow::copyNameToClipboard()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText( kontoTree->currentItem()->text(0), QClipboard::Clipboard );
}

void TimeMainWindow::mouseButtonInKontoTreeClicked(int button, Q3ListViewItem * item, const QPoint & pos, int c)
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
    int delta=lastMinuteTick.secsTo(QDateTime::currentDateTime());    
    if ((delta<120)&&(delta>0)) // Check if we have won or lost a minute.
      abtListToday->minuteVergangen(!pausedAbzur);
    else
    {
      QFile logFile(configDir+"/sctime.log");

      if (logFile.open(QIODevice::Append)) {
         QTextStream stream(&logFile);
         stream<<"Zeitinkonsistenz am "<<QDateTime::currentDateTime().toString()<<" Dauer: "<<QString::number(delta/60-1)<<" Minuten.\n";
      }
      logFile.close();

      QString extrawarnung="";
      if (delta<0)
        extrawarnung="\nACHTUNG: Die Zeit wird zurueckgestellt, wenn Sie mit Ja quittieren!!!";
      lastMinuteTick=QDateTime::currentDateTime(); // we might spend some time in the dialog... set things back to normal
      int answer= QMessageBox::question(this, "Zeitinkonsistenz",
                                   QString("Das System schien fuer ")+QString::number(delta/60)+" Minuten zu haengen oder die Systemzeit wurde veraendert.\n"
                                   "Soll die entstandene Zeitdifferenz auf das aktive Unterkonto gebucht werden?"+extrawarnung, QMessageBox::Yes,QMessageBox::No);
      if (answer==QMessageBox::Yes)
        abtListToday->changeZeit(abt, ko, uko, idx, delta, false);
    }
    kontoTree->refreshItem(abt,ko,uko,idx);
    zeitChanged();
    emit minuteTick();
    if (!pausedAbzur) emit minuteAbzurTick();
  }
  lastMinuteTick=QDateTime::currentDateTime();

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
  Q3ListViewItem * item=kontoTree->currentItem();

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
  int max_working_time=settings->maxWorkingTime();
  abtList->getGesamtZeit(zeit, zeitAbzur);
  TimeCounter tc(zeit);
  setIconText(tc.toString());
  statusBar->setDiff(abtList->getZeitDifferenz());
  emit gesamtZeitChanged(zeit);
  emit gesamtZeitAbzurChanged(zeitAbzur);
  // Beim ersten ueberschreiten von MAX_WORKTIME
  if ((zeit>max_working_time)&&(last<=max_working_time)) {
    // last muss _vor_ dem oeffnen der Messagebox gesetzt werden,
    // da es andernfalls erst nach dem Schliessen der Box gesetzt wird, was bedeuten wuerde,
    // dass (falls der user nicht sofort reagiert), jede Minute eine neue Box aufpoppt
    // => nix gut am naechsten morgen, wenn man das ausloggen vergisst :-)
    last=zeit;
    QMessageBox::warning(0,"Warnung","Warnung: die gesetzlich zulässige Arbeitszeit wurde überschritten.",
                       QMessageBox::Ok | QMessageBox::Default,0);
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
  QMessageBox::warning(0,"Pause Dialog","Die Zeiterfassung wurde angehalten. Ende der Pause mit OK.",
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
  std::vector<int> columnwidthlist;
  kontoTree->getColumnWidthList(columnwidthlist);
  settings->setColumnWidthList(columnwidthlist);

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

void TimeMainWindow::editBereitschaftPressed()
{
  emit callBereitschaftsDialog(kontoTree->currentItem());
}


/**
 * Aktiviert einen Eintrag
 */
void TimeMainWindow::eintragAktivieren()
{
  Q3ListViewItem * item=kontoTree->currentItem();
  setAktivesProjekt(item);
}


/**
 * Fuegt einen Eintrag zum selektierten Unterkonto hinzu.
 */
void TimeMainWindow::eintragHinzufuegen()
{
  Q3ListViewItem * item=kontoTree->currentItem();

  if ((!kontoTree->isEintragsItem(item))&&(!kontoTree->isUnterkontoItem(item))) return;

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
  Q3ListViewItem * item=kontoTree->currentItem();

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
  if (kontoTree->sucheItem(top,abt,ko,uko,idx,topi,abti,koi,ukoi,eti)) {
      for (eti=(KontoTreeItem*)(ukoi->firstChild());
           (eti!=NULL)&&(eti->text(0).toInt()<=idx);
           eti=(KontoTreeItem*)(eti->nextSibling()));
      if (eti!=NULL)
          kontoTree->setCurrentItem(eti);
      else
          kontoTree->setCurrentItem(ukoi);
  }
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
  std::vector<int> columnwidthlist;
  kontoTree->getColumnWidthList(columnwidthlist);
  settings->setColumnWidthList(columnwidthlist);
  if (abtListToday!=abtList) {
    settings->writeSettings(abtListToday);
    settings->writeShellSkript(abtListToday);
    settings->writeSettings(abtList);
    settings->writeShellSkript(abtList);    
    //if (currentDateSel) {
      delete abtList;
      abtList=NULL;
    //}
  } else {
  	settings->writeSettings(abtList);
    settings->writeShellSkript(abtList);
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
  std::vector<int> columnwidthlist;
  kontoTree->getColumnWidthList(columnwidthlist);
  settings->setColumnWidthList(columnwidthlist);
  settings->writeSettings(abtList); // die Settings ueberstehen das Reload nicht
  int diff = abtList->getZeitDifferenz();
  abtList->reload();
  settings->readSettings(abtList);
  if (abtList!=abtListToday) {
    settings->writeSettings(abtListToday); // die Settings ueberstehen das Reload nicht
    abtListToday->reload();
    settings->readSettings(abtListToday);
  }
  kontoTree->load(abtList);
  kontoTree->closeFlaggedPersoenlicheItems();
  abtList->setZeitDifferenz(diff);
}

void TimeMainWindow::reloadDefaultComments()
{
  std::vector<QString> xmlfilelist;
  settings->getDefaultCommentFiles(xmlfilelist);
  if (abtList!=abtListToday) {
    abtListToday->clearDefaultComments();
    defaultCommentReader.read(abtListToday,xmlfilelist);
  }
  abtList->clearDefaultComments();
  defaultCommentReader.read(abtList,xmlfilelist);
}

/**
 * Fuegt das aktuell selektierte Unterkonto den Persoenlichen Konten hinzu.
 * Falls kein Unterkonto selektiert oder inPersoenlicheKonten==false ist, passiert nichts.
 */
void TimeMainWindow::inPersoenlicheKonten(bool hinzufuegen)
{

  if (!inPersoenlicheKontenAllowed) return;

  Q3ListViewItem * item=kontoTree->currentItem();

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
void TimeMainWindow::changeShortCutSettings(Q3ListViewItem * item)
{
  bool iseintragsitem=kontoTree->isEintragsItem(item);
  bool isUnterkontoItem=kontoTree->isUnterkontoItem(item);
  inPersoenlicheKontenAllowed=false; //Vorsorglich disablen, sonst Seiteneffekte mit flagsChanged.
  inPersKontAction->setEnabled(false);

  QString uko,ko,abt;
  QString top=""; // top wird weiter unten ausgelesen, und es ist nicht sicher, ob es initialisiert wurde.
  int idx;

  if (item) kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  emit unterkontoSelected(isUnterkontoItem);

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
    emit augmentableItemSelected(!abtList->checkInState());
    if (abtListToday==abtList)
      emit aktivierbarerEintragSelected(!abtList->checkInState());
  }
  else {
    // Auch bei Konten und Unterkonten in Pers. Konten PersKontAction auf On stellen.
    inPersKontAction->setOn((item&&(top==PERSOENLICHE_KONTEN_STRING)&&(item->depth()>=2)&&(item->depth()<=3)));
    inPersKontAction->setEnabled((!abtList->checkInState())&&(item&&(item->depth()>=2)&&(item->depth()<=3)));
    editUnterKontoAction->setEnabled(false);
    emit eintragSelected(false);
    emit augmentableItemSelected(!abtList->checkInState()&&isUnterkontoItem);
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
   abtList->setZeitDifferenz(0);
   zeitChanged();
}

/**
 * Sollte aufgerufen werden, sobald sich die Einstellungen fuer ein Konto aendern.
 * Toggelt zB inPersKontAction.
 */
void TimeMainWindow::flagsChanged(const QString& abt, const QString& ko, const QString& uko, int idx)
{

  Q3ListViewItem * item=kontoTree->currentItem();

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
    QMessageBox::critical(0,"Fehler","Heutiges Datum kann nicht über die GUI eingecheckt werden.\nZeiten nicht eingecheckt!",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  }
  if (abtList->checkInState()) {
    QMessageBox::critical(0,"Fehler","Ausgewähltes Datum ist bereits eingecheckt.\nZeiten nicht eingecheckt!",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  }
  settings->writeSettings(abtList);
  settings->writeShellSkript(abtList);

  // do checkin
  if (!abtList->checkIn()) {
    QMessageBox::critical(0,"Fehler","Fehler beim einchecken.\nZeiten nicht eingecheckt!",
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
void TimeMainWindow::callUnterKontoDialog(Q3ListViewItem * item)
{
  if ((!kontoTree->isEintragsItem(item))||(abtList->checkInState()))
    return;

  QString top,uko,ko,abt;

  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  unterKontoDialog=new UnterKontoDialog(abt,ko,uko,idx,abtList,&defaultTags, true ,this);
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int )), kontoTree,
  SLOT(refreshItem(const QString&, const QString&, const QString&,int )));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int)), this, SLOT(zeitChanged()));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int)),
           this, SLOT(flagsChanged(const QString&, const QString&, const QString&,int)));
  connect(unterKontoDialog, SIGNAL(entryActivated()), this, SLOT(eintragAktivieren()));
  connect(unterKontoDialog, SIGNAL(bereitschaftChanged(const QString&, const QString&, const QString&)),
          kontoTree, SLOT(refreshAllItemsInUnterkonto(const QString&, const QString&, const QString&)));
  if (abtList->isAktiv(abt,ko,uko,idx) && (abtList->getDatum()==QDate::currentDate()))
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
    QMessageBox::warning(0,"Konto nicht gefunden","Das Konto "+konto+" konnte nicht gefunden werden.",
                       QMessageBox::Ok | QMessageBox::Default,0);
    return;
  }

  Q3ListViewItem *item = kontoTree->sucheKontoItem(ALLE_KONTEN_STRING,abt,konto);
  if (item) {
    kontoTree->setCurrentItem(item);
    kontoTree->ensureItemVisible(item);
  }
}

void TimeMainWindow::callPreferenceDialog()
{
  bool oldshowtypecolumn = settings->showTypeColumn();
	
  PreferenceDialog preferenceDialog(settings, this);
  preferenceDialog.exec();
  showAdditionalButtons(settings->powerUserView());
  configClickMode(settings->singleClickActivation());
  if (settings->useCustomFont()) {
  	QApplication::setFont(QFont(settings->customFont(),settings->customFontSize()));
  }
  else
  {
    QApplication::setFont(qtDefaultFont);
  }
  if (!settings->showTypeColumn()) {
  	kontoTree->hideColumn(1);
  	kontoTree->header()->setResizeEnabled(false, 1);
  } else
  {
  	if (!oldshowtypecolumn)
    	kontoTree->header()->setResizeEnabled(true, 1);
    	kontoTree->adjustColumn(1);
  }
}

/**
 * Setzt das zu Item gehoerende Unterkonto als aktiv.
 */
void TimeMainWindow::setAktivesProjekt(Q3ListViewItem * item)
{
  bool currentDateSel = (abtList->getDatum()==QDate::currentDate());
  if (!kontoTree->isEintragsItem(item)) return;
  if (!currentDateSel) {
    abtList->setAsAktiv("","","",0);
    return;
  }

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
  Q3TextEdit * helpBrowser = new Q3TextEdit(helpDialog,"Anleitung");

  helpBrowser->setMimeSourceFactory(mimeSourceFactory);
  helpBrowser->setTextFormat(Qt::RichText);
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
  aboutBox->setPaletteBackgroundColor(Qt::white);
  QGridLayout* layout=new QGridLayout(aboutBox,7,3);
  QLabel* logo=new QLabel(aboutBox);
  logo->setPixmap(QPixmap((const char **)scLogo_15Farben_xpm));
  // huh?!? why do we need these strange numbers? I guess its a bug in qt.
  layout->setRowMinimumHeight(0,logo->pixmap()->height()+69);

  layout->addWidget(logo,0,0);
  QLabel versioninfo(QString("<h2>sctime</h2><nobr><b>Version:</b> ")+VERSIONSTR+
                             "</nobr><br><nobr><b>Datum des Builds:</b> "+BUILDDATESTR+"</nobr>",aboutBox);
  versioninfo.setTextFormat(Qt::RichText);
  layout->addWidget(&versioninfo,0,1);
  layout->addItem(new QSpacerItem(0, 20), 1, 0);
  layout->addWidget(new QLabel("Core Developer:",aboutBox),2,0);
  layout->addWidget(new QLabel("Florian Schmitt <f.schmitt@science-computing.de>",aboutBox),2,1);
  layout->addWidget(new QLabel("Patches:",aboutBox),3,0);
  layout->addWidget(new QLabel("Marcus Camen <m.camen@science-computing.de>",aboutBox),3,1);
  layout->addItem(new QSpacerItem(0, 18), 4, 0);
  layout->addMultiCellWidget(new QLabel("<center>This Program is licensed under the GNU Public License.</center>",aboutBox),5,5,0,1);
  layout->addItem(new QSpacerItem(0, 18), 6, 0);

  QHBoxLayout* buttonlayout=new QHBoxLayout();
  QPushButton * okbutton=new QPushButton( "OK", aboutBox);

  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addStretch(1);
  layout->addMultiCellLayout(buttonlayout,7,7,0,1);
  connect (okbutton, SIGNAL(clicked()), aboutBox, SLOT(close()));
  layout->addRowSpacing(8,10);
  layout->addItem(new QSpacerItem(0, 10), 8, 0);

  aboutBox->exec();

}

void TimeMainWindow::callBereitschaftsDialog(Q3ListViewItem * item)
{
  if ((!kontoTree->isUnterkontoItem(item))||(abtList->checkInState()))
    return;

  QString top,uko,ko,abt;

  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  UnterKontoListe *ukl;
  UnterKontoListe::iterator ukiter;

  if (!abtList->findUnterKonto(ukiter, ukl, abt, ko, uko)) {
    std::cerr<<"Unterkonto nicht gefunden!"<<std::endl;
    return;
  }

  QDialog dialog(this);
  dialog.setWindowTitle("Bereitschaftszeiten");

  QVBoxLayout* layout=new QVBoxLayout(&dialog);
  layout->setMargin(15);

  QPushButton * okbutton=new QPushButton("OK", &dialog);
  okbutton->setDefault(true);
  QPushButton * cancelbutton=new QPushButton("Abbruch", &dialog);
  
  layout->addStretch(1);
  

  QLabel* infolabel=new QLabel (QString("Bitte wählen Sie die geleisteten Bereitschaften für dieses ")+
          "Unterkonto aus.", &dialog);
  infolabel->setWordWrap(true);
  layout->addWidget(infolabel);
  
  layout->addSpacing(10);

  QStringList bereitschaften = ukiter->second.getBereitschaft();
  BereitschaftsView* bereitschaftsView=new BereitschaftsView(&dialog);
  bereitschaftsView->setSelectionList(bereitschaften);
  layout->addWidget(bereitschaftsView);

  QHBoxLayout* buttonlayout=new QHBoxLayout(layout,3);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addWidget(cancelbutton);

  connect (okbutton, SIGNAL(clicked()), &dialog, SLOT(accept()));
  connect (cancelbutton, SIGNAL(clicked()), &dialog, SLOT(reject()));

  dialog.exec();
  if (dialog.result())
  {
    QStringList bereitschaftenNeu = bereitschaftsView->getSelectionList();
    if (bereitschaften!=bereitschaftenNeu) {
      ukiter->second.setBereitschaft(bereitschaftenNeu);
      kontoTree->refreshAllItemsInUnterkonto(abt,ko,uko);
    }
  }
}
