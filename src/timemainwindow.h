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

#ifndef TIMEMAINWINDOW_H
#define TIMEMAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QDateTime>
class QAction;
class QTreeWidget;
class QTreeWidgetItem;

class BereitschaftsDatenInfo;
class UnterKontoDialog;
class KontoTreeView;
class ToolBar;
class SCTimeXMLSettings;
class KontoDatenInfo;
class EintragsListe;
class StatusBar;
class UnterKontoDialog;
class QTextBrowser;

#include "defaultcommentreader.h"
#include "datasource.h"


/** Diese Klasse implementiert das Hauptfenster des Programms,
    und sorgt zudem fuer das Fortschreiten der Zeit.
*/
class TimeMainWindow: public QMainWindow
{
  Q_OBJECT
public:
    TimeMainWindow();
    QTreeWidget* getKontoTree();
    virtual ~TimeMainWindow();
    SCTimeXMLSettings* settings;
    void infoDialog(QDialog *&dialog, QTextBrowser *&browser, const QString& title, const char* name, int x, int y);
    
  public slots:

    void callUnterKontoDialog(QTreeWidgetItem * item);
    void callDateDialog();
    void callAboutBox();
    void minutenUhr();
    void pause();
    void pauseAbzur(bool on);
    void zeitChanged();
    void updateCaption();
    void save();
    void resetDiff();
    void aktivesKontoPruefen();

    void inPersoenlicheKonten(bool hinzufuegen);
    void flagsChanged(const QString& abt, const QString& ko, const QString& uko, int idx);
    void changeShortCutSettings(QTreeWidgetItem * item);

    void editUnterKontoPressed();
    void editBereitschaftPressed();
    void changeDate(const QDate& datum);
    void setAktivesProjekt(QTreeWidgetItem * item);
    void showAdditionalButtons(bool show);
    void eintragAktivieren();
    void eintragHinzufuegen();
    void eintragEntfernen();
    void addDeltaToZeit(int delta, bool abzurOnly=false);
    void addTimeInc();
    void subTimeInc();
    void addFastTimeInc();
    void subFastTimeInc();
    void addAbzurTimeInc();
    void subAbzurTimeInc();
    void addFastAbzurTimeInc();
    void subFastAbzurTimeInc();

    void callFindKontoDialog();
    void callHelpDialog();
    void callPreferenceDialog();
    void callBereitschaftsDialog(QTreeWidgetItem * item);
    void callColorDialog();
    void removeBgColor();
    void jumpToAlleKonten();

    void refreshKontoListe();
    void configClickMode(bool singleClickActivation);
    void mouseButtonInKontoTreeClicked(QTreeWidgetItem * item, int column );
    void copyNameToClipboard();
    void showContextMenu(const QPoint& pos);
    void showArbeitszeitwarning();
    void checkComment(const QString& abt, const QString& ko , const QString& uko,int idx);
    void commitKontenliste(DSResult data);
    void displayLastLogEntry();

  signals:        
    /** Wird ausgeloest, falls sich die Gesamtzeit geaendert hat. Uebergeben wird die neue Gesamtzahl der Sekunden. */
    void gesamtZeitChanged(int) ;

    /** Wird ausgeloest, falls sich die abzurechnende Gesamtzeit
      * geaendert hat. Uebergeben wird die neue Gesamtzahl der Sekunden.
      */
    void gesamtZeitAbzurChanged(int) ;

    /**
      * Wird minuetlich ausgeloest, falls keine Pause aktiv ist.
      */
    void minuteTick();

    /**
      * Wird minuetlich ausgeloest, falls keine Pause aktiv ist.
      */
    void minuteAbzurTick();

    /**
     * Wird mit true ausgeloest, wenn ein Eintrag im Kontobaum
     * selektiert wurde, bei anderen Selektionen (Konto,Abteilung oder
     * Unterkonto mit mehreren Eintraegen) mit false
     */
    void eintragSelected(bool isEintrag);

    /**
      Wird mit true ausgeloest, wenn ein Unterkonto im Kontobaum
     * selektiert wurde, bei anderen Selektionen (Konto,Abteilung oder
     * blosser Eintrag) mit false
     */
    void unterkontoSelected(bool isUnterkonto);

    /**
     * Wird mit true ausgeloest, wenn ein Item im Kontobaum
     * selektiert wurde, zu dem weitere Eintraege hinzugefuegt werden koennen
     */
    void augmentableItemSelected(bool isAugmentable);

    /** Wird mit true ausgeloest, falls auf das aktuelle Datum gewechselt wird, bei allen anderen
        Datumswechseln mit false. */
    void currentDateSelected(bool);

    /**
     * Wird mit true ausgeloest, wenn ein Eintrag im Kontobaum
     * selektiert wurde, der aktivierbar ist, bei anderen Selektionen (Konto,Abteilung oder
     * Unterkonto mit mehreren Eintraegen) mit false
     */
    void aktivierbarerEintragSelected(bool isActivable);

  private slots:
    void quit();
    void logDialog();
    void commitBereit(DSResult data);

  protected:
    virtual void moveEvent( QMoveEvent *event);
  private:
    void checkLock();
    void updateTaskbarTitle(int zeit);
    void closeEvent(QCloseEvent * event);
    void refreshAfterColorChange(QString&, QString&, QString&);
    KontoTreeView* kontoTree;
    UnterKontoDialog* unterKontoDialog;
    QAction* editUnterKontoAction;
    QAction* inPersKontAction;
    QAction* abzurMin5PlusAction;
    QAction* abzurMin5MinusAction;
    QAction* fastAbzurPlusAction;
    QAction* fastAbzurMinusAction;
    QAction* eintragRemoveAction;
    QAction* checkInAction;
    QAction* bgColorChooseAction;
    QAction* bgColorRemoveAction;
    QAction* jumpAction;
    QDateTime lastMinuteTick;
    QFont qtDefaultFont;
    AbteilungsListe* abtList;
    AbteilungsListe* abtListToday;
    StatusBar* statusBar;
    //QMimeSourceFactory* mimeSourceFactory;
    DefaultCommentReader defaultCommentReader;
    QToolBar* powerToolBar;
    QToolBar* toolBar;
    QStringList defaultTags;
    KontoDatenInfo* zk;
    bool paused;
    bool pausedAbzur;
    void openItem( QTreeWidgetItem *item );
    // Workaround, um beim Setzen der Voreinstellung fuer den inPersoenlicheKonten-Button nicht das zugehoerige
    // Event auzuloesen. Wenn inPersoenlicheKontenAllowed=false, tut inPersoenlicheKonten(bool) gar nichts.
    bool inPersoenlicheKontenAllowed;
};
#endif
