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

#include "timemainwindow.h"

#include <QTextCodec>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QHeaderView>
#include <QFont>
#include <QTextStream>
#include <QInputDialog>
#include <QTimer>
#include <QMessageBox>
#include <QStringList>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QDir>
#include <QPoint>
#include <QRect>
#include <QFile>
#include <QVariant>
#include <QDebug>
#include <QToolBar>
#include <QColorDialog>
#include <QtMsgHandler>
#include "globals.h"
#include "time.h"
#include "preferencedialog.h"
#ifndef HAS_NO_DATETIMEEDIT
#include "datedialog.h"
#endif
#include <QTextBrowser>
#include "findkontodialog.h"
#include "defaulttagreader.h"
#ifndef WIN32
#include "kontodateninfozeit.h"
#else
#include "kontodateninfodatabase.h"
#endif
#include "statusbar.h"
#include "kontotreeitem.h"
#include "bereitschaftsliste.h"
#include "bereitschaftsview.h"
#include "globals.h"
#include "timeedit.h"
#include "kontotreeview.h"
#include "bereitschaftsdateninfo.h"
#include "unterkontodialog.h"
#include "kontotreeview.h"
#include "defaultcommentreader.h"
#include "abteilungsliste.h"
#include "sctimexmlsettings.h"
#include "lock.h"


QTreeWidget* TimeMainWindow::getKontoTree() { return kontoTree; };

static QString logText("-- Start --");
static void printMessage(QtMsgType type, const char *msg) {
  logText.append(type).append(": ").append(msg).append("\n");
}

/** Erzeugt ein neues TimeMainWindow, das seine Daten aus abtlist bezieht. */
TimeMainWindow::TimeMainWindow(KontoDatenInfo* zk, BereitschaftsDatenInfo* bereitschaftsdatenReader):QMainWindow()
{
  setObjectName("sctime");
  qInstallMsgHandler(printMessage);
  connect(zk, SIGNAL(kontoListeGeladen()), this, SLOT(aktivesKontoPruefen()), Qt::QueuedConnection);
  std::vector<QString> xmlfilelist;
  QDate heute;
  abtListToday=new AbteilungsListe(heute.currentDate(), zk);
  abtList=abtListToday;
  paused=false;
  pausedAbzur=false;
  inPersoenlicheKontenAllowed=true;
  powerToolBar = NULL;
  settings=new SCTimeXMLSettings();
  settings->readSettings(abtList);

  this->zk = zk;
  settings->getDefaultCommentFiles(xmlfilelist);
  qtDefaultFont=QApplication::font();
  if (settings->useCustomFont())
  {
    QString custFont=settings->customFont();
    int custFontSize=settings->customFontSize();
    QApplication::setFont(QFont(custFont,custFontSize));
  }

  defaultCommentReader.read(abtList,xmlfilelist);

  bereitschaftsdatenReader->readInto(BereitschaftsListe::getInstance());

  DefaultTagReader defaulttagreader;
  defaulttagreader.read(&defaultTags);

  // restore size+position
  QSize size = QSize(700,400);
  QPoint pos = QPoint(0,0);
  settings->getMainWindowGeometry(pos,size);
  resize(size);
  move(pos);

  statusBar = new StatusBar(this);
  setStatusBar(statusBar);
  std::vector<int> columnwidthlist;

  settings->getColumnWidthList(columnwidthlist);

  kontoTree=new KontoTreeView( this, abtList, columnwidthlist );
  kontoTree->closeFlaggedPersoenlicheItems();
  kontoTree->showPersoenlicheKontenSummenzeit(settings->persoenlicheKontensumme());
#ifndef Q_WS_MAC
  setWindowIcon(QIcon(":/sc_logo"));  
#endif

  setCentralWidget(kontoTree);

  if (!settings->showTypeColumn()) {
    kontoTree->hideColumn(1);
  }

  toolBar   = new QToolBar("Main ToolBar", this);
  toolBar->setIconSize(QSize(22,22));

  configClickMode(settings->singleClickActivation());

  QMenu * kontomenu = menuBar()->addMenu("&Konto");
  QMenu * zeitmenu = menuBar()->addMenu("&Zeit");
  QMenu * settingsmenu = menuBar()->addMenu("&Einstellungen");
  QMenu * hilfemenu = menuBar()->addMenu("&Hilfe");

  QTimer* timer = new QTimer(this);
  connect( timer,SIGNAL(timeout()), this, SLOT(minutenUhr()));
  timer->setInterval(60000); //Alle 60 Sekunden ticken
  timer->start();
  lastMinuteTick=QDateTime::currentDateTime();

  QTimer* autosavetimer=new QTimer(this);
  connect( autosavetimer,SIGNAL(timeout()), this, SLOT(save()));
  autosavetimer->setInterval(300000); //Alle 5 Minuten ticken.
  autosavetimer->start();
  //QAction* pauseAction = new QAction( QPixmap((const char **)hi22_action_player_pause ),
                                      //"&Pause", this);
  QAction* pauseAction = new QAction( QIcon(":/hi22_action_player_pause"), "&Pause", this);
  pauseAction->setShortcut(Qt::CTRL+Qt::Key_P);
  connect(pauseAction, SIGNAL(triggered()), this, SLOT(pause()));

  QAction* pauseAbzurAction = new QAction( QIcon(":/hi22_action_player_pause_half"),
                                           "Pause der &abzur. Zeit", this);
  pauseAbzurAction->setShortcut(Qt::CTRL+Qt::Key_A);
  pauseAbzurAction->setStatusTip(tr("Hält nur die Uhr für die abzurechnende Zeit an"));
  pauseAbzurAction->setCheckable(true);
  connect(pauseAbzurAction, SIGNAL(toggled(bool)), this, SLOT(pauseAbzur(bool)));

  QAction* saveAction = new QAction( QIcon(":/hi22_action_filesave" ), tr("&Speichern"), this);
  saveAction->setShortcut(Qt::CTRL+Qt::Key_S);
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  QAction* copyAction = new QAction(tr("&Copy"), this);
  copyAction->setShortcut(Qt::CTRL+Qt::Key_C);
  copyAction->setStatusTip("Name ins Clipboard kopieren");
  connect(copyAction, SIGNAL(triggered()), this, SLOT(copyNameToClipboard()));

  QAction* changeDateAction = new QAction(tr("&Datum wählen..."), this);
  changeDateAction->setShortcut(Qt::CTRL+Qt::Key_D);
  connect(changeDateAction, SIGNAL(triggered()), this, SLOT(callDateDialog()));

  QAction* resetAction = new QAction( "&Differenz auf Null", this);
  resetAction->setShortcut(Qt::CTRL+Qt::Key_N);
  resetAction->setStatusTip(tr("Beim gewählten Unterkonto die abzurechnenden auf die geleisteten Stunden setzen"));
  connect(resetAction, SIGNAL(triggered()), this, SLOT(resetDiff()));

  inPersKontAction = new QAction( QIcon(":/hi22_action_attach"), tr("In persönliche &Konten"), this);
  inPersKontAction->setShortcut(Qt::CTRL+Qt::Key_K);
  inPersKontAction->setCheckable(true);
  connect(inPersKontAction, SIGNAL(toggled(bool)), this, SLOT(inPersoenlicheKonten(bool)));

  QAction* quitAction = new QAction(tr("&Beenden"), this);
  // force this item to have the quit role so that Qt properly moves it into
  // the Mac application menu.
  // FIXME: This is a workaround. With proper translation, Qt's heuristic would
  // do this automatically based on the menu item title. Also, translation does
  // not seem to work as this time. The menu tetxts always end up English after
  // being merged into the application menu.
  quitAction->setMenuRole(QAction::QuitRole);
  quitAction->setShortcut(Qt::CTRL+Qt::Key_Q);
  quitAction->setStatusTip(tr("Programm beenden"));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

  QAction* findKontoAction = new QAction(tr("Konto &suchen..."), this);
  findKontoAction->setShortcut(Qt::CTRL+Qt::Key_F);
  //findKontoAction->setStatusTip(tr("Konto suchen"));
  connect(findKontoAction, SIGNAL(triggered()), this, SLOT(callFindKontoDialog()));

  QAction* refreshAction = new QAction(tr("&Kontoliste neu laden"), this);
  refreshAction->setShortcut(Qt::CTRL+Qt::Key_R);
  connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshKontoListe()));

  QAction* preferenceAction = new QAction(tr("&Einstellungen..."),this);
  preferenceAction->setMenuRole(QAction::PreferencesRole);
  connect(preferenceAction, SIGNAL(triggered()), this, SLOT(callPreferenceDialog()));

  QAction* defaultCommentAction = new QAction(tr("&Standardkommentare/Mikrokonten neu einlesen"), this);
  connect(defaultCommentAction, SIGNAL(triggered()), this, SLOT(reloadDefaultComments()));

#ifndef NO_TEXTEDIT
  QAction* helpAction = new QAction(tr("&Anleitung..."), this);
  helpAction->setShortcut(Qt::Key_F1);
  connect(helpAction, SIGNAL(triggered()), this, SLOT(callHelpDialog()));  
#endif

  QAction* aboutAction = new QAction(tr("&Über sctime..."), this);
  aboutAction->setStatusTip(tr("Über sctime..."));
  aboutAction->setMenuRole(QAction::AboutRole);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(callAboutBox()));

  QAction* logAction = new QAction(tr("&Meldungen..."), this);
  connect(logAction, SIGNAL(triggered()), this, SLOT(logDialog()));

  editUnterKontoAction = new QAction(QIcon(":/hi22_action_edit" ), "&Editieren...", this);
  editUnterKontoAction->setStatusTip(tr("Unterkonto editieren"));
  connect(editUnterKontoAction, SIGNAL(triggered()), this, SLOT(editUnterKontoPressed()));

  QAction* eintragActivateAction = new QAction(tr("Eintrag a&ktivieren"), this);
  eintragActivateAction->setShortcut(Qt::CTRL+Qt::Key_X);
  connect(eintragActivateAction, SIGNAL(triggered()), this, SLOT(eintragAktivieren()));

  QAction* eintragAddAction = new QAction(QIcon(":/hi22_action_queue" ),
                                             tr("&Eintrag hinzufügen"), this);
  connect(eintragAddAction, SIGNAL(triggered()), this, SLOT(eintragHinzufuegen()));

  eintragRemoveAction = new QAction(tr("&Eintrag löschen"), this);
  eintragRemoveAction->setShortcut(Qt::Key_Delete);
  connect(eintragRemoveAction, SIGNAL(triggered()), this, SLOT(eintragEntfernen()));

  QAction* bereitschaftsAction = new QAction(QIcon(":/hi16_action_stamp" ),
                                            tr("&Bereitschaftszeiten setzen..."), this);
  bereitschaftsAction->setShortcut(Qt::CTRL+Qt::Key_B);
  connect(bereitschaftsAction, SIGNAL(triggered()), this, SLOT(editBereitschaftPressed()));

  bgColorChooseAction = new QAction(tr("&Hintergrundfarbe wählen..."), this);
  bgColorRemoveAction = new QAction(tr("&Hintergrundfarbe entfernen"), this);

  jumpAction = new QAction("&Zu selektiertem Konto in \"Alle Konten\" springen", this);

  QAction* min5PlusAction = new QAction(QIcon(":/hi22_action_1uparrow" ),
                                          tr("Zeit erhöhen"), this);
  QAction* min5MinusAction = new QAction(QIcon(":/hi22_action_1downarrow" ),
                                            tr("Zeit verringern"), this);

  QAction* fastPlusAction = new QAction(QIcon(":/hi22_action_2uparrow" ),
                                           tr("Zeit schnell erhöhen"), this);
  QAction* fastMinusAction = new QAction(QIcon(":/hi22_action_2downarrow" ),
                                            tr("Zeit schnell verringern"), this);

  abzurMin5PlusAction = new QAction(QIcon(":/hi22_action_1uparrow_half" ),
                                      tr("Abrechenbare Zeit erhöhen"), this);
  abzurMin5MinusAction = new QAction(QIcon(":/hi22_action_1downarrow_half" ),
                                       tr("Abrechenbare Zeit verringern"), this);

  fastAbzurPlusAction = new QAction(QIcon(":/hi22_action_2uparrow_half" ),
                                      tr("Abrechenbare Zeit schnell erhöhen"), this);
  fastAbzurMinusAction = new QAction(QIcon(":/hi22_action_2downarrow_half" ),
                                       tr("Abrechenbare Zeit schnell verringern"), this);

  connect(kontoTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(changeShortCutSettings(QTreeWidgetItem * ) ));

  connect(min5PlusAction, SIGNAL(triggered()), this, SLOT(addTimeInc()));
  connect(min5MinusAction, SIGNAL(triggered()), this, SLOT(subTimeInc()));
  connect(fastPlusAction, SIGNAL(triggered()), this, SLOT(addFastTimeInc()));
  connect(fastMinusAction, SIGNAL(triggered()), this, SLOT(subFastTimeInc()));

  connect(bgColorChooseAction, SIGNAL(triggered()),this, SLOT(callColorDialog()));
  connect(bgColorRemoveAction, SIGNAL(triggered()),this, SLOT(removeBgColor()));
  connect(jumpAction, SIGNAL(triggered()),this, SLOT(jumpToAlleKonten()));

  connect(this,SIGNAL(eintragSelected(bool)), min5PlusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), min5MinusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), fastPlusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(eintragSelected(bool)), fastMinusAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(augmentableItemSelected(bool)), eintragAddAction, SLOT(setEnabled(bool)));
  connect(this,SIGNAL(unterkontoSelected(bool)), bereitschaftsAction, SLOT(setEnabled(bool)));

  connect(this,SIGNAL(aktivierbarerEintragSelected(bool)), eintragActivateAction, SLOT(setEnabled(bool)));
  connect(kontoTree,SIGNAL(customContextMenuRequested(const QPoint & )), this, SLOT(showContextMenu( const QPoint & )));

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
  kontomenu->addSeparator();
  kontomenu->addAction(findKontoAction);
  kontomenu->addAction(jumpAction);
  kontomenu->addAction(refreshAction);
  kontomenu->addAction(defaultCommentAction);
  kontomenu->addSeparator();
  kontomenu->addAction(bgColorChooseAction);
  kontomenu->addAction(bgColorRemoveAction);
  kontomenu->addSeparator();
  kontomenu->addAction(bereitschaftsAction);
  kontomenu->addSeparator();
  kontomenu->addAction(quitAction);
  zeitmenu->addAction(changeDateAction);
  zeitmenu->addAction(resetAction);
  settingsmenu->addAction(preferenceAction);
  #ifndef NO_TEXTEDIT
  hilfemenu->addAction(helpAction);
  #endif
  hilfemenu->addAction(aboutAction);
  hilfemenu->addAction(logAction);

  addToolBar(toolBar);

  zeitChanged();

  changeShortCutSettings(NULL); // Unterkontenmenues deaktivieren...

  updateCaption();
  kontoTree->setAcceptDrops(settings->dragNDrop());

  kontoTree->showAktivesProjekt();
  kontoTree->updateColumnWidth();
  //close the flagged items, needed if "Summe in pers. Konten" is 
  //selected
  kontoTree->closeFlaggedPersoenlicheItems();
  showAdditionalButtons(settings->powerUserView());
  QTimer::singleShot(10, this, SLOT(refreshKontoListe()));
}

void TimeMainWindow::aktivesKontoPruefen(){
  QString a,k, u;
  int i;
  abtList->getAktiv(a, k, u, i);
  EintragsListe::iterator dummy;
  EintragsListe *dummy2;
  if (!abtList->findEintrag(dummy, dummy2, a, k, u, i))
    QMessageBox::warning(
          NULL,
          QObject::tr("sctime: Zeiterfassung gestoppt"),
          tr("Ihr zuletzt aktives Konto war %1/%2. Wahrscheinlich wurde es geschlossen oder umbenannt. "
             "Bitte wählen Sie nun ein neues Konto aus, damit die Zeiterfassung beginnt!").arg(k,u));
}

void TimeMainWindow::closeEvent(QCloseEvent * event)
{
  save();
  QMainWindow::closeEvent(event);
}

/** Destruktor  */
TimeMainWindow::~TimeMainWindow()
{
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
    disconnect(kontoTree, SIGNAL(itemClicked ( QTreeWidgetItem * , int )),
               this, SLOT(mouseButtonInKontoTreeClicked(QTreeWidgetItem * , int )));
    disconnect(kontoTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )),
               this, SLOT(callUnterKontoDialog(QTreeWidgetItem *)) );
    disconnect(kontoTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )),
               this, SLOT(setAktivesProjekt(QTreeWidgetItem *)));
    //disconnect(kontoTree, SIGNAL(contextMenuRequested(QTreeWidgetItem *, const QPoint& ,int)),
    //           this, SLOT(callUnterKontoDialog(QTreeWidgetItem *)));

    if (!singleClickActivation) {
        //connect(kontoTree, SIGNAL(contextMenuRequested(QTreeWidgetItem *, const QPoint& ,int)),
        //        this, SLOT(callUnterKontoDialog(QTreeWidgetItem *)) );
        connect(kontoTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )),
                this, SLOT(setAktivesProjekt(QTreeWidgetItem *)));
        }
    else {
        connect(kontoTree, SIGNAL(itemClicked ( QTreeWidgetItem * , int )),
                   this, SLOT(mouseButtonInKontoTreeClicked(QTreeWidgetItem * , int )));
        connect(kontoTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )),
                this, SLOT(callUnterKontoDialog(QTreeWidgetItem *)) );
    }
}

void TimeMainWindow::copyNameToClipboard()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText( kontoTree->currentItem()->text(0), QClipboard::Clipboard );
}

void TimeMainWindow::mouseButtonInKontoTreeClicked(QTreeWidgetItem * item, int column)
{
    if ( (kontoTree->getCurrentButton() == Qt::LeftButton) &&(item)) {

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

  QDateTime currenttime=QDateTime::currentDateTime();
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
         stream<<"Zeitinkonsistenz am "<<currenttime.toString()<<" Dauer: "<<QString::number(delta/60-1)<<" Minuten.\n";
      }
      logFile.close();

      QString extrawarnung="";
      if (delta<0)
        extrawarnung="\nACHTUNG: Die Zeit wird zurueckgestellt, wenn Sie mit Ja quittieren!!!";
      lastMinuteTick=currenttime; // we might spend some time in the dialog... set things back to normal
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
  lastMinuteTick=currenttime;

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
  QTreeWidgetItem * item=kontoTree->currentItem();

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
  //setIconText(tc.toString());
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
    // OK, QTimer erwartet nun, dass der letzte aufruf zurueckgekehrt ist, bevor
    // der nächste kommen kann. Da wir ueber einen QTimer aufgerufen wurden,
    // und wir weiter Tick-Events bekommen muessen, muessen wir den Arbeitszeitdialog asynchron starten.
    // Das tun wir ueber einen weiteren QTimer (das klappt, weil wir hier einen Wegwerftimer benutzen.
    QTimer::singleShot(0, this, SLOT(showArbeitszeitwarning()));
  }
  else
    last=zeit;
    
#ifdef WIN32
	updateTaskbarTitle(zeit);
#endif
}

#ifdef WIN32
/**
 * On OS Windows the taskbartitle will show the amount of workingtime
 * if sctime is minimized.
 * */
void TimeMainWindow::updateTaskbarTitle(int zeit)
{
	TimeCounter tc(zeit);
  QString text;
  text=tc.toString();
	setWindowIconText("sctime - " + text);
}
#endif

void TimeMainWindow::showArbeitszeitwarning()
{
  QMessageBox::warning(0, "Warnung",tr("Warnung: die gesetzlich zulässige Arbeitszeit wurde überschritten."),
                       QMessageBox::Ok, QMessageBox::Ok);
}

/** Ruft einen modalen Dialog auf, der eine Pause anzeigt, und setzt gleichzeitig
  *  paused auf true, um die Zeiten anzuhalten
  */
void TimeMainWindow::pause()
{
  paused=true;
  QMessageBox::warning(0,"Pause Dialog","Die Zeiterfassung wurde angehalten. Ende der Pause mit OK.",
                       QMessageBox::Ok, QMessageBox::Ok);
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
  checkLock();
  settings->setMainWindowGeometry(pos(),size());
  settings->writeSettings(abtListToday);
  settings->writeShellSkript(abtListToday);
  if (abtList!=abtListToday) {
    settings->writeSettings(abtList);
    settings->writeShellSkript(abtList);
  }
}

void TimeMainWindow::checkLock() {
  if (!lock->check()) {
    QMessageBox msg;
    msg.setText(tr("Das Programm beendet sich in wenigen Sekunden ohne zu speichern."));
    msg.setInformativeText(lock->errorString());
    qDebug() << tr("Das Programm beendet sich ohne zu speichern.") << lock->errorString();
    QTimer::singleShot(10000, this, SLOT(quit()));
    msg.exec();
  }
}

void TimeMainWindow::quit() {
  _exit(1);
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
  QTreeWidgetItem * item=kontoTree->currentItem();
  setAktivesProjekt(item);
}


/**
 * Fuegt einen Eintrag zum selektierten Unterkonto hinzu.
 */
void TimeMainWindow::eintragHinzufuegen()
{
  QTreeWidgetItem * item=kontoTree->currentItem();

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
  QTreeWidgetItem * item=kontoTree->currentItem();

  if ((!item)||(kontoTree->getItemDepth(item)!=4)) return;

  QString top,uko,ko,abt;
  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  KontoTreeItem *topi, *abti, *koi, *ukoi, *eti;

  if (abtList->isAktiv(abt,ko,uko,idx)) {
      QMessageBox::warning(NULL, tr("Warnung"), tr("Kann aktiven Eintrag nicht löschen\n"),
                              QMessageBox::Ok, QMessageBox::NoButton);
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
  int etiCurrent = 0;
  if (kontoTree->sucheItem(top,abt,ko,uko,idx,topi,abti,koi,ukoi,eti)) {
      for (eti=(KontoTreeItem*)(ukoi->child(0));
           (eti!=NULL)&&(eti->text(0).toInt()<=idx);
           eti=(KontoTreeItem*)(eti->child(etiCurrent+=1))) ;
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
    checkLock();
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

  //Append Warning if current file is checked in
  if( !currentDateSel ){
    if(abtList->checkInState())
    {
      statusBar->appendWarning(!currentDateSel, " -- Dieser Tag ist bereits eingecheckt!");
    }
  }
}

void TimeMainWindow::refreshKontoListe() {
  qApp->setOverrideCursor(Qt::WaitCursor);
  statusBar->showMessage(tr("Kontenliste laden..."));
  qApp->processEvents();
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
  statusBar->showMessage(tr("Kontenliste geladen"), 2000);
  qApp->restoreOverrideCursor();
}

void TimeMainWindow::reloadDefaultComments() {
  qApp->setOverrideCursor(Qt::WaitCursor);
  std::vector<QString> xmlfilelist;
  settings->getDefaultCommentFiles(xmlfilelist);
  if (abtList!=abtListToday) {
    abtListToday->clearDefaultComments();
    defaultCommentReader.read(abtListToday,xmlfilelist);
  }
  abtList->clearDefaultComments();
  bool rc = defaultCommentReader.read(abtList,xmlfilelist);
  qApp->restoreOverrideCursor();
  if( zk != NULL )
  {
    #ifdef WIN32
    KontoDatenInfoDatabase* kdib = (KontoDatenInfoDatabase*)zk;
    if( kdib != NULL )
    {
      if(!kdib->readDefaultCommentsInto( abtList ) && !rc)
      {
        QString msg = "Defaultkommentare konnten nicht aus der Datenbank oder der Kommentardatei geladen werden.";
        QMessageBox::warning(this,"Warnung",   msg,
          QMessageBox::Ok, QMessageBox::Ok);
      }
    }
    #endif
    #ifndef WIN32
    KontoDatenInfoZeit* kdiz = (KontoDatenInfoZeit*)zk;
    if( kdiz != NULL )
    {
      if(!kdiz->readDefaultComments(abtList))
      {
        QMessageBox::warning(this,"Warnung",
          "Die Default Kommentare konnten nicht neu gelesen werden.",
          QMessageBox::Ok, QMessageBox::Ok);
      }
    }
    #endif
  }
}

/**
 * Fuegt das aktuell selektierte Unterkonto den Persoenlichen Konten hinzu.
 * Falls kein Unterkonto selektiert oder inPersoenlicheKonten==false ist, passiert nichts.
 */
void TimeMainWindow::inPersoenlicheKonten(bool hinzufuegen)
{

  if (!inPersoenlicheKontenAllowed) return;

  QTreeWidgetItem * item=kontoTree->currentItem();

  if (!item) return;

  QString uko,ko,abt,top;
  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  if (kontoTree->getItemDepth(item)==2) {
    abtList->moveKontoPersoenlich(abt,ko,hinzufuegen);
    kontoTree->refreshAllItemsInKonto(abt,ko);
    return;
  }
  else {
    if (kontoTree->getItemDepth(item)==3) {
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
void TimeMainWindow::changeShortCutSettings(QTreeWidgetItem * item)
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

  int depth=-1;
  if (item)
    depth=kontoTree->getItemDepth(item);

  if (iseintragsitem) {

    if ((depth<=3)||(item->parent()->childCount()<=1))
       eintragRemoveAction->setEnabled(false);
    else {
       eintragRemoveAction->setEnabled(true);
    }

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
    inPersKontAction->setChecked((item&&(top==PERSOENLICHE_KONTEN_STRING)&&(kontoTree->getItemDepth(item)>=2)&&(kontoTree->getItemDepth(item)<=3)));
    inPersKontAction->setEnabled((!abtList->checkInState())&&(item&&(kontoTree->getItemDepth(item)>=2)&&(kontoTree->getItemDepth(item)<=3)));
    editUnterKontoAction->setEnabled(false);
    emit eintragSelected(false);
    emit augmentableItemSelected(!abtList->checkInState()&&isUnterkontoItem);
    emit aktivierbarerEintragSelected(false);
    eintragRemoveAction->setEnabled(false);
  }
  bool customColorIsAllowed = (depth>=1)&&(depth<=3);
  bgColorChooseAction->setEnabled(customColorIsAllowed);
  bgColorRemoveAction->setEnabled(customColorIsAllowed);
  jumpAction->setEnabled((top!=ALLE_KONTEN_STRING)&&(depth>=1));
  inPersoenlicheKontenAllowed=true; // Wieder enablen.
}

void TimeMainWindow::updateCaption()
{
   QString abt, ko, uko;
   int idx;
   abtList->getAktiv(abt,ko,uko,idx);
   setWindowTitle("sctime - "+ abt+"/"+ko+"/"+uko);
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

  QTreeWidgetItem * item=kontoTree->currentItem();

  if (!item) return;

  QString selecteduko,selectedko,selectedabt,selectedtop;
  int selectedidx;

  kontoTree->itemInfo(item,selectedtop,selectedabt,selectedko,selecteduko,selectedidx);
  if ((selectedabt==abt)&&(selectedko==ko)&&(selecteduko==uko)&&(selectedidx==idx)) {
    inPersKontAction->setChecked((abtList->getEintragFlags(abt,ko,uko,idx)&UK_PERSOENLICH)&&(!abtList->checkInState()));
  }

  updateCaption();
}

void TimeMainWindow::showContextMenu(const QPoint& pos)
{
   if (!settings->singleClickActivation())
   {
     callUnterKontoDialog(kontoTree->itemAt(pos));
   }
}

/**
 * Erzeugt einen UnterkontoDialog fuer item.
 */
void TimeMainWindow::callUnterKontoDialog(QTreeWidgetItem * item)
{
  if ((!kontoTree->isEintragsItem(item)))
    return;

  QString top,uko,ko,abt;

  int idx;

  kontoTree->itemInfo(item,top,abt,ko,uko,idx);

  unterKontoDialog=new UnterKontoDialog(abt,ko,uko,idx,abtList,&defaultTags, true ,this, abtList->checkInState());
  unterKontoDialog->setSettings(settings);
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int )), kontoTree,
  SLOT(refreshItem(const QString&, const QString&, const QString&,int )));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int )), this,
  SLOT(checkComment(const QString&, const QString&, const QString&,int )));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int)), this, SLOT(zeitChanged()));
  connect(unterKontoDialog, SIGNAL(entryChanged(const QString&, const QString&, const QString&, int)),
           this, SLOT(flagsChanged(const QString&, const QString&, const QString&,int)));
  connect(unterKontoDialog, SIGNAL(entryActivated()), this, SLOT(eintragAktivieren()));
  connect(unterKontoDialog, SIGNAL(bereitschaftChanged(const QString&, const QString&, const QString&)),
          kontoTree, SLOT(refreshAllItemsInUnterkonto(const QString&, const QString&, const QString&)));
  if (abtList->isAktiv(abt,ko,uko,idx) && (abtList->getDatum()==QDate::currentDate()))
    connect(this, SIGNAL(minuteTick()),unterKontoDialog->getZeitBox(),SLOT(incrMin()));

  QPoint pos;
  QSize size;
  settings->getUnterKontoWindowGeometry(pos, size);
  if( !size.isNull() ){
    unterKontoDialog->resize(size);
    unterKontoDialog->move(pos);
  }
  unterKontoDialog->exec();
}

/**
 * Baut den Kontosuchdialog auf, und zeigt das Such-Ergebnis an.
 */
void TimeMainWindow::callFindKontoDialog()
{
  QString konto;

  FindKontoDialog findKontoDialog(abtList,this);
  int rcFindDialog = findKontoDialog.exec();
  if( rcFindDialog == QDialog::Rejected )
  {
    return;
  }
  else if( rcFindDialog == QDialog::Accepted )
  {
    QStringList items = findKontoDialog.getSelectedItems();

    if( items.size() > 0 )
    {
      //Konto was searched
      if( items.size() == 3 )
      {
        QTreeWidgetItem *item = kontoTree->sucheKontoItem(items.at(0),
              items.at(1), items.at(2));
        openItem( item );
      }
      //Unterkonto was searched
      if( items.size() == 4 )
      {
        QTreeWidgetItem *item = kontoTree->sucheUnterKontoItem(
              items.at(0), items.at(1), items.at(2), items.at(3) );
        openItem( item );
      }
      //Kommentar was searched
      if( items.size() == 5 )
      {
        QTreeWidgetItem *item = kontoTree->sucheKommentarItem(
              items.at(0), items.at(1), items.at(2),
              items.at(3), items.at(4));
        openItem( item );
      }
    }
  }
}

void TimeMainWindow::openItem( QTreeWidgetItem *item )
{
  if (item)
  {
    kontoTree->setCurrentItem(item);
  }
}

void TimeMainWindow::callPreferenceDialog()
{
  bool oldshowtypecolumn = settings->showTypeColumn();

  PreferenceDialog preferenceDialog(settings, this);
  preferenceDialog.exec();
  showAdditionalButtons(settings->powerUserView());
  configClickMode(settings->singleClickActivation());
  kontoTree->setAcceptDrops(settings->dragNDrop());
  if (settings->useCustomFont()) {
    QApplication::setFont(QFont(settings->customFont(),settings->customFontSize()));
  }
  else
  {
    QApplication::setFont(qtDefaultFont);
  }
  kontoTree->showPersoenlicheKontenSummenzeit(settings->persoenlicheKontensumme());
  if (!settings->showTypeColumn()) {
    kontoTree->hideColumn(1);
  } else
  {
    if (!oldshowtypecolumn)
      kontoTree->showColumn( 1 );
      kontoTree->resizeColumnToContents( 1 );

  }
  //Needed if "Summe in pers. Konten" is (de-)selected
  kontoTree->flagClosedPersoenlicheItems();
  kontoTree->closeFlaggedPersoenlicheItems();
  
}

/**
 * Setzt das zu Item gehoerende Unterkonto als aktiv.
 */
void TimeMainWindow::setAktivesProjekt(QTreeWidgetItem * item)
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
  kontoTree->setCurrentItem(item);
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

void TimeMainWindow::callHelpDialog() {
  QDialog *dialog;
  QTextBrowser *browser;
  infoDialog(dialog, browser);
  browser->setSource(QUrl("qrc:/hilfe"));
  dialog->resize(600,450);
  dialog->show();
}

void TimeMainWindow::infoDialog(QDialog *&dialog, QTextBrowser *&browser) {
  dialog = new QDialog(this);
  QVBoxLayout *layout = new QVBoxLayout(dialog);
  browser = new QTextBrowser(this);
  browser->setOpenExternalLinks(true);
  layout->addWidget(browser);
  layout->addSpacing(7);
  QHBoxLayout* buttonlayout=new QHBoxLayout();
  buttonlayout->setContentsMargins(3,3,3,3);
  QPushButton * okbutton=new QPushButton( "OK", dialog);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addStretch(1);
  layout->addLayout(buttonlayout);
  layout->addSpacing(4);
  connect (okbutton, SIGNAL(clicked()), dialog, SLOT(close()));
  dialog->show();
}

void TimeMainWindow::callAboutBox() {
  QDialog *dialog;
  QTextBrowser *browser;
  infoDialog(dialog, browser);
  browser->setHtml(tr(
        "<h1><img src=':/scLogo_15Farben' />sctime</h1>"
        "<table><tr><td>Version:</td><td>%1</td></tr>"
        "<tr><td>Qt-Version:</td><td>%2</td></tr>"
        "<tr><td>Entwickler:</td><td>Johannes Abt, Alexander Wütz, Florian Schmitt</td></tr>"
        "<tr><td>Patches:</td><td>Marcus Camen</td></tr>"
        "<tr><td>Mac:</td><td>Michael Weiser</td></tr>"
        "<tr><td>Projektseite:</td><td><a href='http://sourceforge.net/projects/sctime/'>http://sourceforge.net/projects/sctime/</a></td></tr>"
        "</table><p>Dieses Programm ist unter der GNU Public License v2 lizenziert.</p>").arg(version, QT_VERSION_STR));
  dialog->resize(400, 300);
  dialog->show();
}

void TimeMainWindow::logDialog() {
  QDialog *dialog;
  QTextBrowser *browser;
  infoDialog(dialog, browser);
  browser->setPlainText(logText);
}

void TimeMainWindow::callBereitschaftsDialog(QTreeWidgetItem * item) {
  if ((!kontoTree->isUnterkontoItem(item))||(abtList->checkInState()))
    return;
  QString top,uko,ko,abt;
  int idx;
  kontoTree->itemInfo(item,top,abt,ko,uko,idx);
  UnterKontoListe *ukl;
  UnterKontoListe::iterator ukiter;

  if (!abtList->findUnterKonto(ukiter, ukl, abt, ko, uko)) {
    QMessageBox::critical(this, tr("sctime: Bereitschaftszeiten"), tr("Unterkonto nicht gefunden!"));
    return;
  }

  QDialog dialog(this);
  dialog.setObjectName("sctime: Bereitschaftszeiten");
  dialog.setWindowTitle("Bereitschaftszeiten");

  QVBoxLayout* layout=new QVBoxLayout(&dialog);
  layout->setMargin(15);

  QPushButton * okbutton=new QPushButton("OK", &dialog);
  okbutton->setDefault(true);
  QPushButton * cancelbutton=new QPushButton("Abbruch", &dialog);

  layout->addStretch(1);


  QLabel* infolabel=new QLabel (tr("Bitte wählen Sie die geleisteten Bereitschaften für dieses Unterkonto aus."), &dialog);
  infolabel->setWordWrap(true);
  layout->addWidget(infolabel);

  layout->addSpacing(10);

  QStringList bereitschaften = ukiter->second.getBereitschaft();
  BereitschaftsView* bereitschaftsView=new BereitschaftsView(&dialog);
  bereitschaftsView->setSelectionList(bereitschaften);
  layout->addWidget(bereitschaftsView);

  QHBoxLayout* buttonlayout=new QHBoxLayout();
  //buttonlayout->setParent(layout);
  buttonlayout->setContentsMargins(3,3,3,3);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addWidget(cancelbutton);
  layout->addLayout(buttonlayout);

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

void TimeMainWindow::refreshAfterColorChange(QString& abt, QString& ko, QString& uko) {
	if (ko != "") {
		if(uko != "") {
			kontoTree->refreshAllItemsInUnterkonto(abt,ko,uko);
		} else {
			kontoTree->refreshAllItemsInKonto(abt,ko);
		}
	} else {
		kontoTree->load(abtList);
	}
}

void TimeMainWindow::callColorDialog()
{
   QTreeWidgetItem * item=kontoTree->currentItem();
   // when first started there is no selected item, so we cannot select a color
   // for that
   if (!item)
     return;

   QString top,uko,ko,abt;
   int idx;
   kontoTree->itemInfo(item,top,abt,ko,uko,idx);

   QColor color, initial = Qt::white;
   if (abtList->hasBgColor(abt,ko,uko))
     initial = abtList->getBgColor(abt,ko,uko);
   color = QColorDialog::getColor(initial, this, tr("Hintergrundfarbe"));

   if (color.isValid()) {
     abtList->setBgColor(color, abt,ko,uko);
     refreshAfterColorChange(abt, ko, uko);
   }
}

void TimeMainWindow::removeBgColor() {
   QTreeWidgetItem * item=kontoTree->currentItem();
   QString top,uko,ko,abt;
   int idx;
   kontoTree->itemInfo(item,top,abt,ko,uko,idx);
   abtList->unsetBgColor(abt,ko,uko);
   refreshAfterColorChange(abt, ko, uko);
}

void TimeMainWindow::jumpToAlleKonten()
{
   QTreeWidgetItem * item=kontoTree->currentItem();
   QString top,uko,konto,abt;
   int idx;
   kontoTree->itemInfo(item,top,abt,konto,uko,idx);
   QTreeWidgetItem *newitem = kontoTree->sucheKontoItem(ALLE_KONTEN_STRING,abt,konto);
   if (newitem) {
      kontoTree->setCurrentItem(newitem);
      //kontoTree->currentItem(newitem);
   }
}

void TimeMainWindow::checkComment(const QString& abt, const QString& ko , const QString& uko,int idx) {
  UnterKontoEintrag eintrag;
  if (abtList->getEintrag(eintrag, abt, ko, uko, idx)) {
    QTextCodec *codec = QTextCodec::codecForName("ISO 8859-1");
    if (!codec->canEncode(eintrag.kommentar))
      QMessageBox::warning(
            0,
            tr("Warnung"),
            tr("Warnung: In dem von Ihnen eingegebenen Kommentar kommt ein "
               "Zeichen vor, das mit ISO-8859-1 und somit auf manchen Plattformen nicht darstellbar ist. "
               "Dies führt eventuell zu Problemen mit Auswerteskripten."));
  }
}

void TimeMainWindow::moveEvent(QMoveEvent *event) {
  settings->setMainWindowGeometry(pos(),size());
}
