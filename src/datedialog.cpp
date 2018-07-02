/*

    $Id$

    Copyright (C) 2003 Florian Schmitt, science + computing ag
                       f.schmitt@science-computing.de
    Copyright (C) 2003 Marcus Camen, science + computing ag
                       m.camen@science-computing.de

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
#include <QPushButton>
#include <QDir>
#include <QStringList>
#include <QTextCharFormat>
#include "globals.h"

DateDialog::DateDialog(const QDate& datum, QWidget *parent)
: QDialog(parent)
{
  setupUi(this);
  // to make sure month gets updated
  currentMonth=-1;
  currentYear=-1;
  connect(datePicker, SIGNAL(dateChanged(QDate)), this, SLOT(dateChangedSlot(QDate)));
  connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
  connect(datePicker, SIGNAL(tableClicked()), this, SLOT(apply()));
  connect(okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));

  datePicker->setSelectedDate(datum);

  // wird durch setDate nicht ausgeloest, muss aber zur initialisierung
  // aufgerufen werden.
  dateChangedSlot(datum);
}

DateDialog::~DateDialog()
{
}

void DateDialog::dateChangedSlot(QDate date)
{
  if ((date.month()!=currentMonth)||(date.year()!=currentYear)) {
    QDir qd(configDir.filePath("checkedin"));
    QStringList dateList;
    dateList << "zeit-" << date.toString("yyyy-MM") << "-*.xml"; //Since Qt4
    QStringList files=qd.entryList(dateList);
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
      bool ok;
      QDate filedate;
      int day=QString(*it).section('-',3,3).section('.',0,0).toInt(&ok);
      if ((ok)&&filedate.setDate(date.year(),date.month(),day)) {
        QTextCharFormat dtf=datePicker->dateTextFormat(filedate);
        dtf.setForeground(QBrush(Qt::red));
        dtf.setBackground(QBrush(Qt::red));
        datePicker->setDateTextFormat(filedate, dtf);
      }
    }
    currentMonth = date.month();
    currentYear  = date.year();
  }
}

void DateDialog::apply()
{
   emit dateChanged(datePicker->selectedDate());
}

/*$SPECIALIZATION$*/
void DateDialog::accept()
{
  emit dateChanged(datePicker->selectedDate());
  QDialog::accept();
}
