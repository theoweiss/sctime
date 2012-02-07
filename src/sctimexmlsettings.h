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

#include <QDateTime>
#include <QPoint>
#include <QSize>
#include <QColor>
#include <vector>
#include <QString>

#define MAX_WORKTIME_DEFAULT 10*60*60 // Warn if more time is spent


class AbteilungsListe;
class QTextStream;

class SCTimeXMLSettings
{
public:

    SCTimeXMLSettings():backupSettingsXml(true)
    {
      defaultbackends = "QPSQL QODBC command file";
      timeInc = 5*60;
      fastTimeInc = 60*30;
      zeitKommando = "zeit";
      mainwindowPosition = QPoint(0,0);
      mainwindowSize = QSize(700,400);
      alwaysSaveEintrag = false;
      m_powerUserView = false;
      m_singleClickActivation = false;
      m_maxWorkingTime=MAX_WORKTIME_DEFAULT;
      m_persoenlicheKontensumme=false;
      defaultcommentfiles.clear();
      defaultcommentfiles.push_back("defaultcomments.xml");
      columnwidth.clear();
      unterKontoWindowPosition = QPoint(0,0);
      unterKontoWindowSize = QSize(10,10);
      m_showTypeColumn=false;
      m_useCustomFont=false;
      m_dragNDrop=true;
      m_customFont="helvetica";
      m_customFontSize=10;
      backends = defaultbackends;
    }

    void writeSettings(AbteilungsListe* abtList);

    void readSettings();

    void readSettings(AbteilungsListe* abtList);

    void writeShellSkript(AbteilungsListe* abtList);

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

    int timeIncrement() { return timeInc; }

    int fastTimeIncrement() { return fastTimeInc; }

    void setAlwaysSaveEntry(bool newVal)
    {
        alwaysSaveEintrag = newVal;
    }

    bool alwaysSaveEntry()
    {
        return alwaysSaveEintrag;
    }

    void setPersoenlicheKontensumme(bool newVal)
    {
        m_persoenlicheKontensumme=newVal;
    }

    bool persoenlicheKontensumme()
    {
        return m_persoenlicheKontensumme;
    }

    void setPowerUserView(bool newVal)
    {
       m_powerUserView = newVal;
    }

    bool powerUserView()
    {
      return m_powerUserView;
    }

    void setSingleClickActivation(bool activate)
    {
        m_singleClickActivation=activate;
    }

    bool singleClickActivation()
    {
        return m_singleClickActivation;
    }

    int maxWorkingTime()
    {
       return m_maxWorkingTime;
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

    QString codecString(QTextStream& stream);

    void setZeitKontenKommando(const QString& command)
    {
        m_zeitKontenKommando=command;
    }
    QString backends;

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
    bool m_powerUserView;
    bool m_singleClickActivation;
    bool m_dragNDrop;
    bool m_useCustomFont;
    bool m_showTypeColumn;
    bool m_persoenlicheKontensumme;
    int m_maxWorkingTime;
    QString m_customFont;
    int m_customFontSize;
    bool backupSettingsXml; // nur beim ersten Speichern ein  Backup von settings.xml erstellen
    QPoint unterKontoWindowPosition;
    QSize unterKontoWindowSize;
    QString defaultbackends;
};


#endif
