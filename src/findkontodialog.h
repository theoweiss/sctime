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

#ifndef FINDKONTODIALOG_H
#define FINDKONTODIALOG_H

#include <QDialog>
class QPushButton;
class QLabel;
class QComboBox;
class QTreeWidget;
class QVBoxLayout;
class QTreeWidgetItem;
class QHBoxLayout;

class AbteilungsListe;

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
    void searchMicroAccount();
    void setFoundItem(QTreeWidgetItem* item);
    QStringList getNamesFromTreeItems();

    QVBoxLayout *mainLayout;
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
