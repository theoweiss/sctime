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

#ifndef SCTIMESETTINGS_H
#define  SCTIMESETTINGS_H

#include "abteilungsliste.h"
#include "qdatetime.h"
#include "qpoint.h"
#include "qsize.h"
#include <iostream>
#define READ_OBSOLETE_SETTINGS // Sollen veraltete Ini-Dateien eingelesen werden?

#ifdef WIN32  // unter Windows vorlaeufig keine obsoleten Settings einlesen, wg Kompilier-Problem
#undef READ_OBSOLETE_SETTINGS
#endif

#ifdef ZAURUS // Zaurus mag auch keine Obsoleten Settings
#undef READ_OBSOLETE_SETTINGS
#endif

#ifdef READ_OBSOLETE_SETTINGS
#ifdef WIN32
// Auch unter Windows in Config-Dateien schreiben...
#include "qfilesettings.h"
#define QSettings QFileSettings
#else
#include "qsettings.h"
#endif
#endif

class SCTimeXMLSettings
{
  public:

    SCTimeXMLSettings(AbteilungsListe* abtlist)
    {
      abtList = abtlist;
      timeInc = 5*60;
      fastTimeInc = 60*30; 
      zeitKommando = "zeit";
      mainwindowPosition = QPoint(0,0);
      mainwindowSize = QSize(700,400);
      alwaysSaveEintrag = false;
      _powerUserView = false;
      unterKontoWindowPosition = QPoint(0,0);
      unterKontoWindowSize = QSize(0,0);
    }

    void writeSettings();

    void readSettings();

    void writeShellSkript();

    void setTimeIncrement(int sekunden) { timeInc=sekunden; };
    
    void setFastTimeIncrement(int sekunden) { fastTimeInc=sekunden; };
    
    void setMainWindowGeometry(const QPoint& pos, const QSize& size)
    {
        mainwindowPosition=pos;
        mainwindowSize=size;
    };
    void getMainWindowGeometry(QPoint& pos, QSize& size)
    {
        pos  = mainwindowPosition;
        size = mainwindowSize;
    };
    
    void setUnterKontoWindowGeometry(const QPoint& pos, const QSize& size)
    {
        unterKontoWindowPosition = pos;
        unterKontoWindowSize = size;
    };
    void getUnterKontoWindowGeometry(QPoint& pos, QSize& size)
    {
        pos = unterKontoWindowPosition;
        size = unterKontoWindowSize;
    };

    int timeIncrement() { return timeInc; };

    int fastTimeIncrement() { return fastTimeInc; };

    void setAlwaysSaveEntry(bool newVal)
    {
        alwaysSaveEintrag = newVal;
    }

    bool alwaysSaveEntry()
    {
        return alwaysSaveEintrag;
    }

    void setPowerUserView(bool newVal)
    {
       _powerUserView = newVal;
    }

    bool powerUserView()
    {
      return _powerUserView;
    }
    


  private:

    void writeSettings(bool global);
    
    void readSettings(bool global);
     
#ifdef READ_OBSOLETE_SETTINGS

    void readObsSetting(QSettings& settings, const QString& id, const QString& datumsID,bool global);

    void readObsSettings(bool global);

#endif

    AbteilungsListe* abtList;

    QString zeitKommando;

    int timeInc,fastTimeInc;
    
    QPoint mainwindowPosition;
    QSize mainwindowSize;

    bool alwaysSaveEintrag;
    bool _powerUserView;
    
    QPoint unterKontoWindowPosition;
    QSize unterKontoWindowSize;
};


#endif
