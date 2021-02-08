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
    enum DefCommentDisplayModeEnum{DM_BOLD,DM_NOTUSEDBOLD,DM_NOTBOLD};

    SCTimeXMLSettings():backupSettingsXml(true)
    {
      defaultbackends = "QPSQL QODBC command json file";
      defaultdatabaseserver = "zeitdabaserv";
      defaultdatabase = "zeit";
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
      m_lastRecordedTimestamp=QDateTime();
      defaultcommentfiles.clear();
      defaultcommentfiles.push_back("defaultcomments.xml");
      columnwidth.clear();
      unterKontoWindowPosition = QPoint(0,0);
      unterKontoWindowSize = QSize(10,10);
      m_showTypeColumn=false;
      m_showPSPColumn=false;
      m_useCustomFont=false;
      m_useDefaultCommentIfUnique=false;
      m_dragNDrop=true;
      m_customFont="helvetica";
      m_customFontSize=10;
      m_showSpecialRemunSelector=false;
      backends = defaultbackends;
      databaseserver = defaultdatabaseserver;
      database = defaultdatabase;
      m_defCommentDisplayMode = DM_BOLD;
      // databaseuser - empty means "use system username"
      // databasepassword - empty means "try to read from file"

      m_overtimeRegulatedSR = "regulated_overtime";
      m_overtimeOtherSR = "other_overtime";
      m_publicHolidaySR = "public_holiday";
      m_nightSR = "night";
      m_nightModeBegin = QTime();
      m_nightModeAdditionalDialogTimes=QList<QTime>();
      m_nightModeEnd=QTime();

      m_nightModeActive = false;
      m_overtimeRegulatedModeActive = false;
      m_overtimeOtherModeActive = false;
      m_publicHolidayModeActive = false;

      m_lastSave = QDateTime();

#ifdef ATOS_ETV_2018
      m_overtimeRegulatedSR = "sc_angeordnete_regulierte_mehrarbeit";
      m_overtimeOtherSR = "sc_angeordnete_sonstige_mehrarbeit";
      m_publicHolidaySR = "sc_feiertagsarbeit";
      m_nightSR = "sc_nachtarbeit";
      m_nightModeBegin=QTime(20,00);
      m_nightModeAdditionalDialogTimes.append(QTime(22,00));
      m_nightModeEnd=QTime(6,00);
#endif
     
    }

    bool writeSettings(AbteilungsListe* abtList);

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
    
    bool showPSPColumn()
    {
      return m_showPSPColumn;
    }

    void setShowPSPColumn(bool showPSPColumn)
    {
      m_showPSPColumn=showPSPColumn;
    }
    
    bool useDefaultCommentIfUnique()
    {
      return m_useDefaultCommentIfUnique;
    }

    void setUseDefaultCommentIfUnique(bool useDefaultCommentIfUnique)
    {
      m_useDefaultCommentIfUnique=useDefaultCommentIfUnique;
    }
    
    DefCommentDisplayModeEnum defCommentDisplayMode()
    {
        return m_defCommentDisplayMode;
    }

    void setDefCommentDisplayMode(DefCommentDisplayModeEnum defCommentDisplayMode)
    {
        m_defCommentDisplayMode=defCommentDisplayMode;
    }

    bool dragNDrop()
    {
        return m_dragNDrop;
    }

    void setDragNDrop(bool on)
    {
        m_dragNDrop=on;
    }
    
     bool showSpecialRemunSelector()
    {
        return m_showSpecialRemunSelector;
    }

    void setShowSpecialRemunSelector(bool on)
    {
        m_showSpecialRemunSelector=on;
    }

    QString zeitKontenKommando()
    {
        return m_zeitKontenKommando;
    }

    const char* charmap();

    void setZeitKontenKommando(const QString& command)
    {
        m_zeitKontenKommando=command;
    }

    /** returns true, if the night mode is active */
    bool nightModeActive()
    {
        return m_nightModeActive;
    }

    /** sets the night mode active/inactive) */
    void setNightModeActive(bool active)
    {
        m_nightModeActive=active;
    }

    /** returns true, if the regulated overtime mode is active */
    bool overtimeRegulatedModeActive()
    {
        return m_overtimeRegulatedModeActive;
    }

    /** sets the regulated overtime mode active/inactive) */
    void setOvertimeRegulatedModeActive(bool active)
    {
        m_overtimeRegulatedModeActive=active;
    }

    /** returns true, if the other overtime mode is active */
    bool overtimeOtherModeActive()
    {
        return m_overtimeOtherModeActive;
    }

    /** sets the other overtime mode active/inactive) */
    void setOvertimeOtherModeActive(bool active)
    {
        m_overtimeOtherModeActive=active;
    }

    /** returns true, if the public holiday mode is active */
    bool publicHolidayModeActive()
    {
        return m_publicHolidayModeActive;
    }

    /** sets the public holiday mode active/inactive) */
    void setPublicHolidayModeActive(bool active)
    {
        m_publicHolidayModeActive=active;
    }

    /* returns the global identification string for regulated overtime special remunerations */
    QString overtimeRegulatedSR() {
        return m_overtimeRegulatedSR;
    }

    /* returns the global identification string for other overtime special remunerations */
    QString overtimeOtherSR() {
        return m_overtimeOtherSR;
    }

    /* returns the global identification string for other overtime special remunerations */
    QString publicHolidaySR() {
        return m_publicHolidaySR;
    }

    /* returns the global identification string for night time special remunerations */
    QString nightSR() {
        return m_nightSR;
    }

    /* returns the time when night mode should start */
    QTime nightModeBegin() {
        return m_nightModeBegin;
    }

    /* returns the last time stamp sctime has registered (possibly in a previous run) */
    QDateTime lastRecordedTimestamp() {
        return m_lastRecordedTimestamp;
    }

    /* sets the last time stamp sctime has registered */
    void setLastRecordedTimestamp(const QDateTime& timestamp) {
        m_lastRecordedTimestamp = timestamp;
    }
    
    /* additional times when the night mode dialog should be shown */
    QList<QTime> nightModeAdditionalDialogTimes()
    {
        return m_nightModeAdditionalDialogTimes;
    }

    /* returns the time when night mode should end */
    QTime nightModeEnd() {
        return m_nightModeEnd;
    };

    /* the format in which timestamps are written into the configuration file */
    QString timestampFormat() {
        return "yyyy-MM-dd HH:mm:ss";
    }


    QString backends;

    // database backend
    QString databaseserver;
    QString database;
    QString databaseuser;
    QString databasepassword;

  private:

    bool writeSettings(bool global, AbteilungsListe* abtList);

    void readSettings(bool global, AbteilungsListe* abtList);
    
    int compVersion(const QString& version1, const QString& version2);

    QString color2str(const QColor& color);

    QColor str2color(const QString& str);

    QString zeitKommando;
    QString m_zeitKontenKommando;

    int timeInc,fastTimeInc;

    QPoint mainwindowPosition;
    QSize mainwindowSize;

    std::vector<QString> defaultcommentfiles;
    std::vector<int> columnwidth;

    QDateTime m_lastRecordedTimestamp;

    bool alwaysSaveEintrag;
    bool m_powerUserView;
    bool m_singleClickActivation;
    bool m_dragNDrop;
    bool m_useCustomFont;
    bool m_showTypeColumn;
    bool m_showPSPColumn;
    bool m_useDefaultCommentIfUnique;
    bool m_persoenlicheKontensumme;
    bool m_showSpecialRemunSelector;
    int m_maxWorkingTime;
    QString m_customFont;
    int m_customFontSize;
    bool backupSettingsXml; // nur beim ersten Speichern ein  Backup von settings.xml erstellen
    DefCommentDisplayModeEnum m_defCommentDisplayMode;
    QPoint unterKontoWindowPosition;
    QSize unterKontoWindowSize;
    QString defaultbackends;

    QString m_overtimeRegulatedSR;
    QString m_overtimeOtherSR;
    QString m_publicHolidaySR;
    QString m_nightSR;
    QTime m_nightModeBegin;
    QList<QTime> m_nightModeAdditionalDialogTimes;
    QTime m_nightModeEnd;

    QDateTime m_lastSave;

    bool m_nightModeActive;
    bool m_overtimeRegulatedModeActive;
    bool m_overtimeOtherModeActive;
    bool m_publicHolidayModeActive;

    // database backend
    QString defaultdatabaseserver;
    QString defaultdatabase;
};


#endif
