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

#ifndef STATUSBAR_H
#define  STATUSBAR_H

#include <QWidget>
#include <QString>
#include <QStatusBar>
#include <QLabel>
#include <QDateTime>
#include "timecounter.h"

/** The status bar of the main window */
class StatusBar:public QStatusBar
{
  Q_OBJECT

  public:
    StatusBar(QWidget * parent = 0);
    virtual ~StatusBar() {};

  public slots:
    void setSekunden(int sec);
    void repaintZeitFeld();
    void setDiff(int sec);
    void dateWarning(bool on, QDate datum=QDate::currentDate());
    void appendWarning(bool on, QString str);
    void setMode(QString modedesc, bool on);

  private:
    QLabel* zeitLabel;
    QLabel* datumsWarnung;
    QLabel* modeList;
    QSet<QString> modes;
    int secDiff;
    int sekunden;
};

#endif


