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

#include "statusbar.h"

StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent)
{
    zeitLabel = new QLabel(tr("Overall time: ") + "0", this);
    addPermanentWidget(zeitLabel);
    connect(parent, SIGNAL(gesamtZeitChanged(int)), this, SLOT(setSekunden(int)));
    datumsWarnung = new QLabel("", this);
    addWidget(datumsWarnung);
    modeList = new QLabel("", this);
    addWidget(modeList);
    secDiff = 0;
    sekunden = 0;
}

void StatusBar::setSekunden(int sec)
{
    sekunden = sec;
    repaintZeitFeld();
}

void StatusBar::repaintZeitFeld()
{
    TimeCounter tc(sekunden);
    QString text;
    text = tc.toString();
    if (secDiff / 60 != 0)
    {
        QString diffstr;
        diffstr.setNum(secDiff / 60);
        if (secDiff > 0)
            text = text + "/+" + diffstr;
        else
            text = text + "/" + diffstr;
    }
    zeitLabel->setText(tr("Overall time: ") + text);
}

void StatusBar::setDiff(int sec)
{
    secDiff = sec;
    repaintZeitFeld();
}

void StatusBar::dateWarning(bool on, QDate datum)
{
    if (on)
    {
        datumsWarnung->setText(tr("Warning: Non-current date %1 is being edited.").arg(datum.toString(Qt::SystemLocaleShortDate)));
        datumsWarnung->setStyleSheet("color:#800000;");
    }
    else
    {
        datumsWarnung->setText("");
    }
}

void StatusBar::appendWarning(bool on, QString str)
{
    if (on)
    {
        QString labelTxt = datumsWarnung->text();
        datumsWarnung->setText(labelTxt + str);
    }
    else
    {
        datumsWarnung->setText("");
    }
}

void StatusBar::setMode(QString modedesc, bool on)
{
    if (on)
    {
        modes.insert(modedesc);
    }
    else
    {
        modes.remove(modedesc);
    }
    QString modestr = "";
    foreach (QString m, modes)
    {
        modestr += " "+m+" |";
    }
    if (!modestr.isEmpty()) {
        modestr="|"+modestr;
        modeList->setStyleSheet("color:#000070;");
    }
    modeList->setText(modestr);
}