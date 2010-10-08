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

#ifndef TIMECOUNTER_H
#define TIMECOUNTER_H

#include <QString>
#include <QStringList>

/**
 * Simple Klasse, die einen Sekundenzaehler, sowie eine formatierte Ausgabe dazu bietet.
 */
class TimeCounter {
  public:
    TimeCounter() { seconds=0; }
    TimeCounter(const int& sec) {seconds=sec;}
    TimeCounter(const TimeCounter& tc) {seconds=tc.seconds; }

    void addTime(const int& sec)
    {
      seconds+=sec;
    }

    void addTime(const TimeCounter& tc)
    {
      seconds+=tc.seconds;
    }

    QString toString()
    {
      QString qs;

      return qs.sprintf("%2i:%.2i ",seconds/3600, (seconds%3600)/60);
    }

    static TimeCounter fromString(QString str)
    {
      str=str.trimmed();
      QStringList l=str.split(":");
      if (l.size()<2) return TimeCounter(0);
      int secs=60*(60*l[0].toInt()+l[1].toInt());
      return TimeCounter(secs);
    }

  private:
    int seconds;
};

#endif
