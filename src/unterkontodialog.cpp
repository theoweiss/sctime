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

#include "qdialog.h"
#include "qpushbutton.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qgroupbox.h"
#include "qcheckbox.h"
#include "abteilungsliste.h"
#include "qlistview.h"
#include "timeedit.h"
#include "unterkontodialog.h"
#include <iostream>
#include "globals.h"
#include "sctimeapp.h"

/**
 * Baut den Dialog zum Aendern der Eigenschaften eines Unterkontos auf.
 * connectZeiten gibt an, ob die Schalter der abzurechnenden Zeit mit der Gesamtzeit verbunden werden soll.
 */
UnterKontoDialog::UnterKontoDialog(const QString& abt,const QString& ko, const  QString& uko, int idx,
                                   AbteilungsListe* abtlist, bool connectZeiten, QWidget * parent):QDialog(parent,"Einstellungen für Unterkonto", true)
{
  abtList=abtlist;
  unterKontoName=uko;
  kontoName=ko;
  abteilungsName=abt;
  eintragIndex=idx;

  UnterKontoEintrag et;

  if (!abtList->getEintrag(et,abt,ko,uko,idx)) std::cout<<"Warning: Unterkonto nicht gefunden!"<<std::endl;

  setCaption(uko);

  QVBoxLayout* layout=new QVBoxLayout(this,3);


  QPushButton * okbutton=new QPushButton( "OK", this );
  QPushButton * cancelbutton=new QPushButton( "Abbruch", this );
  commentedit = new QLineEdit(et.kommentar,this);

  layout->addWidget(new QLabel("Kommentar",this));
  layout->addWidget(commentedit);
  commentedit->setFocus();
  layout->addSpacing(5);
  layout->addStretch(2);

  zeitBox=new ZeitBox("Zeit", et.sekunden, this );
  layout->addWidget(zeitBox);
  layout->addSpacing(5);
  layout->addStretch(2);

  zeitAbzurBox=new ZeitBox("Abzurechnende Zeit", et.sekundenAbzur, this );
  layout->addWidget(zeitAbzurBox);
  layout->addSpacing(5);
  layout->addStretch(2);


  aktivesProjekt=new QCheckBox("Projekt aktivieren",this);
  aktivesProjekt->setChecked(abtList->isAktiv(abt,ko,uko,idx));
  layout->addWidget(aktivesProjekt);
  persoenlichesKonto=new QCheckBox("In Persönliche Konten",this);
  persoenlichesKonto->setChecked(et.flags&UK_PERSOENLICH);
  layout->addWidget(persoenlichesKonto);
  layout->addSpacing(10);
  layout->addStretch(2);

  QHBoxLayout* buttonlayout=new QHBoxLayout(layout,3);

  buttonlayout->addWidget(okbutton);
  buttonlayout->addWidget(cancelbutton);

  connect (okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
  connect (aktivesProjekt, SIGNAL(clicked()), this, SLOT(aktivesProjektButtonClicked()));

  if (connectZeiten) {
    connect (zeitBox, SIGNAL(minuteUp()), zeitAbzurBox, SLOT(incrMin()));
    connect (zeitBox, SIGNAL(minuteDown()), zeitAbzurBox, SLOT(decrMin()));
    connect (zeitBox, SIGNAL(hourUp()), zeitAbzurBox, SLOT(incrHour()));
    connect (zeitBox, SIGNAL(hourDown()), zeitAbzurBox, SLOT(decrHour()));
  }

  // Dialog auf die richtige Größe und Position bringen
  QPoint pos;
  QSize size;
  TimeMainWindow *timeMainWindow = static_cast<TimeMainWindow *>(qApp->mainWidget());
  timeMainWindow->settings->getUnterKontoWindowGeometry(pos, size);
  if (! size.isNull()) {
    resize(size);
    move(pos);
  }
};


/**
 * Wird aufgerufen, wenn der Benutzer auf OK clickt, speichert die Aenderungen in Abtlist
 * und loest die passenden entryChanged Signale aus.
 */
void UnterKontoDialog::accept()
{
  int flags=0;

  //if (persoenlichesKonto->isChecked()) flags|=UK_PERSOENLICH;

  abtList->setEintrag(abteilungsName,kontoName,unterKontoName,eintragIndex,
    UnterKontoEintrag(commentedit->text(),zeitBox->getSekunden(),
    zeitAbzurBox->getSekunden(), flags));
    abtList->moveEintragPersoenlich(abteilungsName,kontoName,unterKontoName,eintragIndex,(persoenlichesKonto->isChecked()));
  
  if (aktivesProjekt->isChecked()) {
    int oldidx;
    QString oldabt, oldko, olduk;
    abtList->getAktiv(oldabt, oldko, olduk,oldidx);
    abtList->setAsAktiv(abteilungsName,kontoName,unterKontoName,eintragIndex);
    emit entryChanged(oldabt,oldko,olduk,oldidx);
  }
  else {
    if (abtList->isAktiv(abteilungsName,kontoName,unterKontoName,eintragIndex))
      {
        abtList->setAsAktiv("","","",0);
      }
  }
  emit entryChanged(abteilungsName,kontoName,unterKontoName,eintragIndex);

  // Größe des Dialogs festhalten
  TimeMainWindow *timeMainWindow = static_cast<TimeMainWindow *>(qApp->mainWidget());
  timeMainWindow->settings->setUnterKontoWindowGeometry(pos(), size());

  QDialog::accept();
};


/**
 *  Behinhaltet die Logik, um beim Click auf Aktives Projekt zusaetzlich auch das persoenliche Projekt 
 *  zu aktivieren
 */
void UnterKontoDialog::aktivesProjektButtonClicked()
{
  if ((aktivesProjekt->isOn())&&(!persoenlichesKonto->isOn())) emit persoenlichesKonto->toggle();
}

ZeitBox* UnterKontoDialog::getZeitAbzurBox()
{
  return zeitAbzurBox;
}

ZeitBox* UnterKontoDialog::getZeitBox()
{
  return zeitBox;
}


