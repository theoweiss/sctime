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
#include <QFont>
#include <QTimer>
#include <QMessageBox>
#include <QStringList>
#include <QDir>
#include <QPoint>
#include <QFile>
#include <QDebug>
#include <QToolBar>
#include <QColorDialog>
#include <QTextBrowser>
#include <QAction>

#include "globals.h"
#include "time.h"
#include "preferencedialog.h"
#include "datedialog.h"
#include "findkontodialog.h"
#include "defaulttagreader.h"
#include "statusbar.h"
#include "kontotreeitem.h"
#include "bereitschaftsliste.h"
#include "bereitschaftsview.h"
#include "globals.h"
#include "timeedit.h"
#include "kontotreeview.h"
#include "unterkontodialog.h"
#include "kontotreeview.h"
#include "defaultcommentreader.h"
#include "abteilungsliste.h"
#include "sctimexmlsettings.h"
#include "lock.h"
#include "datasource.h"
#include "setupdsm.h"


QTreeWidget* TimeMainWindow::getKontoTree() { return kontoTree; }

static QString logTextLastLine(QObject::tr("-- Start --"));
static QString logText(logTextLastLine + "\n");
void trace(const QString &msg) {
  logError(msg);
}

void logError(const QString &msg) {
  logText.append(msg).append("\n");
  logTextLastLine = msg;
}

/** Erzeugt ein neues TimeMainWindow, das seine Daten aus abtlist bezieht. */
TimeMainWindow::TimeMainWindow():QMainWindow(), startTime(QDateTime::currentDateTime()) {
  paused = false;
  sekunden = 0;
  setObjectName(tr("sctime"));
  std::vector<QString> xmlfilelist;
  QDate heute;
  abtListToday=new AbteilungsListe(heute.currentDate(), zk);
  abtList=abtListToday;
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

  toolBar   = new QToolBar(tr("Main toolbar"), this);
  toolBar->setIconSize(QSize(22,22));

  configClickMode(settings->singleClickActivation());

  QMenu * kontomenu = menuBar()->addMenu(tr("&Account"));
  QMenu * zeitmenu = menuBar()->addMenu(tr("&Time"));
  QMenu * settingsmenu = menuBar()->addMenu(tr("&Settings"));
  QMenu * hilfemenu = menuBar()->addMenu(tr("&Help"));

  minutenTimer = new QTimer(this);
  connect( minutenTimer,SIGNAL(timeout()), this, SLOT(minuteHochzaehlen()));
  lastMinuteTick = startTime;
  minutenTimer->setInterval(60000); //Alle 60 Sekunden ticken
  minutenTimer->start();

  autosavetimer=new QTimer(this);
  connect( autosavetimer,SIGNAL(timeout()), this, SLOT(save()));
  autosavetimer->setInterval(300000); //Alle 5 Minuten ticken.
  autosavetimer->start();
  QAction* pauseAction = new QAction( QIcon(":/hi22_action_player_pause"), tr("&Pause"), this);
  pauseAction->setShortcut(Qt::CTRL+Qt::Key_P);
  connect(pauseAction, SIGNAL(triggered()), this, SLOT(pause()));

  QAction* pauseAbzurAction = new QAction( QIcon(":/hi22_action_player_pause_half"),
                                           tr("Pause &accountable time"), this);
  pauseAbzurAction->setShortcut(Qt::CTRL+Qt::Key_A);
  pauseAbzurAction->setStatusTip(tr("Pause only tracking of accountable time"));
  pauseAbzurAction->setCheckable(true);
  connect(pauseAbzurAction, SIGNAL(toggled(bool)), this, SLOT(pauseAbzur(bool)));

  QAction* saveAction = new QAction( QIcon(":/hi22_action_filesave" ), tr("&Save"), this);
  saveAction->setShortcut(Qt::CTRL+Qt::Key_S);
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  QAction* copyAction = new QAction(tr("&Copy"), this);
  copyAction->setShortcut(Qt::CTRL+Qt::Key_C);
  copyAction->setStatusTip(tr("Copy name to clipboard"));
  connect(copyAction, SIGNAL(triggered()), this, SLOT(copyNameToClipboard()));

  QAction* changeDateAction = new QAction(tr("C&hoose Date..."), this);
  changeDateAction->setShortcut(Qt::CTRL+Qt::Key_D);
  connect(changeDateAction, SIGNAL(triggered()), this, SLOT(callDateDialog()));

  QAction* resetAction = new QAction( tr("&Set accountable equal worked"), this);
  resetAction->setShortcut(Qt::CTRL+Qt::Key_N);
  resetAction->setStatusTip(tr("Set active account's accountable time equal worked time"));
  connect(resetAction, SIGNAL(triggered()), this, SLOT(resetDiff()));

  inPersKontAction = new QAction( QIcon(":/hi22_action_attach"), tr("Select as personal &account"), this);
  inPersKontAction->setShortcut(Qt::CTRL+Qt::Key_K);
  inPersKontAction->setCheckable(true);
  connect(inPersKontAction, SIGNAL(toggled(bool)), this, SLOT(inPersoenlicheKonten(bool)));

  QAction* quitAction = new QAction(tr("&Quit"), this);
  // force this item to have the quit role so that Qt properly moves it into
  // the Mac application menu.
  // FIXME: This is a workaround. With proper translation, Qt's heuristic would
  // do this automatically based on the menu item title. Also, translation does
  // not seem to work as this time. The menu tetxts always end up English after
  // being merged into the application menu.
  quitAction->setMenuRole(QAction::QuitRole);
  quitAction->setShortcut(Qt::CTRL+Qt::Key_Q);
  quitAction->setStatusTip(tr("Quit program"));
  connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

  QAction* findKontoAction = new QAction(tr("&Search account..."), this);
  findKontoAction->setShortcut(Qt::CTRL+Qt::Key_F);
  //findKontoAction->setStatusTip(tr("Konto suchen"));
  connect(findKontoAction, SIGNAL(triggered()), this, SLOT(callFindKontoDialog()));

  QAction* refreshAction = new QAction(tr("&Reread account list"), this);
  refreshAction->setShortcut(Qt::CTRL+Qt::Key_R);
  connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshKontoListe()));

  QAction* preferenceAction = new QAction(tr("&Settings..."),this);
  preferenceAction->setMenuRole(QAction::PreferencesRole);
  connect(preferenceAction, SIGNAL(triggered()), this, SLOT(callPreferenceDialog()));

  QAction* helpAction = new QAction(tr("&Manual..."), this);
  helpAction->setShortcut(Qt::Key_F1);

  connect(helpAction, SIGNAL(triggered()), this, SLOT(callHelpDialog()));  
  QAction* aboutAction = new QAction(tr("&About sctime..."), this);
  aboutAction->setStatusTip(tr("About sctime..."));
  aboutAction->setMenuRole(QAction::AboutRole);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(callAboutBox()));

  QAction* qtAction = new QAction(tr("About &Qt..."), this);
  qtAction->setMenuRole(QAction::AboutQtRole);
  connect(qtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  QAction* logAction = new QAction(tr("&Messages..."), this);
  connect(logAction, SIGNAL(triggered()), this, SLOT(logDialog()));

  editUnterKontoAction = new QAction(QIcon(":/hi22_action_edit" ), tr("&Edit..."), this);
  editUnterKontoAction->setStatusTip(tr("Edit subaccount"));
  connect(editUnterKontoAction, SIGNAL(triggered()), this, SLOT(editUnterKontoPressed()));

  QAction* eintragActivateAction = new QAction(tr("&Activate entry"), this);
  eintragActivateAction->setShortcut(Qt::CTRL+Qt::Key_X);
  connect(eintragActivateAction, SIGNAL(triggered()), this, SLOT(eintragAktivieren()));

  QAction* eintragAddAction = new QAction(QIcon(":/hi22_action_queue" ),
                                             tr("Add &entry"), this);
  connect(eintragAddAction, SIGNAL(triggered()), this, SLOT(eintragHinzufuegen()));

  eintragRemoveAction = new QAction(tr("&Delete entry"), this);
  eintragRemoveAction->setShortcut(Qt::Key_Delete);
  connect(eintragRemoveAction, SIGNAL(triggered()), this, SLOT(eintragEntfernen()));

  QAction* bereitschaftsAction = new QAction(QIcon(":/hi16_action_stamp" ),
                                            tr("Set &on-call times..."), this);
  bereitschaftsAction->setShortcut(Qt::CTRL+Qt::Key_B);
  connect(bereitschaftsAction, SIGNAL(triggered()), this, SLOT(editBereitschaftPressed()));

  bgColorChooseAction = new QAction(tr("Choose &background colour..."), this);
  bgColorRemoveAction = new QAction(tr("&Remove background colour"), this);

  jumpAction = new QAction(tr("&Show selected account in 'all accounts'"), this);

  QAction* min5PlusAction = new QAction(QIcon(":/hi22_action_1uparrow" ),
                                          tr("Increase time"), this);
  QAction* min5MinusAction = new QAction(QIcon(":/hi22_action_1downarrow" ),
                                            tr("Decrease time"), this);

  QAction* fastPlusAction = new QAction(QIcon(":/hi22_action_2uparrow" ),
                                           tr("Increase time fast"), this);
  QAction* fastMinusAction = new QAction(QIcon(":/hi22_action_2downarrow" ),
                                            tr("Decrease time fast"), this);

  abzurMin5PlusAction = new QAction(QIcon(":/hi22_action_1uparrow_half" ),
                                      tr("Increase accountable time"), this);
  abzurMin5MinusAction = new QAction(QIcon(":/hi22_action_1downarrow_half" ),
                                       tr("Decrease accountable time"), this);

  fastAbzurPlusAction = new QAction(QIcon(":/hi22_action_2uparrow_half" ),
                                      tr("Increase accountable time fast"), this);
  fastAbzurMinusAction = new QAction(QIcon(":/hi22_action_2downarrow_half" ),
                                       tr("Decrease accountable time fast"), this);

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
  hilfemenu->addAction(helpAction);
  hilfemenu->addAction(aboutAction);
  hilfemenu->addAction(qtAction);
  hilfemenu->addSeparator();
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
  connect(kontenDSM, SIGNAL(finished(DSResult)), this, SLOT(commitKontenliste(DSResult)));
  connect(bereitDSM, SIGNAL(finished(DSResult)), this, SLOT(commitBereit(DSResult)));
  connect(kontenDSM, SIGNAL(aborted()), this, SLOT(displayLastLogEntry()));
  connect(bereitDSM, SIGNAL(aborted()), this, SLOT(displayLastLogEntry()));
  QMetaObject::invokeMethod(bereitDSM, "start", Qt::QueuedConnection);
  QMetaObject::invokeMethod(this, "refreshKontoListe", Qt::QueuedConnection);
}

void TimeMainWindow::displayLastLogEntry(){
  statusBar->showMessage(logTextLastLine);
  QApplication::restoreOverrideCursor();
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
          QObject::tr("sctime: accounting stopped"),
          tr("The last active account was %1/%2. It seems to have been closed or renamed. "
             "Please activate a new account to start time accounting!").arg(k,u));
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
      powerToolBar = addToolBar(tr("Power Buttons"));
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

    if (!singleClickActivation) {
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

void TimeMainWindow::suspend() {
    // sonst könnten die anderen Timer früher als das Resume-Ereignis eintreffen
    stopTimers(tr("suspend"));
}

void TimeMainWindow::resume() {
  QDateTime now(QDateTime::currentDateTime()), before(lastMinuteTick);
  if (! paused) {
      minutenTimer->start();
      autosavetimer->start();
  }
  int pauseSecs = before.secsTo(now);
  statusBar->showMessage(tr("resume"), 3000);
  trace(tr("resume %2; suspend was %1").arg(lastMinuteTick.toString(), now.toString()));
  if (pauseSecs < 60) return;
  sekunden += pauseSecs; // damit  sich driftKorrektur() nicht beschwert
  lastMinuteTick = now;
  if (pauseSecs > 12 * 3600 * 3600) {
    QMessageBox::information(
        this,  tr("sctime: resume"),
          tr("The machine was suspended from %1 until %2. Please check and adjust accounted time if necessary!")
          .arg(before.toString(), now.toString()));
    return;
  }
  if (QMessageBox::question(
        this, tr("sctime: resume"),
        tr("The machine was suspended from %1 until %2. Should this time be added to the active account?")
        .arg(before.toString(), now.toString()),
	QMessageBox::Yes, QMessageBox::No)
      == QMessageBox::Yes)
    zeitKorrektur(pauseSecs);
}

void TimeMainWindow::zeitKorrektur(int delta) {
  QString abt,ko,uko;
  int idx;
  abtListToday->getAktiv(abt,ko,uko,idx);
  abtListToday->changeZeit(abt, ko, uko, idx, delta, false);
  kontoTree->refreshItem(abt,ko,uko,idx);
  zeitChanged();
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

void TimeMainWindow::driftKorrektur() {
  int drift = startTime.secsTo(lastMinuteTick) - sekunden;
  if (abs(drift) < 60) return;
  QString msg = tr("Drift is %2s (%1)").arg(lastMinuteTick.toString()).arg(drift);
  logError(msg);
  sekunden += drift;
  {
    QFile logFile(configDir.filePath("sctime.log"));
    if (logFile.open(QIODevice::Append)) {
      QTextStream stream(&logFile);
      stream<< msg << endl;
    }
  }
  int answer =
          drift > 0
          ? QMessageBox::question(
                  this, tr("sctime: Programm was frozen"),
                  tr("The program (or whole system) seems to have hung for %1min or system time was changed.\n"
                     "Should the time difference be added to the active account?\n"
                     "(current system time: %2)").arg(drift/60).arg(lastMinuteTick.toString()),
                  QMessageBox::Yes, QMessageBox::No)
          : QMessageBox::question(
                this, tr("sctime: system time set back"),
                tr("The system's time has been set back by %1min to %2."
                   "Should this time be subtracted from the active account?\n"),
                QMessageBox::No, QMessageBox::Yes);
  if (answer == QMessageBox::Yes)
      zeitKorrektur(drift);
}

/* Wird durch einen Timer einmal pro Minute aufgerufen,
und sorgt fuer die korrekte Aktualisierung der Objekte.
Da der Timer weiter laufen soll, muss sich diese Methode
beenden ohne zu blockieren.
*/
void TimeMainWindow::minuteHochzaehlen() {
  sekunden += 60;
  QString abt,ko,uko;
  int idx;
  QDateTime now = QDateTime::currentDateTime();
  int delta = lastMinuteTick.secsTo(now) - 60; // Abweichung seit letzter Minute
  lastMinuteTick = now;
  abtListToday->minuteVergangen(!pausedAbzur);
  // -> notwendig?
  abtListToday->getAktiv(abt,ko,uko,idx);
  kontoTree->refreshItem(abt,ko,uko,idx);
  zeitChanged();
  // <- notwendig?
  if (lastMinuteTick.time().secsTo(QTime(0,2)) > 0)
    tageswechsel();
  if (abs(delta) >= 5)
      logError(tr("Minute-signal %1s arrived late (%2)").arg(delta).arg(now.toString()));
  QMetaObject::invokeMethod(this, "driftKorrektur", Qt::QueuedConnection);
}

void TimeMainWindow::tageswechsel() {
  if (abtList->getDatum().daysTo(lastMinuteTick.date()))
    emit changeDate(QDate::currentDate());
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

void TimeMainWindow::showArbeitszeitwarning() {
  QMessageBox::warning(0, tr("Warning") ,tr("Warning: Legally allowed working time has been exceeded."));
}

void TimeMainWindow::stopTimers(const QString& grund) {
    minutenTimer->stop();
    autosavetimer->stop();
    // die Sekunden seit dem letzten Tick speichern
    QDateTime now = QDateTime::currentDateTime();
    int secSeitTick = lastMinuteTick.secsTo(now);
    if (secSeitTick > 60) 
      logError(grund + tr(": will be ignored (%1)").arg(now.toString()));
    lastMinuteTick = now;
    zeitKorrektur(secSeitTick);
    emit save();
    trace(tr("%1: Accounting stopped (%2, +%3s)").arg(grund, now.toString()).arg(secSeitTick));
}

/** Ruft einen modalen Dialog auf, der eine Pause anzeigt, und setzt gleichzeitig
  *  paused auf true, um die Zeiten anzuhalten
  */
void TimeMainWindow::pause() {
    paused = true;
    stopTimers(tr("Pause"));
    QDateTime pauseBeginn = QDateTime::currentDateTime();
    QMessageBox::warning(this, tr("sctime: Pause"), tr("Accounting has been stopped. Resume work with OK."));
    tageswechsel();
    autosavetimer->start();
    minutenTimer->start();
    paused = false;
    QDateTime now = QDateTime::currentDateTime();
    sekunden += pauseBeginn.secsTo(now);
    lastMinuteTick = now;
    trace(tr("End of break: ") +now.toString());
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
    msg.setText(tr("The program will quit in a few seconds without saving."));
    msg.setInformativeText(lock->errorString());
    qDebug() << tr("The program will now quit without saving.") << lock->errorString();
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
      QMessageBox::warning(NULL, tr("Warning"), tr("Cannot delete active entry"),
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
      statusBar->appendWarning(!currentDateSel, tr(" -- This day has already been checked in!"));
    }
  }
}

void TimeMainWindow::refreshKontoListe() {
  statusBar->showMessage(tr("Reading account list..."));
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  QTimer::singleShot(100, kontenDSM, SLOT(start()));
}

void TimeMainWindow::commitKontenliste(DSResult data) {
  abtList->kontoDatenInfoSuccess = true;
  kontoTree->flagClosedPersoenlicheItems();
  std::vector<int> columnwidthlist;
  kontoTree->getColumnWidthList(columnwidthlist);
  settings->setColumnWidthList(columnwidthlist);
  settings->writeSettings(abtList); // die Settings ueberstehen das Reload nicht
  int diff = abtList->getZeitDifferenz();
  abtList->reload(data);
  settings->readSettings(abtList);
  if (abtList!=abtListToday) {
    settings->writeSettings(abtListToday); // die Settings ueberstehen das Reload nicht
    abtListToday->reload(data);
    settings->readSettings(abtListToday);
  }
  kontoTree->load(abtList);
  kontoTree->closeFlaggedPersoenlicheItems();
  abtList->setZeitDifferenz(diff);
  statusBar->showMessage(tr("Account list successfully read."), 2000);
  QApplication::restoreOverrideCursor();
  QMetaObject::invokeMethod(this, "aktivesKontoPruefen", Qt::QueuedConnection);
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
   setWindowTitle(tr("sctime - ")+ abt+"/"+ko+"/"+uko);
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
    connect(minutenTimer, SIGNAL(timeout()),unterKontoDialog->getZeitBox(),SLOT(incrMin()));

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

void TimeMainWindow::infoDialog(QDialog *&dialog, QTextBrowser *&browser, const QString& title, const QString& name, int x, int y) {
  dialog = new QDialog(this);
  dialog->setObjectName(name);
  dialog->setWindowTitle(title);
  QVBoxLayout *layout = new QVBoxLayout(dialog);
  browser = new QTextBrowser(this);
  browser->setOpenExternalLinks(true);
  layout->addWidget(browser);
  layout->addSpacing(7);
  QHBoxLayout* buttonlayout=new QHBoxLayout();
  buttonlayout->setContentsMargins(3,3,3,3);
  QPushButton * okbutton=new QPushButton( tr("OK"), dialog);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addStretch(1);
  layout->addLayout(buttonlayout);
  layout->addSpacing(4);
  connect (okbutton, SIGNAL(clicked()), dialog, SLOT(close()));
  dialog->resize(x, y);
  dialog->show();
}

void TimeMainWindow::callHelpDialog() {
  QDialog *dialog;
  QTextBrowser *browser;
  infoDialog(dialog, browser, tr("sctime: Help"), tr("sctime help"), 600, 450);
  browser->setSource(QUrl("qrc:/help"));
}


void TimeMainWindow::callAboutBox() {
  QDialog *dialog;
  QTextBrowser *browser;
  infoDialog(dialog, browser, tr("About sctime"), tr("sctime about"), 400, 300);
  browser->setHtml(tr(
        "<h1><img src=':/scLogo_15Farben' />sctime</h1>"
        "<table><tr><td>Version:</td><td>%1</td></tr>"
        "<tr><td>Qt-Version:</td><td>%2 (development)</td></tr>"
        "<tr><td></td><td>%3 (runtime)</td></tr>"
        "<tr><td>Developers:</td><td>Johannes Abt, Alexander Wütz, Florian Schmitt</td></tr>"
        "<tr><td>Patches:</td><td>Marcus Camen</td></tr>"
        "<tr><td>Mac:</td><td>Michael Weiser</td></tr>"
        "<tr><td>Project page:</td><td><a href='http://sourceforge.net/projects/sctime/'>http://sourceforge.net/projects/sctime/</a></td></tr>"
        "</table><p>This program is licensed under the GNU Public License v2.</p>")
                   .arg(qApp->applicationVersion(), QT_VERSION_STR, qVersion()));
}

void TimeMainWindow::logDialog() {
  QDialog *dialog;
  QTextBrowser *browser;
  infoDialog(dialog, browser, tr("sctime: Messages"), tr("sctime message log"), 700, 300);
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
    QMessageBox::critical(this, tr("sctime: On-call times"), tr("subaccount not found!"));
    return;
  }

  QDialog dialog(this);
  dialog.setObjectName(tr("sctime: On-call times"));
  dialog.setWindowTitle(tr("On-call times"));

  QVBoxLayout* layout=new QVBoxLayout(&dialog);
  layout->setMargin(15);

  QPushButton * okbutton=new QPushButton(tr("OK"), &dialog);
  okbutton->setDefault(true);
  QPushButton * cancelbutton=new QPushButton(tr("Cancel"), &dialog);

  layout->addStretch(1);


  QLabel* infolabel=new QLabel (tr("Please choose the rendered on-call times for this subaccount."), &dialog);
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
		kontoTree->refreshAllItemsInDepartment(abt);
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
   color = QColorDialog::getColor(initial, this); // nur Qt >= 4.5: , tr("Hintergrundfarbe"));

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
            tr("Warning"),
            tr("Warning: The entered comment contains a character that is not part of "
               "ISO-8859-1 and might not render correctly on some platforms. "
               "This may cause problems with custom reporting scripts."));
  }
}

void TimeMainWindow::moveEvent(QMoveEvent *event) {
  settings->setMainWindowGeometry(pos(),size());
}

void TimeMainWindow::commitBereit(DSResult data) {
  BereitschaftsListe *berListe = BereitschaftsListe::getInstance();
  QStringList ql;
  foreach (ql, data){
    if (ql.isEmpty()) continue;
    QString name = ql[0].simplified();
    QString beschreibung = ql[1].simplified();
    if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden. //FIXME: notwendig?
    berListe->insertEintrag(name, beschreibung);
  }
}
