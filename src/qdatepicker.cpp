/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
              (C) 2003 Florian Schmitt <f.schmitt@science-computing.de>
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// This file has been ported from KDE to plain QT

#include <qlayout.h>
#include <q3frame.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qstyle.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qfont.h>
#include <qvalidator.h>
#include <q3popupmenu.h>
#include <QMenuItem>
#include <QDesktopWidget>
#include <iostream>

#include "qdatepicker.h"

#include <qapplication.h>

//Added by qt3to4:
#include <QBoxLayout>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>

#include "../pics/cr16-action-bookmark.xpm"

QDateValidator::QDateValidator(QWidget* parent, const char* name)
    : QValidator(parent, name)
{
}

QValidator::State
QDateValidator::validate(QString& text, int&) const
{
  QDate temp;
  // ----- everything is tested in date():
  return date(text, temp);
}

QValidator::State
QDateValidator::date(const QString& text, QDate& d) const
{
  QDate tmp = QDate::fromString(text,Qt::ISODate);
  if (!tmp.isNull())
    {
      d = tmp;
      return Acceptable;
    } else
      return QValidator::Intermediate;
}

void
QDateValidator::fixup( QString& ) const
{

}

QDateInternalWeekSelector::QDateInternalWeekSelector
(QWidget* parent, const char* name)
  : QLineEdit(parent, name),
    val(new QIntValidator(this)),
    result(0)
{
  QFont font;
  // -----
  setFont(font);
  setFrame(false);
  setValidator(val);
  connect(this, SIGNAL(returnPressed()), SLOT(weekEnteredSlot()));
}

void
QDateInternalWeekSelector::weekEnteredSlot()
{
  bool ok;
  int week;
  // ----- check if this is a valid week:
  week=text().toInt(&ok);
  if(!ok)
    {
      return;
    }
  result=week;
  emit(closeMe(1));
}

int
QDateInternalWeekSelector::getWeek()
{
  return result;
}

void
QDateInternalWeekSelector::setWeek(int week)
{
  QString temp;
  // -----
  temp.setNum(week);
  setText(temp);
}

void
QDateInternalWeekSelector::setMaxWeek(int max)
{
  val->setRange(1, max);
}

class QDatePicker::QDatePickerPrivate
{
public:
    QDatePickerPrivate() : selectWeek(0L), todayButton(0) {}

    void fillWeeksCombo(const QDate &date);

    QComboBox *selectWeek;
    QToolButton *todayButton;
};

void QDatePicker::fillWeeksCombo(const QDate &date)
{
  // every year can have a different number of weeks
  // the 28th of december is always in the last week...
  int i, weeks = (QDate(date.year(),12,28)).weekNumber();

  if ( d->selectWeek->count() == weeks ) return;  // we already have the correct number

  d->selectWeek->clear();

  for (i = 1; i <= weeks; i++)
    d->selectWeek->insertItem(QString("Woche %1").arg(i));
}

QDatePicker::QDatePicker(QWidget *parent, QDate dt, const char *name)
  : Q3Frame(parent,name)
{
  init( dt );
}

QDatePicker::QDatePicker(QWidget *parent, QDate dt, const char *name, Qt::WFlags f)
  : Q3Frame(parent,name, f)
{
  init( dt );
}

QDatePicker::QDatePicker( QWidget *parent, const char *name )
  : Q3Frame(parent,name)
{
  init( QDate::currentDate() );
}

void QDatePicker::init( const QDate &dt )
{
  d = new QDatePickerPrivate();

  QBoxLayout * topLayout = new QVBoxLayout(this);

  line = new QLineEdit(this);
  val = new QDateValidator(this);
  table = new QCalendarWidget(this);
  QFont font;
  fontsize = font.pointSize();
  if (fontsize == -1)
     fontsize = QFontInfo(font).pointSize();

  fontsize++; // Make a little bigger

  d->selectWeek = new QComboBox(false, this);  // read only week selection
  d->todayButton = new QToolButton(this);
  d->todayButton->setIconSet(QIcon(QPixmap((const char **)cr16_action_bookmark)));
  table->setFirstDayOfWeek(Qt::Monday);

  QToolTip::add(d->selectWeek, "Woche wählen");
  QToolTip::add(d->todayButton, "Heutigen Tag wählen");

  // -----
  setFontSize(fontsize);
  line->setValidator(val);
  line->installEventFilter( this );

  setDate(dt); // set button texts
  connect(table, SIGNAL(selectionChanged()), SLOT(dateChangedSlot()));
  connect(table, SIGNAL(activated(QDate)), SLOT(tableClickedSlot(QDate)));
  connect(table,SIGNAL(currentPageChanged(int, int)), SLOT(currentPageChangedSlot(int, int)));
  connect(d->selectWeek, SIGNAL(activated(int)), SLOT(weekSelected(int)));
  connect(d->todayButton, SIGNAL(clicked()), SLOT(todayButtonClicked()));
  connect(line, SIGNAL(returnPressed()), SLOT(lineEnterPressed()));
  table->setFocus();

  topLayout->addWidget(table);

  QBoxLayout * bottomLayout = new QHBoxLayout(topLayout);
  bottomLayout->addWidget(d->todayButton);
  bottomLayout->addWidget(line);
  bottomLayout->addWidget(d->selectWeek);
}

QDatePicker::~QDatePicker()
{
  delete d;
}

bool
QDatePicker::eventFilter(QObject *o, QEvent *e )
{
 /*  if ( e->type() == QEvent::KeyPress ) {
      QKeyEvent *k = (QKeyEvent *)e;

      if ( (k->key() == Qt::Key_Prior) ||
           (k->key() == Qt::Key_Next)  ||
           (k->key() == Qt::Key_Up)    ||
           (k->key() == Qt::Key_Down) )
       {
          QApplication::sendEvent( table, e );
          table->setFocus();
          return true; // eat event
       }
   }*/
   return Q3Frame::eventFilter( o, e );
}

void
QDatePicker::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);
}

void QDatePicker::currentPageChangedSlot (int year, int month)
{
  QDate date=QDate(year,month,1);
  fillWeeksCombo(date);
  d->selectWeek->setCurrentItem(date.weekNumber() - 1);
}

void
QDatePicker::dateChangedSlot()
{
    QDate date=table->selectedDate();
    line->setText(date.toString(Qt::ISODate));

    emit(dateChanged(date));
}

void
QDatePicker::tableClickedSlot(QDate date)
{
  emit(dateSelected(date));
  emit(tableClicked());
}

/*const QDate&
QDatePicker::getDate() const
{
  return table->getDate();
}*/

const QDate
QDatePicker::date() const
{
    return table->selectedDate();
}

bool
QDatePicker::setDate(const QDate& date)
{
    if(date.isValid())
    {
        table->setSelectedDate(date);
        fillWeeksCombo(date);
        d->selectWeek->setCurrentItem(date.weekNumber() - 1);
        line->setText(date.toString(Qt::ISODate));
        return true;
    }
    else
    {
        return false;
    }
}

void
QDatePicker::weekSelected(int week)
{
  week++; // week number starts with 1

  int year = table->yearShown();

  QDate date = QDate(year, 1, 1);
  date=date.addDays(-7);
  while (date.weekNumber() != 1)
    date=date.addDays(1);

  // date is now first day in week 1 some day in week 1
  date=date.addDays((week - date.weekNumber()) * 7);

  setDate(date);
}

void
QDatePicker::setEnabled(bool enable)
{
  QWidget *widgets[]= {
    line, table, d->selectWeek, d->todayButton };
  const int Size=sizeof(widgets)/sizeof(widgets[0]);
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      widgets[count]->setEnabled(enable);
    }
}

void
QDatePicker::lineEnterPressed()
{
  QDate temp;
  // -----
  if(val->date(line->text(), temp)==QValidator::Acceptable)
    {
        emit(dateEntered(temp));
        setDate(temp);
    }
}

void
QDatePicker::todayButtonClicked()
{
  setDate(QDate::currentDate());
}

QSize
QDatePicker::sizeHint() const
{
  return QWidget::sizeHint();
}

void
QDatePicker::setFontSize(int s)
{
  QFont currfont=table->font();
  currfont.setPointSize(s);
  table->setFont(currfont);
}

void QDatePicker::virtual_hook( int /*id*/, void* /*data*/ )
{ /*BASE::virtual_hook( id, data );*/ }

