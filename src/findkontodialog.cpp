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
#include "qdialog.h"
#include "qpushbutton.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qcombobox.h"
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "abteilungsliste.h"
#include "timeedit.h"
#include "qstringlist.h"

/**
 * Baut den Suchdialog auf. In abtlist wird die zu durchsuchende AbteilungsListe angegeben
 * das gesuchte Konto wird in den QString geschrieben, auf den _konto zeigt.
 * Warnung: natuerlich muss der _konto Zeiger waehrend der Lebensdauer des Dialogs
 * existent bleiben!
 */

FindKontoDialog::FindKontoDialog(AbteilungsListe* abtlist, QString* _konto, QWidget * parent):QDialog(parent)
{
  QVBoxLayout* layout=new QVBoxLayout(this,8);

  QStringList kontoStringList;

  kontoChoose=new QComboBox(this);
  kontoChoose->setEditable(true);
  kontoChoose->setAutoCompletion(true);
  kontoChoose->setCurrentText("");

  for (AbteilungsListe::iterator posAbt=abtlist->begin(); posAbt!=abtlist->end(); ++posAbt) {

    KontoListe* kontoliste=&(posAbt->second);

    for (KontoListe::iterator itKo=kontoliste->begin(); itKo!=kontoliste->end(); ++itKo)
    {
      kontoStringList.append (itKo->first);
    }

  }

  konto=_konto;

  kontoStringList.sort();

  kontoChoose->insertStringList(kontoStringList);

  layout->addWidget(new QLabel("Bitte wählen Sie den Namen des gesuchten Kontos:",this));
  layout->addStretch(1);
  QHBoxLayout* chooselayout=new QHBoxLayout(layout,5);
  chooselayout->addStretch(1);
  chooselayout->addWidget(kontoChoose);
  chooselayout->addStretch(1);
  
  layout->addSpacing(10);
  layout->addStretch(2);

  QPushButton * okbutton=new QPushButton( "OK", this );
  QPushButton * cancelbutton=new QPushButton( "Cancel", this );

  QHBoxLayout* buttonlayout=new QHBoxLayout(layout,5);
  buttonlayout->addStretch(1);
  buttonlayout->addWidget(okbutton);
  buttonlayout->addWidget(cancelbutton);
  buttonlayout->addStretch(1);

  connect (okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
}
    

void FindKontoDialog::accept ()
{
  *konto=kontoChoose->currentText();
  QDialog::accept();
}

