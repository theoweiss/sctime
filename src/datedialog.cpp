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

#include "datedialog.h"
#include "qlayout.h"

/*
 * Baut einen DateDialog auf.
*/
DateDialog::DateDialog(QDate datum, QWidget * parent):QDialog(parent)
{
  QVBoxLayout* layout=new QVBoxLayout(this,3);
  dateEdit=new QDateEdit(datum, this);
  layout->addWidget(new QLabel("Zu editierendes Datum:",this));
  layout->addWidget(dateEdit);

  QPushButton * okbutton=new QPushButton( "OK", this );
  QPushButton * cancelbutton=new QPushButton( "Cancel", this );

  layout->addWidget(okbutton);
  layout->addWidget(cancelbutton);

  connect (okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
}

/** Uebernimmt die eingegebenen einstellungen, und loest ein dateChanged Signal aus. */
void DateDialog::accept()
{
  emit dateChanged(dateEdit->date());
  QDialog::accept();
}



