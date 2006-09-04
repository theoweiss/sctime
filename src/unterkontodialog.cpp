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
#include "qstringlist.h"
#include "qcombobox.h"
#include <QGroupBox>
#include "qcheckbox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "abteilungsliste.h"
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
                                   AbteilungsListe* abtlist, QStringList* taglist,
                                   bool connectZeiten, QWidget * parent)
                                   :QDialog(parent,"Einstellungen für Unterkonto", true)
{
  abtList=abtlist;
  unterKontoName=uko;
  kontoName=ko;
  abteilungsName=abt;
  eintragIndex=idx;

  UnterKontoEintrag et;
  EintragsListe::iterator etiter;

  if (!abtList->findEintrag(etiter,m_unterkonto, abt, ko, uko,idx)) {
    std::cerr<<"Unterkonto nicht gefunden!"<<std::endl;
    return;
  }
  et = etiter->second;

  setCaption(uko);

  QVBoxLayout* layout=new QVBoxLayout(this,3);

  QPushButton * okbutton=new QPushButton( "OK", this );
  okbutton->setDefault(true);
  QPushButton * cancelbutton=new QPushButton( "Abbruch", this );

  QStringList* defaultcomments = m_unterkonto->getDefaultCommentList();

  // seufz, leider treffen sich QCombobox und QLineedit erst auf QWidget-Ebene
  if (defaultcomments->empty()) {
    commentedit = new QLineEdit(et.kommentar,this);
    commentcombo = NULL;
  } else {
    commentedit = NULL;
    commentcombo = new QComboBox(true,this);
    commentcombo->insertStringList(*defaultcomments);
    commentcombo->insertItem(0,et.kommentar);
    commentcombo->setCurrentIndex(0);
  }

  layout->addWidget(new QLabel("Kommentar",this));
  if (commentedit) {
    layout->addWidget(commentedit);
    commentedit->setFocus();
  } else {
    layout->addWidget(commentcombo);
    commentcombo->setFocus();
  }
  layout->addSpacing(5);
  layout->addStretch(2);
  if ((taglist) && (!taglist->isEmpty())) {
     QGroupBox* taggroup = new QGroupBox("Tags",this);
     QHBoxLayout *hboxLayout = new QHBoxLayout(taggroup);
     tagcombo = new QComboBox(taggroup);
     hboxLayout->addWidget(tagcombo);
     tagcombo->insertStringList(*taglist);
     QPushButton * addtagbutton = new QPushButton( "Hinzufügen", taggroup );;
     hboxLayout->addWidget(addtagbutton);
     layout->addWidget(taggroup);
     connect (addtagbutton, SIGNAL(clicked()), this, SLOT(addTag()));
     layout->addSpacing(5);
     layout->addStretch(2);
  } else tagcombo=NULL;

  zeitBox=new ZeitBox("Zeit", et.sekunden, this );
  layout->addWidget(zeitBox);
  layout->addSpacing(5);
  layout->addStretch(2);

  zeitAbzurBox=new ZeitBox("Abzurechnende Zeit", et.sekundenAbzur, this );
  layout->addWidget(zeitAbzurBox);
  layout->addSpacing(5);
  layout->addStretch(2);

  /*m_bereitschaften = m_unterkonto->getBereitschaft();

  bereitschaftsView=new BereitschaftsView(this);
  bereitschaftsView->setSelectionList(m_bereitschaften);
  layout->addWidget(bereitschaftsView);*/
  QHBoxLayout* buttonlayout=new QHBoxLayout(layout,3);

  projektAktivieren=new QPushButton("Eintrag aktivieren",this);
  projektAktivieren->setDisabled(abtList->isAktiv(abt,ko,uko,idx));  
  buttonlayout->addWidget(projektAktivieren);
  buttonlayout->addSpacing(10);
  buttonlayout->addStretch(1);
  persoenlichesKonto=new QCheckBox("In Persönliche Konten",this);
  persoenlichesKonto->setChecked(et.flags&UK_PERSOENLICH);
  buttonlayout->addWidget(persoenlichesKonto);
  buttonlayout->addStretch(1);
  layout->addSpacing(10);
  layout->addStretch(2);

  buttonlayout=new QHBoxLayout(layout,3);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);  
  buttonlayout->addWidget(cancelbutton);

  connect (okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
  connect (projektAktivieren, SIGNAL(clicked()), this, SLOT(projektAktivierenButtonClicked()));  

  if (connectZeiten) {
    connect (zeitBox, SIGNAL(minuteChangedBy(int)), zeitAbzurBox, SLOT(doStepMin(int)));
    connect (zeitBox, SIGNAL(hourChangedBy(int)), zeitAbzurBox, SLOT(doStepHour(int)));
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

  QString comment;

  if (commentedit)
    comment = commentedit->text();
  else
    comment = commentcombo->currentText();

  abtList->setEintrag(abteilungsName,kontoName,unterKontoName,eintragIndex,
    UnterKontoEintrag(comment,zeitBox->getSekunden(),
    zeitAbzurBox->getSekunden(), flags));
  abtList->moveEintragPersoenlich(abteilungsName,kontoName,unterKontoName,eintragIndex,
    (persoenlichesKonto->isChecked()));
  /*QStringList bereitschaften = bereitschaftsView->getSelectionList();
  m_unterkonto->setBereitschaft(bereitschaften);*/

  emit entryChanged(abteilungsName,kontoName,unterKontoName,eintragIndex);

  /*if (m_bereitschaften!=bereitschaften) {
     emit bereitschaftChanged(abteilungsName,kontoName,unterKontoName);
  }*/

  // Größe des Dialogs festhalten
  TimeMainWindow *timeMainWindow = static_cast<TimeMainWindow *>(qApp->mainWidget());
  timeMainWindow->settings->setUnterKontoWindowGeometry(pos(), size());

  QDialog::accept();
};

void UnterKontoDialog::projektAktivierenButtonClicked()
{
    projektAktivieren->setDisabled(true);
    emit entryActivated();
}

ZeitBox* UnterKontoDialog::getZeitAbzurBox()
{
  return zeitAbzurBox;
}

ZeitBox* UnterKontoDialog::getZeitBox()
{
  return zeitBox;
}

void UnterKontoDialog::addTag()
{
  if (tagcombo) {
      if (commentedit) {
           commentedit->setText(tagcombo->currentText()+", "+commentedit->text());
      } else {
           commentcombo->setCurrentText(tagcombo->currentText()+", "+commentcombo->currentText());
      }
  }
}
