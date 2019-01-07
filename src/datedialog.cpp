/*
    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt, Marcus Camen et al.

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

#include "datedialog.h"
#include <QPushButton>
#include <QDir>
#include <QStringList>
#include <QTextCharFormat>
#include "globals.h"

DateDialog::DateDialog(const QDate& date, QWidget *parent)
: QDialog(parent)
{
  setupUi(this);
  selectedDate=QDate();
  connect(datePicker, SIGNAL(clicked(const QDate&)), this, SLOT(setSelectedDate(const QDate&)));
  connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
  connect(this, SIGNAL(dateChanged(const QDate&)), this, SLOT(setSelectedDate(const QDate&)));
  connect(okbutton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(todayButton, SIGNAL(clicked()), this, SLOT(todaySelected()));
  connect(dateEdit,SIGNAL(dateChanged(const QDate&)), this, SLOT(setSelectedDate(const QDate&)));
  weekSelector->setEditable(false);
  setSelectedDate(date);
  connect(weekSelector,SIGNAL(currentIndexChanged(int)), this, SLOT(weekSelected(int)));
}

DateDialog::~DateDialog()
{
}

void DateDialog::setSelectedDate(const QDate& date)
{
  //int prevweek=datePicker->selectedDate().weekNumber();
  if ((date.month()!=selectedDate.month())||(date.year()!=selectedDate.year())) {
    QDir qd(configDir.filePath("checkedin"));
    QStringList dateList;
    dateList << "zeit-" << date.toString("yyyy-MM") << "-*.xml";
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
  }
  /* avoid triggering the weekSelected slot, as this might cause endless loops. There might be a
     more elegant solution for this, but for now it works fine this way */
  disconnect(weekSelector,SIGNAL(currentIndexChanged(int)), this, SLOT(weekSelected(int)));
  if (selectedDate.year()!=date.year()) {
    QDate yearend=QDate(date.year(),12,31);
    // determine if we have 52 or 53 weeks
    int lastweek=yearend.weekNumber();
    if (lastweek==1) {
      lastweek=52;
    }
    weekSelector->clear();
    for (int i=0; i<lastweek; i++) {
      weekSelector->insertItem(i,tr("Week %1").arg(i+1));
    }
  }
  datePicker->setSelectedDate(date);
  dateEdit->setDate(date);
  selectedDate=date;
  weekSelector->setCurrentIndex(date.weekNumber()-1);
  connect(weekSelector,SIGNAL(currentIndexChanged(int)), this, SLOT(weekSelected(int)));
}

void DateDialog::apply()
{
   emit dateChanged(selectedDate);
}

/*$SPECIALIZATION$*/
void DateDialog::accept()
{
  emit dateChanged(selectedDate);
  QDialog::accept();
}

void DateDialog::todaySelected()
{
  QDate today=QDate::currentDate();
  if (selectedDate!=today) {
    setSelectedDate(today);
  }
}

void DateDialog::weekSelected(int week)
{
  // week starts with 0 we want to have it started with 1
  week=week+1;
  QDate prevdate=selectedDate;
  int currentweek=prevdate.weekNumber();
  QDate date = prevdate.addDays((week-currentweek)*7);

  if ((week>0) && (prevdate!=date)) {
    setSelectedDate(date);
  }
}
