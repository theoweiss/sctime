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

#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStringList>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextCodec>
#include <QMessageBox>

#include "unterkontodialog.h"

/**
 * Baut den Dialog zum Aendern der Eigenschaften eines Unterkontos auf.
 * connectZeiten gibt an, ob die Schalter der abzurechnenden Zeit mit der Gesamtzeit verbunden werden soll.
 */
UnterKontoDialog::UnterKontoDialog(const QString& abt,const QString& ko, const  QString& uko, int idx,
                                   AbteilungsListe* abtlist, QStringList* taglist,
                                   bool connectZeiten, QMainWindow * parent, bool readOnly)
                                   :QDialog(parent, Qt::Dialog)
{
  setModal(true);
  setWindowTitle( "Einstellungen für Unterkonto" );
  abtList=abtlist;
  unterKontoName=uko;
  kontoName=ko;
  abteilungsName=abt;
  eintragIndex=idx;
  settings=NULL;

  parent = parent;
  UnterKontoEintrag et;
  EintragsListe::iterator etiter;

  if (!abtList->findEintrag(etiter,m_unterkonto, abt, ko, uko,idx)) {
    QMessageBox::information(parent,tr("sctime: Einstellungen des Unterkontos"), tr("Unterkonto nicht gefunden!"));
    return;
  }
  et = etiter->second;

  //setCaption(uko);
  setWindowTitle(uko);

  QVBoxLayout* layout=new QVBoxLayout(this);
  layout->setContentsMargins(3,3,3,3);

  QPushButton * cancelbutton=new QPushButton(tr("Abbruch"), this );
  QPushButton * okbutton=NULL;
  if (!readOnly) {
    okbutton=new QPushButton( "OK", this );
    okbutton->setDefault(true);
  }
  else {
    cancelbutton->setDefault(true);
  }

  QStringList* defaultcomments = m_unterkonto->getDefaultCommentList();

  // seufz, leider treffen sich QCombobox und QLineedit erst auf QWidget-Ebene
  if (defaultcomments->empty()) {
    commentedit = new QLineEdit(et.kommentar,this);
    commentcombo = NULL;
    commentedit->setReadOnly(readOnly);
  } else {
    commentedit = NULL;
    commentcombo = new QComboBox(this);
    commentcombo->setEditable(true);
    //commentcombo->insertStringList(*defaultcomments); //Qt3
    commentcombo->insertItems(0, *defaultcomments);
    commentcombo->insertItem(0,et.kommentar);
    commentcombo->setCurrentIndex(0);
    commentcombo->setEditable(!readOnly);
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
     //tagcombo->insertStringList(*taglist); //Qt3
     tagcombo->insertItems(0, *taglist);
     QPushButton * addtagbutton = new QPushButton( "Hinzufügen", taggroup );;
     hboxLayout->addWidget(addtagbutton);
     layout->addWidget(taggroup);
     connect (addtagbutton, SIGNAL(clicked()), this, SLOT(addTag()));
     layout->addSpacing(5);
     layout->addStretch(2);
  } else tagcombo=NULL;

  zeitBox=new ZeitBox("Zeit", et.sekunden, this );
  zeitBox->setReadOnly(readOnly);
  layout->addWidget(zeitBox);
  layout->addSpacing(5);
  layout->addStretch(2);

  zeitAbzurBox=new ZeitBox("Abzurechnende Zeit", et.sekundenAbzur, this );
  zeitAbzurBox->setReadOnly(readOnly);
  layout->addWidget(zeitAbzurBox);
  layout->addSpacing(5);
  layout->addStretch(2);

  QString beschreibung = abtList->getDescription(abt,ko,uko).description().simplified();
  if (!beschreibung.isEmpty()) {
    QLabel *l = new QLabel(tr("Beschreibung: ") + beschreibung, this);
    layout->addWidget(l);
    layout->addSpacing(5);
    layout->addStretch(2);
    l->setOpenExternalLinks(true);
  }
  QLabel *l2 = new QLabel(tr("Verantwortlich: ") + abtList->getDescription(abt,ko,uko).responsible(), this);
  layout->addWidget(l2);
  layout->addSpacing(5);
  layout->addStretch(2);

  /*m_bereitschaften = m_unterkonto->getBereitschaft();

  bereitschaftsView=new BereitschaftsView(this);
  bereitschaftsView->setSelectionList(m_bereitschaften);
  layout->addWidget(bereitschaftsView);*/
  QHBoxLayout* buttonlayout=new QHBoxLayout();
  //buttonlayout->setParent(layout);
  buttonlayout->setContentsMargins(3,3,3,3);

  projektAktivieren=new QPushButton("Eintrag aktivieren",this);
  projektAktivieren->setDisabled(abtList->isAktiv(abt,ko,uko,idx));
  buttonlayout->addWidget(projektAktivieren);
  buttonlayout->addSpacing(10);
  buttonlayout->addStretch(1);
  persoenlichesKonto=new QCheckBox(tr("In die persönlichen Konten übernehmen"),this);
  persoenlichesKonto->setChecked(et.flags&UK_PERSOENLICH);
  buttonlayout->addWidget(persoenlichesKonto);
  buttonlayout->addStretch(1);
  layout->addSpacing(10);
  layout->addStretch(2);
  layout->addLayout(buttonlayout);

  buttonlayout=new QHBoxLayout();
  //buttonlayout->setParent(layout);
  buttonlayout->setContentsMargins(3,3,3,3);
  buttonlayout->addStretch(1);

  if (!readOnly)
    buttonlayout->addWidget(okbutton);
  buttonlayout->addWidget(cancelbutton);
  layout->addLayout(buttonlayout);
  if (!readOnly)
    connect (okbutton, SIGNAL(clicked()), this, SLOT(checkInput()));
  connect (cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
  connect (projektAktivieren, SIGNAL(clicked()), this, SLOT(projektAktivierenButtonClicked()));

  if (connectZeiten) {
    connect (zeitBox, SIGNAL(minuteChangedBy(int)), zeitAbzurBox, SLOT(doStepMin(int)));
    connect (zeitBox, SIGNAL(hourChangedBy(int)), zeitAbzurBox, SLOT(doStepHour(int)));
  }
};

QString UnterKontoDialog::getComment()
{
  QString comment;

  if (commentedit)
    comment = commentedit->text();
  else
    comment = commentcombo->currentText();

  comment=comment.remove("\n");
  comment=comment.remove("\r");
  return comment;
}

/**
 * Wird aufgerufen, wenn der Benutzer auf OK clickt, speichert die Aenderungen in Abtlist
 * und loest die passenden entryChanged Signale aus.
 */
void UnterKontoDialog::accept()
{
  int flags=0;

  QString comment=getComment();


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

  // Größe/Position des Dialogs festhalten

  if( settings != NULL )
  {
    settings->setUnterKontoWindowGeometry(pos(), size());
  }

  QDialog::accept();
};

void UnterKontoDialog::setSettings(SCTimeXMLSettings* s)
{
  settings = s;
}

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
           commentcombo->setEditText(tagcombo->currentText()+", "+commentcombo->currentText());
           //commentcombo->insertItem(commentcombo->count()+1, tagcombo->currentText()+", "+commentcombo->currentText());
      }
  }
}

void UnterKontoDialog::checkInput()
{
  QTextCodec* codec=QTextCodec::codecForLocale();
  if (!codec->canEncode(getComment())) {
    QMessageBox::critical(0,"Fehler","Fehler: In dem von "
        "Ihnen eingegebenen Kommentar kommt ein Zeichen vor, das in Ihrem Locale "
            "nicht darstellbar ist. ",
            QMessageBox::Ok | QMessageBox::Default,0);
    reject();
  } else
    accept();
}

