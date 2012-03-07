#ifndef STATUSBAR_H
#define  STATUSBAR_H

#include <QWidget>
#include <QString>
#include <QStatusBar>
#include <QLabel>
#include <QDateTime>
#include "timecounter.h"

/** Die Statusbar des Hauptfensters */
class StatusBar:public QStatusBar
{
  Q_OBJECT

  public:
    StatusBar(QWidget * parent = 0):QStatusBar(parent)
    {
      zeitLabel=new QLabel(tr("Overall time: ") + "0",this);
      addPermanentWidget(zeitLabel);
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
      zeitLabel->setText(tr("Overall time: ")+text);
    }

    void setDiff(int sec)
    {
      secDiff=sec;
      repaintZeitFeld();
    }

    void dateWarning(bool on, QDate datum=QDate::currentDate())
    {
      if (on) {
        datumsWarnung->setText(tr("Warning: Non-current date ") +
          datum.toString(Qt::SystemLocaleShortDate) +
          tr(" is being edited."));
        datumsWarnung->setStyleSheet("color:#800000;");

        //datumsWarnung->setPaletteForegroundColor(Qt::darkRed);
      }
      else
      {
        datumsWarnung->setText("");
      }


    }

    void appendWarning(bool on, QString str)
    {
      if( on )
      {
        QString labelTxt = datumsWarnung->text();
        datumsWarnung->setText(labelTxt + str);
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


