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

#include "qdatepicker.h"

#include <qapplication.h>
#include "qcalendarsystem.h"

#include "qdatetbl.h"
//Added by qt3to4:
#include <QBoxLayout>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>

#include "../pics/cr16-action-1leftarrow.xpm"
#include "../pics/cr16-action-1rightarrow.xpm"
#include "../pics/cr16-action-2rightarrow.xpm"
#include "../pics/cr16-action-2leftarrow.xpm"
#include "../pics/cr16-action-bookmark.xpm"


class QDatePicker::QDatePickerPrivate
{
public:
    QDatePickerPrivate() : closeButton(0L), selectWeek(0L), todayButton(0), navigationLayout(0) {}

    void fillWeeksCombo(const QDate &date);

    QToolButton *closeButton;
    QComboBox *selectWeek;
    QToolButton *todayButton;
    QBoxLayout *navigationLayout;
};

void QDatePicker::fillWeeksCombo(const QDate &date)
{
  // every year can have a different number of weeks
  int i, weeks = calendar->weeksInYear(calendar->year(date));

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

  calendar=new QCalendarSystemGregorian();

  //d->tb = new QToolBar("DatePicker-Toolbar",this);

  QBoxLayout * topLayout = new QVBoxLayout(this);

  d->navigationLayout = new QHBoxLayout(topLayout);
  d->navigationLayout->addStretch();
  yearBackward = new QToolButton(this);
  yearBackward->setAutoRaise(true);
  d->navigationLayout->addWidget(yearBackward);
  monthBackward = new QToolButton(this);
  monthBackward ->setAutoRaise(true);
  d->navigationLayout->addWidget(monthBackward);
  d->navigationLayout->addSpacing(1);

  selectMonth = new QToolButton(this);
  selectMonth ->setAutoRaise(true);
  d->navigationLayout->addWidget(selectMonth);
  selectYear = new QToolButton(this);
  selectYear->setToggleButton(true);
  selectYear->setAutoRaise(true);
  d->navigationLayout->addWidget(selectYear);
  d->navigationLayout->addSpacing(1);

  monthForward = new QToolButton(this);
  monthForward ->setAutoRaise(true);
  d->navigationLayout->addWidget(monthForward);
  yearForward = new QToolButton(this);
  yearForward ->setAutoRaise(true);
  d->navigationLayout->addWidget(yearForward);
  d->navigationLayout->addStretch();

  line = new QLineEdit(this);
  val = new QDateValidator(this);
  table = new QDateTable(calendar, this);
  QFont font;
  fontsize = font.pointSize();
  if (fontsize == -1)
     fontsize = QFontInfo(font).pointSize();

  fontsize++; // Make a little bigger

  d->selectWeek = new QComboBox(false, this);  // read only week selection
  d->todayButton = new QToolButton(this);
  d->todayButton->setIconSet(QIcon(QPixmap((const char **)cr16_action_bookmark)));

  QToolTip::add(yearForward, "Nächstes Jahr");
  QToolTip::add(yearBackward, "Voriges Jahr");
  QToolTip::add(monthForward, "Nächster Monat");
  QToolTip::add(monthBackward, "Voriger Monat");
  QToolTip::add(d->selectWeek, "Woche wählen");
  QToolTip::add(selectMonth, "Monat wählen");
  QToolTip::add(selectYear, "Jahr wählen");
  QToolTip::add(d->todayButton, "Heutigen Tag wählen");

  // -----
  setFontSize(fontsize);
  line->setValidator(val);
  line->installEventFilter( this );

  yearForward->setIconSet(QIcon(QPixmap((const char **)cr16_action_2rightarrow)));
  yearBackward->setIconSet(QIcon(QPixmap((const char **)cr16_action_2leftarrow)));
  monthForward->setIconSet(QIcon(QPixmap((const char **)cr16_action_1rightarrow)));
  monthBackward->setIconSet(QIcon(QPixmap((const char **)cr16_action_1leftarrow)));

  setDate(dt); // set button texts
  connect(table, SIGNAL(dateChanged(QDate)), SLOT(dateChangedSlot(QDate)));
  connect(table, SIGNAL(tableClicked()), SLOT(tableClickedSlot()));
  connect(table, SIGNAL(tableDoubleClicked()), SLOT(tableDoubleClickedSlot()));
  connect(monthForward, SIGNAL(clicked()), SLOT(monthForwardClicked()));
  connect(monthBackward, SIGNAL(clicked()), SLOT(monthBackwardClicked()));
  connect(yearForward, SIGNAL(clicked()), SLOT(yearForwardClicked()));
  connect(yearBackward, SIGNAL(clicked()), SLOT(yearBackwardClicked()));
  connect(d->selectWeek, SIGNAL(activated(int)), SLOT(weekSelected(int)));
  connect(d->todayButton, SIGNAL(clicked()), SLOT(todayButtonClicked()));
  connect(selectMonth, SIGNAL(clicked()), SLOT(selectMonthClicked()));
  connect(selectYear, SIGNAL(clicked()), SLOT(selectYearClicked()));
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
  delete calendar;
}

bool
QDatePicker::eventFilter(QObject *o, QEvent *e )
{
   if ( e->type() == QEvent::KeyPress ) {
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
   }
   return Q3Frame::eventFilter( o, e );
}

void
QDatePicker::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);
}

void
QDatePicker::dateChangedSlot(QDate date)
{
    line->setText(date.toString(Qt::ISODate));
    selectMonth->setText(calendar->monthName(date, false));
    fillWeeksCombo(date);
    d->selectWeek->setCurrentItem(calendar->weekNumber(date) - 1);
    selectYear->setText(calendar->yearString(date, false));

    emit(dateChanged(date));
}

void
QDatePicker::tableClickedSlot()
{
  emit(dateSelected(table->getDate()));
  emit(tableClicked());
}

void
QDatePicker::tableDoubleClickedSlot()
{
  emit(dateSelected(table->getDate()));
  emit(tableDoubleClicked());
}

/*const QDate&
QDatePicker::getDate() const
{
  return table->getDate();
}*/

const QDate &
QDatePicker::date() const
{
    return table->getDate();
}

bool
QDatePicker::setDate(const QDate& date)
{
    if(date.isValid())
    {
        table->setDate(date);
        fillWeeksCombo(date);
        d->selectWeek->setCurrentItem(calendar->weekNumber(date) - 1);
        selectMonth->setText(calendar->monthName(date, false));
        selectYear->setText(calendar->yearString(date, true));
        line->setText(date.toString(Qt::ISODate));
        return true;
    }
    else
    {
        return false;
    }
}

void
QDatePicker::monthForwardClicked()
{
    QDate temp;
    temp = calendar->addMonths( table->getDate(), 1 );

    setDate( temp );
}

void
QDatePicker::monthBackwardClicked()
{
    QDate temp;
    temp = calendar->addMonths( table->getDate(), -1 );

    setDate( temp );
}

void
QDatePicker::yearForwardClicked()
{
    QDate temp;
    temp = calendar->addYears( table->getDate(), 1 );

    setDate( temp );
}

void
QDatePicker::yearBackwardClicked()
{
    QDate temp;
    temp = calendar->addYears( table->getDate(), -1 );

    setDate( temp );
}

//void QDatePicker::selectWeekClicked() {}  // ### in 3.2 obsolete; kept for binary compatibility

void
QDatePicker::weekSelected(int week)
{
  week++; // week number starts with 1

  QDate date = table->getDate();
  int year = calendar->year(date);

  calendar->setYMD(date, year, 1, 1);
  date = calendar->addDays(date, -7);
  while (calendar->weekNumber(date) != 1)
    date = calendar->addDays(date, 1);

  // date is now first day in week 1 some day in week 1
  date = calendar->addDays(date, (week - calendar->weekNumber(date)) * 7);

  setDate(date);
}

void
QDatePicker::selectMonthClicked()
{
  // every year can have different month names (in some calendar systems)
  QDate date = table->getDate();
  int i, month, months = calendar->monthsInYear(date);

  Q3PopupMenu popup(selectMonth);

  for (i = 1; i <= months; i++)
    popup.insertItem(calendar->monthName(i, calendar->year(date)), i);

  QMenuItem *item = popup.findItem (calendar->month(date));
  if (item)
    popup.setActiveAction(item);

  if ( (month = popup.exec(selectMonth->mapToGlobal(QPoint(0, 0)), calendar->month(date) - 1)) == -1 ) return;  // canceled

  int day = calendar->day(date);
  // ----- construct a valid date in this month:
  //date.setYMD(date.year(), month, 1);
  //date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));
  calendar->setYMD(date, calendar->year(date), month,
                   QMIN(day, calendar->daysInMonth(date)));
  // ----- set this month
  setDate(date);
}

void
QDatePicker::selectYearClicked()
{
  int year;
  QPopupFrame* popup = new QPopupFrame(this);
  QDateInternalYearSelector* picker = new QDateInternalYearSelector(calendar, popup);
  // -----
  picker->resize(picker->sizeHint());
  popup->setMainWidget(picker);
  connect(picker, SIGNAL(closeMe(int)), popup, SLOT(close(int)));
  picker->setFocus();
  if(popup->exec(selectYear->mapToGlobal(QPoint(0, selectMonth->height()))))
    {
      QDate date;
      int day;
      // -----
      year=picker->getYear();
      date=table->getDate();
      day=calendar->day(date);
      // ----- construct a valid date in this month:
      //date.setYMD(year, date.month(), 1);
      //date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));
      calendar->setYMD(date, year, calendar->month(date),
                       QMIN(day, calendar->daysInMonth(date)));
      // ----- set this month
      setDate(date);
    } else {
    }
  delete popup;
}

void
QDatePicker::setEnabled(bool enable)
{
  QWidget *widgets[]= {
    yearForward, yearBackward, monthForward, monthBackward,
    selectMonth, selectYear,
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
  QWidget *buttons[]= {
    // yearBackward,
    // monthBackward,
    selectMonth,
    selectYear,
    // monthForward,
    // yearForward
  };
  const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
  int count;
  QFont font;
  QRect r;
  // -----
  fontsize=s;
  for(count=0; count<NoOfButtons; ++count)
    {
      font=buttons[count]->font();
      font.setPointSize(s);
      buttons[count]->setFont(font);
    }
  QFontMetrics metrics(selectMonth->fontMetrics());

  for (int i = 1; ; ++i)
    {
      QString str = calendar->monthName(i,
         calendar->year(table->getDate()), false);
      if (str.isNull()) break;
      r=metrics.boundingRect(str);
      maxMonthRect.setWidth(QMAX(r.width(), maxMonthRect.width()));
      maxMonthRect.setHeight(QMAX(r.height(),  maxMonthRect.height()));
    }

    QStyleOptionToolButton qstb;

    QSize metricBound = style()->sizeFromContents(QStyle::CT_ToolButton, &qstb,
                                               maxMonthRect, selectMonth);


  selectMonth->setMinimumSize(metricBound);


  table->setFontSize(s);
}

void
QDatePicker::setCloseButton( bool enable )
{
    if ( enable == (d->closeButton != 0L) )
        return;

    if ( enable ) {
        d->closeButton = new QToolButton( this );
        d->navigationLayout->addWidget(d->closeButton);
        QToolTip::add(d->closeButton,"Schliessen");
        //d->closeButton->setPixmap( SmallIcon("remove") );
        connect( d->closeButton, SIGNAL( clicked() ),
                 topLevelWidget(), SLOT( close() ) );
    }
    else {
        delete d->closeButton;
        d->closeButton = 0L;
    }

    updateGeometry();
}

bool QDatePicker::hasCloseButton() const
{
    return (d->closeButton != 0L);
}

void QDatePicker::virtual_hook( int /*id*/, void* /*data*/ )
{ /*BASE::virtual_hook( id, data );*/ }

