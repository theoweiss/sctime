/* $Id$ */

#ifndef STATUSBAR_H
#define  STATUSBAR_H

#include "qmainwindow.h"
#include <qstring.h>
#include "qstatusbar.h"
#include "qlabel.h"
#include "timecounter.h"
#include "qdatetime.h"

/** Die Statusbar des Hauptfensters */
class StatusBar:public QStatusBar
{
  Q_OBJECT

  public:
    StatusBar(QMainWindow * parent = 0):QStatusBar(parent)
    {
      addWidget(new QLabel("Gesamtzeit:",this));
      zeitLabel=new QLabel("0",this);
      addWidget(zeitLabel);
      connect(parent,SIGNAL(gesamtZeitChanged(int)),this, SLOT(setSekunden(int)));
      datumsWarnung=new QLabel("",this);
      addWidget(datumsWarnung);
      secDiff=0;
      sekunden=0;
    }

  public slots:
    void setSekunden(int sec)
    {
      sekunden=sec;
      repaintZeitFeld();
    }
    
    void repaintZeitFeld()
    {
      TimeCounter tc(sekunden);
      QString text;
      text=tc.toString();
      if (secDiff/60!=0) {
        QString diffstr;
        diffstr.setNum(secDiff/60);
        if (secDiff>0)
          text=text+"/+"+diffstr;
        else
          text=text+"/"+diffstr;
      }
      zeitLabel->setText(text);
    }
    
    void setDiff(int sec)
    {
      secDiff=sec;
      repaintZeitFeld();
    }

    void dateWarning(bool on, QDate datum=QDate::currentDate())
    {
      if (on) {
        datumsWarnung->setText("Warnung: Es wird der "+datum.toString("dd.MM.yyyy")+" editiert.");
        datumsWarnung->setPaletteForegroundColor(darkRed);
      }
      else
      {
        datumsWarnung->setText("");
      }


    }

  private:
    QLabel* zeitLabel;
    QLabel* datumsWarnung;
    int secDiff;
    int sekunden;
};

#endif


