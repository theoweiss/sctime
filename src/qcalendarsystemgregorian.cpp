/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>
   Copyright (c) 2003 Florian Schmitt <f.schmitt@science-computing.de>

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

// Derived gregorian kde calendar class
// Just a schema.

#include <qdatetime.h>
#include <qstring.h>

#include "qcalendarsystemgregorian.h"

QCalendarSystemGregorian::QCalendarSystemGregorian()
  : QCalendarSystem()
{
}

QCalendarSystemGregorian::~QCalendarSystemGregorian()
{
}

int QCalendarSystemGregorian::year(const QDate& date) const
{
  return date.year();
}

int QCalendarSystemGregorian::monthsInYear( const QDate & date ) const
{
  Q_UNUSED( date )

  return 12;
}

int QCalendarSystemGregorian::weeksInYear(int year) const
{
  QDate temp;
  temp.setYMD(year, 12, 31);

  // If the last day of the year is in the first week, we have to check the
  // week before
  if ( temp.weekNumber() == 1 )
    temp = temp.addDays(-7);

  return temp.weekNumber();
}

int QCalendarSystemGregorian::weekNumber(const QDate& date,
                                         int * yearNum) const
{
  return date.weekNumber(yearNum);
}

QString QCalendarSystemGregorian::monthName(const QDate& date,
                                            bool shortName) const
{
  return monthName(month(date), shortName);
}

QString QCalendarSystemGregorian::monthNamePossessive(const QDate& date, bool shortName) const
{
  return monthNamePossessive(month(date), shortName);
}

QString QCalendarSystemGregorian::monthName(int month, int year, bool shortName) const
{
  Q_UNUSED(year);

  if ( shortName )
    switch ( month )
      {
      case 1:
        return "Jan";
      case 2:
        return "Feb";
      case 3:
        return "Mar";
      case 4:
        return "Apr";
      case 5:
        return "Mai";
      case 6:
        return "Jun";
      case 7:
        return "Jul";
      case 8:
        return "Aug";
      case 9:
        return "Sep";
      case 10:
        return "Okt";
      case 11:
        return "Nov";
      case 12:
        return "Dez";
      }
  else
    switch ( month )
      {
      case 1:
        return "Januar";
      case 2:
        return "Februar";
      case 3:
        return "Maerz";
      case 4:
        return "April";
      case 5:
        return "Mai";
      case 6:
        return "Juni";
      case 7:
        return "Juli";
      case 8:
        return "August";
      case 9:
        return "September";
      case 10:
        return "Oktober";
      case 11:
        return "November";
      case 12:
        return "Dezember";
      }

  return QString::null;
}

QString QCalendarSystemGregorian::monthNamePossessive(int month, int year,
                                                      bool shortName) const
{
  Q_UNUSED(year);
  return monthName(month,year,shortName);

  /*if ( shortName )
    switch ( month )
      {
      case 1:
        return locale()->translate("of January", "of Jan");
      case 2:
        return locale()->translate("of February", "of Feb");
      case 3:
        return locale()->translate("of March", "of Mar");
      case 4:
        return locale()->translate("of April", "of Apr");
      case 5:
        return locale()->translate("of May short", "of May");
      case 6:
        return locale()->translate("of June", "of Jun");
      case 7:
        return locale()->translate("of July", "of Jul");
      case 8:
        return locale()->translate("of August", "of Aug");
      case 9:
        return locale()->translate("of September", "of Sep");
      case 10:
        return locale()->translate("of October", "of Oct");
      case 11:
       return locale()->translate("of November", "of Nov");
      case 12:
        return locale()->translate("of December", "of Dec");
      }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("of January");
      case 2:
        return locale()->translate("of February");
      case 3:
        return locale()->translate("of March");
      case 4:
        return locale()->translate("of April");
      case 5:
        return locale()->translate("of May long", "of May");
      case 6:
        return locale()->translate("of June");
      case 7:
        return locale()->translate("of July");
      case 8:
        return locale()->translate("of August");
      case 9:
        return locale()->translate("of September");
      case 10:
        return locale()->translate("of October");
      case 11:
        return locale()->translate("of November");
      case 12:
        return locale()->translate("of December");
      }

  return QString::null;*/
}

bool QCalendarSystemGregorian::setYMD(QDate & date, int y, int m, int d) const
{
  // We don't want Qt to add 1900 to them
  if ( y >= 0 && y <= 99 )
    return false;

  // QDate supports gregorian internally
  return date.setYMD(y, m, d);
}

QDate QCalendarSystemGregorian::addYears(const QDate & date, int nyears) const
{
  return date.addYears(nyears);
}

QDate QCalendarSystemGregorian::addMonths(const QDate & date, int nmonths) const
{
  return date.addMonths(nmonths);
}

QDate QCalendarSystemGregorian::addDays(const QDate & date, int ndays) const
{
  return date.addDays(ndays);
}

QString QCalendarSystemGregorian::weekDayName(int col, bool shortName) const
{
  // ### Should this really be different to each calendar system? Or are we
  //     only going to support weeks with 7 days?

  //kdDebug(5400) << "Gregorian wDayName" << endl;
  if (shortName)
    switch (col % 7) {
      case 0:
        return "So";
      case 1:
        return "Mo";
      case 2:
        return "Di";
      case 3:
        return "Mi";
      case 4:
        return "Do";
      case 5:
        return "Fr";
      case 6:
        return "Sa";
  } else
    switch (col % 7) {
      case 0:
        return "Sonntag";
      case 1:
        return "Montag";
      case 2:
        return "Dienstag";
      case 3:
        return "Mittwoch";
      case 4:
        return "Donnerstag";
      case 5:
        return "Freitag";
      case 6:
        return "Samstag";
  }
  return QString::null;
}

QString QCalendarSystemGregorian::weekDayName(const QDate& date, bool shortName) const
{
  return weekDayName(dayOfWeek(date), shortName);
}


int QCalendarSystemGregorian::dayOfWeek(const QDate& date) const
{
  return date.dayOfWeek();
}

int QCalendarSystemGregorian::dayOfYear(const QDate & date) const
{
  return date.dayOfYear();
}

int QCalendarSystemGregorian::daysInMonth(const QDate& date) const
{
  return date.daysInMonth();
}

int QCalendarSystemGregorian::minValidYear() const
{
  return 1753; // QDate limit
}

int QCalendarSystemGregorian::maxValidYear() const
{
  return 8000; // QDate limit
}

int QCalendarSystemGregorian::day(const QDate& date) const
{
  return date.day();
}

int QCalendarSystemGregorian::month(const QDate& date) const
{
  return date.month();
}

int QCalendarSystemGregorian::daysInYear(const QDate& date) const
{
  return date.daysInYear();
}

int QCalendarSystemGregorian::weekDayOfPray() const
{
  return 7; // sunday
}

QString QCalendarSystemGregorian::calendarName() const
{
  return QString::fromLatin1("gregorian");
}

bool QCalendarSystemGregorian::isLunar() const
{
  return false;
}

bool QCalendarSystemGregorian::isLunisolar() const
{
  return false;
}

bool QCalendarSystemGregorian::isSolar() const
{
  return true;
}

int QCalendarSystemGregorian::yearStringToInteger(const QString & sNum, int & iLength) const
{
  int iYear;
  iYear = QCalendarSystem::yearStringToInteger(sNum, iLength);

  // Qt treats a year in the range 0-100 as 1900-1999.
  // It is nicer for the user if we treat 0-68 as 2000-2068
  if (iYear < 69)
    iYear += 2000;
  else if (iYear < 100)
    iYear += 1900;

  return iYear;
}
