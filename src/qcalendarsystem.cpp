/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>
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

// Gregorian calendar system implementation factory for creation of calendar
// systems.
// Also default gregorian and factory classes


#include "qcalendarsystem.h"

QCalendarSystem::QCalendarSystem()
{
}

QCalendarSystem::~QCalendarSystem()
{
}

QString QCalendarSystem::dayString(const QDate & pDate, bool bShort) const
{
  QString sResult;

  sResult.setNum(day(pDate));
  if (!bShort && sResult.length() == 1 )
    sResult.prepend('0');

  return sResult;
}

QString QCalendarSystem::monthString(const QDate & pDate, bool bShort) const
{
  QString sResult;

  sResult.setNum(month(pDate));
  if (!bShort && sResult.length() == 1 )
    sResult.prepend('0');

  return sResult;
}

QString QCalendarSystem::yearString(const QDate & pDate, bool bShort) const
{
  QString sResult;

  sResult.setNum(year(pDate));
  if (!bShort && sResult.length() == 1 )
    sResult.prepend('0');

  return sResult;
}

static int stringToInteger(const QString & sNum, int & iLength)
{
  unsigned int iPos = 0;

  int result = 0;
  for (; sNum.length() > iPos && sNum.at(iPos).isDigit(); iPos++)
    {
      result *= 10;
      result += sNum.at(iPos).digitValue();
    }

  iLength = iPos;
  return result;
}


int QCalendarSystem::dayStringToInteger(const QString & sNum, int & iLength) const
{
  return stringToInteger(sNum, iLength);
}

int QCalendarSystem::monthStringToInteger(const QString & sNum, int & iLength) const
{
  return stringToInteger(sNum, iLength);
}

int QCalendarSystem::yearStringToInteger(const QString & sNum, int & iLength) const
{
  return stringToInteger(sNum, iLength);
}
