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

#include "qspinbox.h"
#include <iostream>
#include "q3groupbox.h"
#include "qlabel.h"

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
  TimeEdit(int maxVal, QWidget* parent): QSpinBox(0,maxVal,1,parent) { }

  /** Es wird statt dem Erhoehen um eins ein upButtonPressed Signal ausgeloest.
   * (noetig zum gleichschalten mehrerer SpinBoxes).
   */
  virtual void stepUp()
  {
  //  QSpinBox::stepUp();
    emit upButtonPressed();
  }


  /** Es wird statt dem Verringern um eins ein downButtonPressed Signal ausgeloest.
   * (noetig zum gleichschalten mehrerer SpinBoxes).
   */
  virtual void stepDown()
  {
 //   QSpinBox::stepDown();
    emit downButtonPressed();
  }


  signals:
    
    /** Wird durch stepUp ausgeloest */
    void upButtonPressed();

    /** Wird durch StepDown ausgeloest */
    void downButtonPressed();

  public slots:
  
    /** Erhoeht den Wert um eins, ohne ein *ButtonPressed Signal zu erzeugen. */
    void incr()
    {
      setValue(value()+1);
    }
    
    /** Verringert den Wert um eins, ohne ein *ButtonPressed Signal zu erzeugen. */
    void decr()
    {
      setValue(value()-1);
    }
};


/**
 * ZeitBox ist ein von QGroupBox abgeleitetes Widget, das eine Spinbox
 * zum editieren der Minuten und eine zum editieren der Stunden verbindet.
 */
class ZeitBox: public Q3GroupBox
{
  Q_OBJECT
  
  public:

    /**
    * Erzeugt eine neue Zeitbox mit der Ueberschrift Titel und der voreingestellten Zeit sekunden.
    */
    ZeitBox(QString titel, int _sekunden, QWidget* parent=0):Q3GroupBox(5,Qt::Horizontal,titel,parent)
    {
      sekunden=_sekunden;
      hourEdit = new TimeEdit(24,this);
      hourEdit->setValue(sekunden/3600);
      new QLabel("h",this);
      addSpace(0);
      minuteEdit = new TimeEdit(60,this);
      minuteEdit->setValue((sekunden/60)%60);
      minuteEdit->setWrapping(true);
      new QLabel("m",this);
      connect(minuteEdit,SIGNAL(upButtonPressed()),this,SLOT(incrMin()));
      connect(minuteEdit,SIGNAL(downButtonPressed()),this,SLOT(decrMin()));
      connect(hourEdit,SIGNAL(upButtonPressed()),this,SLOT(incrHour()));
      connect(hourEdit,SIGNAL(downButtonPressed()),this,SLOT(decrHour()));
      connect(minuteEdit,SIGNAL(valueChanged(int)),this,SLOT(minuteChanged(int)));
      connect(hourEdit,SIGNAL(valueChanged(int)),this,SLOT(hourChanged(int)));
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
    void minuteChanged(int newVal)
    {
      sekunden=(sekunden/3600)*3600+newVal*60; //Stunden unveraendertlassen, Neue Minuten uebernehmen
    }

    void hourChanged(int newVal)
    {
      sekunden=(sekunden%3600)+newVal*3600; //Minuten unveraendert lassen, neue Stunden uebernehmen
    }

    /**
     * Erhoeht die Minuten um eine
     */
    void incrMin()
    {
      sekunden+=60;
      minuteEdit->setValue((sekunden/60)%60);
      hourEdit->setValue(sekunden/3600);
      emit minuteUp();
    }

    /**
     * Verringert die Minuten um eine
     */
    void decrMin()
    {
      if (sekunden>=60) sekunden-=60; else sekunden=0;
      minuteEdit->setValue((sekunden/60)%60);
      hourEdit->setValue(sekunden/3600);
      emit minuteDown();
    }

    /**
     * Erhoeht die Stunden um eine
     */
    void incrHour()
    {
      sekunden+=3600;
      hourEdit->setValue(sekunden/3600);
      emit hourUp();
    }

    /**
     * Verringert die Stunden um eine
     */
    void decrHour()
    {
      if (sekunden>=3600) sekunden-=3600; else sekunden=0;
      minuteEdit->setValue((sekunden/60)%60);
      hourEdit->setValue(sekunden/3600);
      emit hourDown();
    }

  signals:
    /**
     * Wird durch incrMin() ausgeloest
     */
    void minuteUp();

    /**
     * Wird durch decrMin() ausgeloest
     */
    void minuteDown();

    /**
     * Wird durch incrHour() ausgeloest
     */
    void hourUp();

    /**
     * Wird durch decrHour() ausgeloest
     */
    void hourDown();

  private:
    TimeEdit *minuteEdit;
    TimeEdit *hourEdit;
    int sekunden;
};

#endif
