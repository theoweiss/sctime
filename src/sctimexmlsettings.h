/*
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

#define MAX_WORKTIME_DEFAULT 10*60*60 // Warn if more time is spent

#include "abteilungsliste.h"
#include "qdatetime.h"
#include "qpoint.h"
#include "qsize.h"
#include <iostream>
#include <vector>

class SCTimeXMLSettings
{
  public:

    SCTimeXMLSettings()
    {
      timeInc = 5*60;
      fastTimeInc = 60*30;
      zeitKommando = "zeit";
      mainwindowPosition = QPoint(0,0);
      mainwindowSize = QSize(700,400);
      alwaysSaveEintrag = false;
      _powerUserView = false;
      _singleClickActivation = false;
      _maxWorkingTime=MAX_WORKTIME_DEFAULT;
      defaultcommentfiles.clear();
      columnwidth.clear();
      unterKontoWindowPosition = QPoint(0,0);
      unterKontoWindowSize = QSize(0,0);
      m_showTypeColumn=false;
      m_useCustomFont=false;
      m_dragNDrop=true;
      m_customFont="helvetica";
      m_customFontSize=10;
    }

    void writeSettings(AbteilungsListe* abtList);

    void readSettings();

    void readSettings(AbteilungsListe* abtList);

    void writeShellSkript(AbteilungsListe* abtList);

    void setTimeIncrement(int sekunden) { timeInc=sekunden; };

    void setFastTimeIncrement(int sekunden) { fastTimeInc=sekunden; };

    bool moveToCheckedIn(AbteilungsListe* abtList);

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

    void setSingleClickActivation(bool activate)
    {
        _singleClickActivation=activate;
    }

    bool singleClickActivation()
    {
        return _singleClickActivation;
    }

    int maxWorkingTime()
    {
       return _maxWorkingTime;
    }

    void getDefaultCommentFiles(std::vector<QString>& list)
    {
       list = defaultcommentfiles;
    }

    void getColumnWidthList(std::vector<int>& list)
    {
        list = columnwidth;
    }

    void setColumnWidthList(const std::vector<int>& list)
    {
        columnwidth = list;
    }

    bool useCustomFont()
    {
    	return m_useCustomFont;
    }
    
    void setUseCustomFont(bool useCustomFont)
    {
    	m_useCustomFont=useCustomFont;
    }
    
    QString customFont()
    {
    	return m_customFont;
    }
    
    void setCustomFont(QString customFont)
    {
        m_customFont=customFont;
    }
    
    int customFontSize()
    {
    	return m_customFontSize;
    }
    
    void setCustomFontSize(int customFontSize)
    {
        m_customFontSize=customFontSize;
    }

    bool showTypeColumn()
    {
    	return m_showTypeColumn;
    }
    
    void setShowTypeColumn(bool showTypeColumn)
    {
    	m_showTypeColumn=showTypeColumn;
    }

    bool dragNDrop()
    {
        return m_dragNDrop;
    }

    void setDragNDrop(bool on)
    {
        m_dragNDrop=on;
    }

    QString zeitKontenKommando()
    {
        return m_zeitKontenKommando;
    }

    void setZeitKontenKommando(const QString& command)
    {
        m_zeitKontenKommando=command;
    }

  private:

    void writeSettings(bool global, AbteilungsListe* abtList);

    void readSettings(bool global, AbteilungsListe* abtList);

    QString color2str(const QColor& color);

    QColor str2color(const QString& str);

    QString zeitKommando;
    QString m_zeitKontenKommando;

    int timeInc,fastTimeInc;

    QPoint mainwindowPosition;
    QSize mainwindowSize;

    std::vector<QString> defaultcommentfiles;
    std::vector<int> columnwidth;

    bool alwaysSaveEintrag;
    bool _powerUserView;
    bool _singleClickActivation;
    bool m_dragNDrop;
    bool m_useCustomFont;
    bool m_showTypeColumn;
    int _maxWorkingTime;
    QString m_customFont;
    int m_customFontSize;

    QPoint unterKontoWindowPosition;
    QSize unterKontoWindowSize;
};


#endif
