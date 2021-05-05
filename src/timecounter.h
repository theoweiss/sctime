/*
    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt et al.

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
      return QString("%1:%2 ").arg(seconds/3600,2).arg((seconds%3600)/60,2,10,QChar('0'));
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
