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

#ifndef KONTOTREEVIEW_H
#define KONTOTREEVIEW_H

#include <QTreeWidget>
#include <QString>
#include "sctimexmlsettings.h"

class KontoTreeItem;
class QString;
class QTextStream;
class QToolTip;
class AbteilungsListe;
class TimeCounter;
class EintragsListe;

extern QString PERSOENLICHE_KONTEN_STRING;
extern QString ALLE_KONTEN_STRING;
#define MIMETYPE_ACCOUNT "application/sctime.account"
#define MIMETYPE_SECONDS "application/sctime.seconds"

/**
 * KontoTreeView ist ein von QTreeWidget abgeleitetes Widget, das sich um die Darstellung
 * des Kontobaumes kuemmert.
 */

class KontoTreeView: public QTreeWidget
{

  Q_OBJECT

  public:
    
    KontoTreeView(QWidget *parent, AbteilungsListe* abtlist, const std::vector<int>& columnwidth, SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode);

    bool sucheItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos, int idx,
         KontoTreeItem* &topi, KontoTreeItem* &abti, KontoTreeItem* &koi, KontoTreeItem* &ukoi, KontoTreeItem* &eti);

    KontoTreeItem* sucheKontoItem(const QString& tops, const QString& abts, const QString& kos);

    KontoTreeItem* sucheUnterKontoItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos);

    KontoTreeItem* sucheKommentarItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos, const QString& koms);

    void load(AbteilungsListe* abtlist);

    void itemInfo(QTreeWidgetItem* item,QString& tops, QString& abts, QString& kos, QString& ukos, int& idx);

    bool isEintragsItem(QTreeWidgetItem* item);

    bool isUnterkontoItem(QTreeWidgetItem* item);

    void flagClosedPersoenlicheItems();

    void closeFlaggedPersoenlicheItems();

    void showAktivesProjekt();

    void getColumnWidthList(std::vector<int>& columnwidth);

    void refreshParentSumTime(QTreeWidgetItem* item, QString prefix);

    void getSumTime(QTreeWidgetItem* item, TimeCounter& sum, TimeCounter& sumAbs);

    void showPersoenlicheKontenSummenzeit(bool show);

    int getItemDepth( QTreeWidgetItem* );

    void updateColumnWidth();

    void setDisplayMode(SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode);

  public slots:
    virtual void refreshItem(const QString& abt, const QString& ko,const QString& uko, int idx);
    void refreshAllItemsInUnterkonto(const QString& abt, const QString& ko,const QString& uko);
    void refreshAllItemsInKonto(const QString& abt, const QString& ko);
    void refreshAllItemsInDepartment(const QString&);

  signals:
    void itemRightClicked(QTreeWidgetItem *item);

  protected:
    virtual bool eventFilter ( QObject* obj, QEvent * e );
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);


  private:
    void refreshComment(const QString& comment, KontoTreeItem* item, EintragsListe* etl);
    bool m_showPersoenlicheKontenSummenzeit;
    AbteilungsListe* abtList;
    QPoint dragStartPosition;
    Qt::KeyboardModifiers keyboardModifier;
    QPersistentModelIndex rightPressedIndex;
    SCTimeXMLSettings::DefCommentDisplayModeEnum displaymode;
};

#endif
