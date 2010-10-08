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

#ifndef TIMEEDIT_H
#define TIMEEDIT_H

#include <QSpinBox>
#include <iostream>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

/**
  * TimeEdit ist ein QSpinBox-Objekt, das zusaetzlich Signale zum verschalten der "Scrollbuttons" mehrerer
  * TimeEdits liefert
  */

class TimeEdit: public QSpinBox
{

  Q_OBJECT

  public:

  /**
   * Erzeugt ein TimeEdit-Objekt mit Wertebereich 0..maxVal.
   */
  TimeEdit(int maxVal, QWidget* parent): QSpinBox(parent)
  {
     setMinimum( 0 );
     setMaximum( maxVal );
     setSingleStep( 1 );
  }

  /** Es wird statt dessen ein ein stepButtonPressed Signal ausgeloest.
   * (noetig zum gleichschalten mehrerer SpinBoxes).
   */
  virtual void stepBy(int s)
  {
    emit stepButtonPressed(s);
  }

  signals:

    /** Wird durch stepBy ausgeloest */
    void stepButtonPressed(int s);

  public slots:

      /** erhoeht/verringert tatsaechlich um step schritte. */
    void doStep(int s)
    {
      setValue(value()+s);
    }
};


/**
 * ZeitBox ist ein von QGroupBox abgeleitetes Widget, das eine Spinbox
 * zum editieren der Minuten und eine zum editieren der Stunden verbindet.
 */
class ZeitBox: public QGroupBox
{
  Q_OBJECT

  public:

    /**
    * Erzeugt eine neue Zeitbox mit der Ueberschrift Titel und der voreingestellten Zeit sekunden.
    */
    ZeitBox(QString titel, int _sekunden, QWidget* parent=0):QGroupBox(titel,parent)
    {
      QHBoxLayout *layout = new QHBoxLayout(this);
      sekunden=_sekunden;
      hourEdit = new TimeEdit(24,this);
      hourEdit->setValue(sekunden/3600);
      layout->addStretch(2);
      layout->addWidget(hourEdit);
      layout->addWidget(new QLabel("h",this));
      layout->addStretch(1);
      minuteEdit = new TimeEdit(60,this);
      minuteEdit->setValue((sekunden/60)%60);
      minuteEdit->setWrapping(true);
      layout->addWidget(minuteEdit);
      layout->addWidget(new QLabel("m",this));
      layout->addStretch(2);
      connect(minuteEdit,SIGNAL(stepButtonPressed(int)),this,SLOT(doStepMin(int)));
      connect(hourEdit,SIGNAL(stepButtonPressed(int)),this,SLOT(doStepHour(int)));
      connect(minuteEdit,SIGNAL(valueChanged(int)),this,SLOT(setMinutes(int)));
      connect(hourEdit,SIGNAL(valueChanged(int)),this,SLOT(setHours(int)));
    }

    int getSekunden()
    {
      hourEdit->value(); // Das auslesen der Werte bewirkt, dass per Hand eingegebene
      minuteEdit->value(); // Eintraege ausgewertet werden, und minuteChanged bzw hourChanged ausgeloest wird.
      return sekunden;
    };

    void setSekunden(const int _sekunden)
    {
      sekunden=_sekunden;
      minuteEdit->setValue((sekunden/60)%60);
      hourEdit->setValue(sekunden/3600);
    }


  public slots:
    void setMinutes(int newVal)
    {
      sekunden=(sekunden/3600)*3600+newVal*60; //Stunden unveraendert lassen, Neue Minuten uebernehmen
    }

    void setHours(int newVal)
    {
      sekunden=(sekunden%3600)+newVal*3600; //Minuten unveraendert lassen, neue Stunden uebernehmen
    }

    /**
     * Veraendert die Minuten um Steps
     */
    void doStepMin(int steps)
    {
      sekunden+=steps*60;
      if (sekunden<0) sekunden=0;
      minuteEdit->setValue((sekunden/60)%60);
      hourEdit->setValue(sekunden/3600);
      emit minuteChangedBy(steps);
    }

    void incrMin()
    {
        doStepMin(1);
    }

    /**
     * Veraendert die Stunden um Steps
     */
    void doStepHour(int steps)
    {
      sekunden+=3600*steps;
      if (sekunden<0) sekunden=0;
      hourEdit->setValue(sekunden/3600);
      emit hourChangedBy(steps);
    }

    void setReadOnly(bool readonly)
    {
      hourEdit->setReadOnly(readonly);
      minuteEdit->setReadOnly(readonly);
    }

  signals:
    /**
     * Wird durch doStepMinMin() ausgeloest
     */
    void minuteChangedBy(int steps);

    /**
     * Wird durch doStepHour() ausgeloest
     */
    void hourChangedBy(int steps);

  private:
    TimeEdit *minuteEdit;
    TimeEdit *hourEdit;
    int sekunden;
};

#endif
