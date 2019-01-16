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

class TextViewerDialog;

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
    void infoDialog(TextViewerDialog *&dialog, const QString& title, const QString& name, int x, int y, bool plaintext_links=false);
    
  public slots:

    void callUnterKontoDialog(QTreeWidgetItem * item);
    void callDateDialog();
    void callAboutBox();
    void callNightTimeDialog(bool isnight);
    void minuteHochzaehlen();
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
    void specialRemunPressed();
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
    void callSpecialRemunerationsDialog(QTreeWidgetItem * item);
    void callColorDialog();
    void callAdditionalLicenseDialog();
    void removeBgColor();
    void jumpToAlleKonten();

    void refreshKontoListe();
    void configClickMode(bool singleClickActivation);
    void copyNameToClipboard();
    void copyEntryAsText();
    void showArbeitszeitwarning();
    void checkComment(const QString& abt, const QString& ko , const QString& uko,int idx);
    void commitKontenliste(DSResult data);
    void displayLastLogEntry();
    void resume(); // APM event
    void suspend(); // APM event
    void switchOvertimeRegulatedMode(bool enabled);
    void switchOvertimeOtherMode(bool enabled);
    void switchPublicHolidayMode(bool enabled);
    void switchNightMode(bool enabled);

    void updateSpecialModesAfterPause();


  signals:        
    /** Wird ausgeloest, falls sich die Gesamtzeit geaendert hat. Uebergeben wird die neue Gesamtzahl der Sekunden. */
    void gesamtZeitChanged(int) ;

    /** Wird ausgeloest, falls sich die abzurechnende Gesamtzeit
      * geaendert hat. Uebergeben wird die neue Gesamtzahl der Sekunden.
      */
    void gesamtZeitAbzurChanged(int) ;

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
    void commitSpecialRemun(DSResult);
    void driftKorrektur();
    void callNightTimeBeginDialog();
    void callNightTimeEndDialog();
  protected:
    virtual void moveEvent( QMoveEvent *event);
  private:
    void checkLock();
    void updateTaskbarTitle(int zeit);
    void closeEvent(QCloseEvent * event);
    void refreshAfterColorChange(QString&, QString&, QString&);
    void resizeToIfSensible(QDialog* dialog, const QPoint& pos, const QSize& size);
    bool checkAndChangeSREntry(int& idx, const QString& abt, const QString& ko , const QString& uko, const QSet<QString>& specialRemuns);
    void switchOvertimeMode(bool enabled, QString otmSR);
    void cantMoveTimeDialog(int delta);
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
    QAction* specialRemunAction;
    QAction* overtimeRegulatedModeAction;
    QAction* overtimeOtherModeAction;
    QAction* nightModeAction;
    QAction* publicHolidayModeAction;
    QDateTime startTime;
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
    QString additionalLicensesFile;
    bool pausedAbzur;
    void openItem( QTreeWidgetItem *item );
    // Workaround, um beim Setzen der Voreinstellung fuer den inPersoenlicheKonten-Button nicht das zugehoerige
    // Event auzuloesen. Wenn inPersoenlicheKontenAllowed=false, tut inPersoenlicheKonten(bool) gar nichts.
    bool inPersoenlicheKontenAllowed;
    int sekunden; // Minuten * 60 seit Beginn minus Pausen; zur Drift-Berechnung
    QTimer *minutenTimer;
    QTimer *restTimer;
    QTimer *autosavetimer;
    void tageswechsel();
    void zeitKorrektur(int delta);
    bool paused;
    int stopTimers(const QString& grund); // rv: Sekunden seit letztem Tick
    void resumeTimers(int secSinceTick, const QString& reason);
};
#endif
