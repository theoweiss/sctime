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

#ifndef KONTOTREEVIEW_H
#define KONTOTREEVIEW_H

#include "qlistview.h"
#include "abteilungsliste.h"
#include "qstring.h"
#include "qpixmap.h"
#include "qtooltip.h"
#include "kontotreeitem.h"
#include <iostream>

#define PERSOENLICHE_KONTEN_STRING "Persönliche Konten"
#define ALLE_KONTEN_STRING "Alle Konten"

/**
 * KontoTreeView ist ein von QListView abgeleitetes Widget, das sich um die Darstellung
 * des Kontobaumes kuemmert.
 */

class KontoTreeView: public QListView
{

  Q_OBJECT

  public:
    KontoTreeView(QWidget *parent, AbteilungsListe* abtlist);

    bool sucheItem(const QString& tops, const QString& abts, const QString& kos, const QString& ukos, int idx,
         KontoTreeItem* &topi, KontoTreeItem* &abti, KontoTreeItem* &koi, KontoTreeItem* &ukoi, KontoTreeItem* &eti);

    KontoTreeItem* sucheKontoItem(const QString& tops, const QString& abts, const QString& kos);

    void load(AbteilungsListe* abtlist);
    
    void itemInfo(QListViewItem* item,QString& tops, QString& abts, QString& kos, QString& ukos, int& idx);
    
    bool isEintragsItem(QListViewItem* item);
    
    void flagClosedPersoenlicheItems();
    
    void closeFlaggedPersoenlicheItems();

    void showAktivesProjekt();

  public slots:

    virtual void refreshItem(const QString& abt, const QString& ko,const QString& uko, int idx);
    void refreshAllItemsInUnterkonto(const QString& abt, const QString& ko,const QString& uko);
    void refreshAllItemsInKonto(const QString& abt, const QString& ko);

  private:
    QPixmap* aktivPixmap;
    QPixmap emptyPixmap;
    AbteilungsListe* abtList;   

};

#endif
