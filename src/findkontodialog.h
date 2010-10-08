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

#ifndef FINDKONTODIALOG_H
#define FINDKONTODIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QComboBox>

#include "abteilungsliste.h"
#include "timeedit.h"
//#include "kontotreeitem.h"
#include "kontotreeview.h"
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMouseEvent>

/**
 * Der Dialog Aendern der Einstellungen
 */
class FindKontoDialog: public QDialog
{
  Q_OBJECT

  public:
    FindKontoDialog(AbteilungsListe* abtlist, QWidget * parent = 0);
    QStringList getSelectedItems();

  protected slots:
    void reloadValueChoose();
    void doSearch();
    void toggleButton(QTreeWidgetItem*, QTreeWidgetItem*);
    void setSearchFocus();

  private:
    void createLayout();
    void createConnects();
    void createWidgets();
    void getKontoListe();
    void getUnterKontoListe();
    void getKommentarListe();
    void searchKonto();
    void searchUnterKonto();
    void searchKommentar();
    void setFoundItem(QTreeWidgetItem* item);
    QStringList getNamesFromTreeItems();

    QGridLayout *mainLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    QHBoxLayout *buttonLayout;

    QComboBox *kontoChoose;
    QComboBox *valueChoose;
    QComboBox *typeChoose;

    QStringList valueStringList, typeStringList;
    QStringList resultList;


    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *searchButton;

    QTreeWidget *resultTree;

    QTreeWidgetItem* allekonten;
    QTreeWidgetItem* abteilungsitem;
    QTreeWidgetItem* kontoitem;
    QTreeWidgetItem* unterkontoitem;
    QTreeWidgetItem* kommentaritem;

    AbteilungsListe* abtlist;

    QString currentAbteilung;
    QString currentKonto;
    QString currentUnterKonto;
    QString chosenValueString;

    QColor foundItemColor;
};

#endif
