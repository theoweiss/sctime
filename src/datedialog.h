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

#ifndef DATEDIALOG_H
#define DATEDIALOG_H

#include "qdialog.h"
#include "qpushbutton.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qdatetimeedit.h"


/**
 * Der Dialog zum eingeben eines Datums
 */
class DateDialog: public QDialog
{
  Q_OBJECT

  public:
    DateDialog(QDate datum, QWidget * parent = 0);
  public slots:
    virtual void accept();

  signals:
    void dateChanged(const QDate& datum);

  private:
    QDateEdit* dateEdit;

};

#endif

