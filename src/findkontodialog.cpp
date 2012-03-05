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

#include "findkontodialog.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "abteilungsliste.h"
#include "kontotreeview.h"


#define ALLE QObject::tr("All")
#define KONTO QObject::tr("Account")
#define UNTERKONTO QObject::tr("Subaccount")
#define KOMMENTAR QObject::tr("Comment")

/**
 * Baut den Suchdialog auf. In abtlist wird die zu durchsuchende AbteilungsListe angegeben
 * das gesuchte Konto wird in den QString geschrieben, auf den _konto zeigt.
 * Warnung: natuerlich muss der _konto Zeiger waehrend der Lebensdauer des Dialogs
 * existent bleiben!
 */
FindKontoDialog::FindKontoDialog(AbteilungsListe* abtlist, QWidget * parent):QDialog(parent)
{
  this->abtlist = abtlist;

  createLayout();
  createWidgets();
  createConnects();

  typeStringList.append(ALLE);
  typeStringList.append(KONTO);
  typeStringList.append(UNTERKONTO);
  typeStringList.append(KOMMENTAR);

  typeChoose->insertItems(0,typeStringList);

  foundItemColor = QColor(0,191,255);
}

void FindKontoDialog::createLayout()
{
  mainLayout = new QGridLayout(this);

  leftLayout=new QVBoxLayout();
  rightLayout=new QVBoxLayout();
  buttonLayout=new QHBoxLayout();

  mainLayout->addLayout(leftLayout, 0, 0, Qt::AlignLeft | Qt::AlignTop);
  mainLayout->addLayout(rightLayout, 0, 1, Qt::AlignLeft | Qt::AlignTop);
  mainLayout->addLayout(buttonLayout, 1, 1, Qt::AlignRight | Qt::AlignBottom);
}

void FindKontoDialog::createWidgets()
{
  this->setMinimumHeight( 300 );
  this->setWindowTitle(tr("sctime - Search"));

  typeChoose = new QComboBox(this);
  typeChoose->setEditable(false);

  valueChoose = new QComboBox(this);
  valueChoose->setEditable(true);
  valueChoose->setAutoCompletion(false);
  valueChoose->setFocus();

  leftLayout->addWidget(new QLabel(tr("Please select type of item to search for:"),this));
  leftLayout->addWidget(typeChoose);
  leftLayout->addWidget(new QLabel(tr("Please select name of item to search for:"), this));
  leftLayout->addWidget(valueChoose);

  resultTree = new QTreeWidget(this);
  resultTree->setHeaderLabel(tr("Search result"));
  resultTree->setColumnCount(1) ;
  resultTree->setSelectionMode(QAbstractItemView::SingleSelection);
  rightLayout->addWidget(resultTree);

  okButton=new QPushButton( tr("&OK"), this );
  okButton->setEnabled(false);
  cancelButton=new QPushButton(tr("&Cancel"), this );
  searchButton=new QPushButton(tr("&Search"), this );
  searchButton->setDefault( true );

  buttonLayout->setAlignment(Qt::AlignRight);
  buttonLayout->addWidget(searchButton);
  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(cancelButton);
}

void FindKontoDialog::createConnects()
{
  connect (okButton,      SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect (searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));
  connect (typeChoose,   SIGNAL(currentIndexChanged(QString)), this, SLOT(reloadValueChoose()));
  connect (resultTree,   SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(accept()));
  connect (resultTree,   SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
    this, SLOT(toggleButton(QTreeWidgetItem*, QTreeWidgetItem*)));
  connect (valueChoose,  SIGNAL(editTextChanged ( const QString & )), this, SLOT(setSearchFocus()));
}

void FindKontoDialog::toggleButton(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  if( !current ) return;
  if( current->childCount() == 0 )
  {
    okButton->setEnabled(true);
  }
  else
  {
    okButton->setEnabled(false);
  }
}

//Update the QComboBox with values for the current selected type
void FindKontoDialog::reloadValueChoose()
{
  QString chosenString = typeChoose->currentText();
  valueStringList.clear();
  valueChoose->clear();

  if( chosenString == ALLE )
  {
    getKontoListe();
    getUnterKontoListe();
    getKommentarListe();
  } else if( chosenString == KONTO )
  {
    getKontoListe();
  } else if( chosenString == UNTERKONTO )
  {
    getUnterKontoListe();
  } else if( chosenString == KOMMENTAR )
  {
    getKommentarListe();
  }

  #if QT_VERSION >= 0x040500
    //Method only available at Qt4.5 or greater
    valueStringList.removeDuplicates();
  #endif
  valueStringList.sort();
  valueChoose->insertItem(0, "");
  valueChoose->insertItems(1, valueStringList);
}

//Get Value list for konto
void FindKontoDialog::getKontoListe()
{
  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt)
  {
    KontoListe* kontoliste=&(posAbt->second);
    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      if( !valueStringList.contains(itKo->first) )
      {
        valueStringList.append (itKo->first);
      }
    }
  }
}

//Get Value list for Unterkonto
void FindKontoDialog::getUnterKontoListe()
{
  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt)
  {
    KontoListe* kontoliste=&(posAbt->second);
    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      UnterKontoListe* unterkontoliste =&(itKo->second);
      for(UnterKontoListe::iterator itUko=unterkontoliste->begin(); itUko!=unterkontoliste->end(); ++itUko)
      {
        if( !valueStringList.contains(itUko->first) )
        {
          valueStringList.append (itUko->first);
        }
      }
    }
  }
}

//Get Value list for Kommentar
void FindKontoDialog::getKommentarListe()
{
  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt)
  {
    KontoListe* kontoliste=&(posAbt->second);
    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      UnterKontoListe* unterkontoliste =&(itKo->second);
      for(UnterKontoListe::iterator itUko=unterkontoliste->begin(); itUko!=unterkontoliste->end(); ++itUko)
      {
        EintragsListe* eintragliste =&(itUko->second);
        for(EintragsListe::iterator itEt=eintragliste->begin(); itEt!=eintragliste->end(); ++itEt)
        {
          QString kommentar = itEt->second.kommentar;
          if( kommentar != "" && (!valueStringList.contains(kommentar)))
          {
            valueStringList.append (kommentar);
          }
        }
      }
    }
  }
}

//Get the selected item path as qstringlist
QStringList FindKontoDialog::getSelectedItems()
{
  return getNamesFromTreeItems();
}

//Get the TreeItem Names from the result tree
QStringList FindKontoDialog::getNamesFromTreeItems()
{
  QTreeWidgetItem *currentItem = resultTree->currentItem();
  QStringList itemNames;
  if(currentItem)
  {
    itemNames.push_front(currentItem->text(0));
    //Find TopItem
    while( currentItem->parent() != 0 )
    {
      QTreeWidgetItem *parentItem = currentItem->parent();
      //Add text to return list
      itemNames.push_front(parentItem->text(0));
      currentItem = parentItem;
    }
  }
  return itemNames;
}

//Run the search
void FindKontoDialog::doSearch()
{
  currentAbteilung = "";
  currentKonto = "";
  currentUnterKonto = "";

  QString chosenTypeString = typeChoose->currentText();
  chosenValueString = valueChoose->currentText();
  resultTree->clear();
  resultList.clear();

  allekonten=new QTreeWidgetItem(resultTree, 0);
  allekonten->setText(0, ALLE_KONTEN_STRING);

  if( chosenValueString == "" ) return;
  if( chosenTypeString == ALLE )
  {
    searchKonto();
    searchUnterKonto();
    searchKommentar();
  } else if( chosenTypeString == KONTO || chosenTypeString == ALLE )
  {
    searchKonto();
  } else if( chosenTypeString == UNTERKONTO )
  {
    searchUnterKonto();
  } else if( chosenTypeString == KOMMENTAR )
  {
    searchKommentar();
  }

  resultTree->expandAll();
  resultTree->resizeColumnToContents( 0 );
  resultTree->setFocus();
  okButton->setDefault(true);
}

void FindKontoDialog::searchKonto()
{
  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt)
  {
    KontoListe* kontoliste=&(posAbt->second);
    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      if( itKo->first.contains(chosenValueString, Qt::CaseInsensitive) )
      {
        if( posAbt->first != currentAbteilung )
        {
          currentAbteilung = posAbt->first;
          abteilungsitem = new QTreeWidgetItem( allekonten, 0);
          abteilungsitem->setText(0, posAbt->first );
        }
        kontoitem= new QTreeWidgetItem( abteilungsitem, 0);
        kontoitem->setText(0, itKo->first );
        setFoundItem(kontoitem);
      }
    }
  }
}

void FindKontoDialog::searchUnterKonto()
{
  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt)
  {
    KontoListe* kontoliste=&(posAbt->second);
    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      UnterKontoListe* unterkontoliste =&(itKo->second);
      for(UnterKontoListe::iterator itUko=unterkontoliste->begin(); itUko!=unterkontoliste->end(); ++itUko)
      {
        if(itUko->first.contains(chosenValueString, Qt::CaseInsensitive))
        {
          if( posAbt->first != currentAbteilung )
          {
            currentAbteilung = posAbt->first;
            abteilungsitem = new QTreeWidgetItem( allekonten, 0);
            abteilungsitem->setText(0, posAbt->first );
          }
          if( itKo->first != currentKonto  )
          {
            currentKonto = itKo->first;
            kontoitem= new QTreeWidgetItem( abteilungsitem, 0);
            kontoitem->setText(0, itKo->first);
          }
          unterkontoitem= new QTreeWidgetItem( kontoitem, 0);
          unterkontoitem->setText(0, itUko->first);
          setFoundItem(unterkontoitem);
        }
      }
    }
  }
}

void FindKontoDialog::searchKommentar()
{
  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt)
  {
    KontoListe* kontoliste=&(posAbt->second);
    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      UnterKontoListe* unterkontoliste =&(itKo->second);
      for(UnterKontoListe::iterator itUko=unterkontoliste->begin(); itUko!=unterkontoliste->end(); ++itUko)
      {
        EintragsListe* eintragliste =&(itUko->second);
        for(EintragsListe::iterator itEt=eintragliste->begin(); itEt!=eintragliste->end(); ++itEt)
        {
          QString kommentar = itEt->second.kommentar;
          if( kommentar != "" )
          {
            if( kommentar.contains(chosenValueString, Qt::CaseInsensitive) )
            {
              if( posAbt->first != currentAbteilung )
              {
                currentAbteilung = posAbt->first;
                abteilungsitem = new QTreeWidgetItem( allekonten, 0);
                abteilungsitem->setText(0, posAbt->first );
              }
              if( itKo->first != currentKonto  )
              {
                currentKonto = itKo->first;
                kontoitem= new QTreeWidgetItem( abteilungsitem, 0);
                kontoitem->setText(0, itKo->first);
              }
              if( itUko->first != currentUnterKonto )
              {
                currentUnterKonto = itUko->first;
                unterkontoitem = new QTreeWidgetItem( kontoitem, 0);
                unterkontoitem->setText(0, itUko->first);
              }
              kommentaritem= new QTreeWidgetItem( unterkontoitem, 0);
              kommentaritem->setText(0,itEt->second.kommentar);
              setFoundItem( kommentaritem );
            }
          }
        }
      }
    }
  }
}

void FindKontoDialog::setFoundItem(QTreeWidgetItem* item)
{
  resultTree->setCurrentItem( item );
  item->setExpanded(true);
  item->setSelected(true);
  //item->setBackgroundColor(0, foundItemColor);
  toggleButton( item, 0);
}

void FindKontoDialog::setSearchFocus()
{
  searchButton->setDefault( true );
}
