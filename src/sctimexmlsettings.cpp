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

#include "sctimexmlsettings.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QRect>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#ifndef WIN32
#include <langinfo.h>
#include <errno.h>
#else
#include <windows.h>
#endif
#include "qdom.h"
#include "abteilungsliste.h"
#include "globals.h"
#include "timecounter.h"
#define WIN_CODEC "utf8"
#include "globals.h"
#include "util.h"

/** Schreibt die Eintraege in ein Shellskript */
void SCTimeXMLSettings::writeShellSkript(AbteilungsListe* abtList)
{
  if (abtList->checkInState()) {
      trace(QObject::tr("Shell script not written because it has already been checked in."));
      return;
  }
  QString filename="zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".sh";
  QFile shellFile(configDir.filePath(filename));

  if (!shellFile.open(QIODevice::WriteOnly)) {
      QMessageBox::warning(NULL, QObject::tr("sctime: writing shell script"), shellFile.fileName() + ": " + shellFile.errorString());
      return;
  }
  QTextStream stream( & shellFile);
  stream.setCodec(charmap());
  int sek, abzurSek;
  abtList->getGesamtZeit(sek,abzurSek);
  QRegExp apostrophExp=QRegExp("'");

  TimeCounter tc(sek), tcAbzur(abzurSek);
  stream<<
           "#!/bin/sh\n"
           "# -*- coding: "<<charmap()<<" -*-\n\n"
           "set -e\n"
           "trap '[ $? -gt 0 ] && echo \"ABBRUCH in $PWD/$0 bei Zeile $LINENO - nicht alle Buchungen sind uebernommen\" >&2 && exit 1' 0"
           "\n\n"
           "# Zeit Aufrufe von sctime "<< qApp->applicationVersion() <<" generiert \n"
           "# Gesamtzeit: "<<tc.toString()<<"/"<<tcAbzur.toString()<<"\n"
           "ZEIT_ENCODING="<<charmap()<<"\n"
           "export ZEIT_ENCODING\n";

  AbteilungsListe::iterator abtPos;

  for (abtPos=abtList->begin(); abtPos!=abtList->end(); ++abtPos) {
    QString abt=abtPos->first;
    KontoListe* kontoliste=&(abtPos->second);
    bool firstInBereich=true;
    for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
      UnterKontoListe* unterkontoliste=&(kontPos->second);
      for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
        EintragsListe* eintragsliste=&(ukontPos->second);
        QStringList bereitschaften= eintragsliste->getBereitschaft();
        if ((bereitschaften.size()&&firstInBereich)) {
          stream<<"\n# Bereich: "<<abt<<"\n";
          firstInBereich=false;
        }
        for (int i=0; i<bereitschaften.size(); i++)
        {
           stream<<"zeitbereit "<<abtList->getDatum().toString("dd.MM.yyyy")<<" "<<
                 kontPos->first<<" "<<ukontPos->first<<"\t"<<bereitschaften.at(i)<<"\n";
        }
        for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {
          if ((etPos->second.sekunden!=0)||(etPos->second.sekundenAbzur!=0)) {
             if (firstInBereich) {
               stream<<"\n# Bereich: "<<abt<<"\n";
               firstInBereich=false;
             }
             QString kommentar=etPos->second.kommentar.replace(apostrophExp,""); // Apostrophe nicht in Skript speichern!
             
             QString srstr="";
             QSet<QString> specialRemunSet = etPos->second.getAchievedSpecialRemunSet();
             foreach (QString specialremun, specialRemunSet) {
               if (srstr.isEmpty()) {
                  srstr=specialremun;
               } else {
                  srstr+=","+specialremun;
               }
             }
             if (!srstr.isEmpty()) {
               srstr="--sonderzeiten='"+srstr+"' ";
             }
             stream<<zeitKommando<<" "<<srstr<<
                     abtList->getDatum().toString("dd.MM.yyyy")<<" "<<
                     kontPos->first<<" "<<ukontPos->first<<"\t"<<
                     roundTo(1.0/3600*etPos->second.sekunden,0.01)<<"/"<<
                     roundTo(1.0/3600*etPos->second.sekundenAbzur,0.01)<<
                     " \'"<<kommentar.simplified()<<"\'\n";
          }
        }
      }
    }
  }
  stream<<endl;
  shellFile.close();
}

// returns the encoding that the user has chosen by his locale settings
const char* SCTimeXMLSettings::charmap() {
#if defined(WIN32) || defined(Q_OS_MAC)
    return "UTF-8";
#else
    return nl_langinfo(CODESET); // has same result as the command "locale charmap"
#endif
}

void SCTimeXMLSettings::readSettings(AbteilungsListe* abtList)
{
  abtList->clearKonten();
  // Erst globale Einstellungen lesen
  readSettings(true, abtList);
  // Dann die Tagesspezifischen
  readSettings(false, abtList);
}

void SCTimeXMLSettings::readSettings()
{
  // Nur globale Einstellungen lesen
  readSettings(true, NULL);
}

int SCTimeXMLSettings::compVersion(const QString& version1, const QString& version2)
{
  QStringList v1list=version1.split(".");
  QStringList v2list=version2.split(".");
  for (int i=0; i<v1list.size(); i++) {
     if (i>=v2list.size())
	return 1;
     int v1=v1list[i].toInt();
     int v2=v2list[i].toInt();
     if (v1>v2) {
	return 1;
     } else
     if (v1<v2) {
	return -1;
     } 
  }
  return 0; 
}

/**
 * Liest alle Einstellungen.
 */

void SCTimeXMLSettings::readSettings(bool global, AbteilungsListe* abtList)
{
  QDomDocument doc("settings");
  QString filename;
  if (global)
    filename="settings.xml";
  else {
    filename = "zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".xml";
    QFileInfo qf(configDir.filePath(abtList->getDatum().toString("yyyy")), filename);
    if (abtList) {
        if (qf.exists()) {
          filename = abtList->getDatum().toString("yyyy")+ "/" + filename;
          abtList->setCheckInState(true);
        }
        else
          abtList->setCheckInState(false);
    }
  }

  QFile f(configDir.filePath(filename));
  if ( !f.open( QIODevice::ReadOnly ) ) {
    logError(f.fileName() + ": " + f.errorString());
    if (global || f.exists()) {
      // keine Fehlerausgabe, wenn "zeit-HEUTE.xml" fehlt
      QMessageBox::warning(NULL, QObject::tr("sctime: opening configuration file"),
                           QObject::tr("%1 : %2").arg(f.fileName(), f.errorString()));
      if (global) backupSettingsXml = false;
    }
    return;
  }
  QString errMsg;
  int errLine, errCol;
  if (!doc.setContent(&f, &errMsg, &errLine, &errCol)) {
    QMessageBox::critical(NULL, QObject::tr("sctime: reading configuration file"),
                          QObject::tr("error in %1, line %2, column %3: %4.").arg(errMsg).arg(errLine).arg(errCol).arg(errMsg));
    if (global) backupSettingsXml = false;
    return;
  }
  f.close();
  QDomElement aktiveskontotag;
  QDomElement docElem = doc.documentElement();
  QString lastVersion=docElem.attribute("version");
  if (global) {
    defaultcommentfiles.clear();
    columnwidth.clear();
    setUseCustomFont(false);
  }

  for(QDomNode node1 = docElem.firstChild(); !node1.isNull(); node1 = node1.nextSibling()) {
    QDomElement elem1 = node1.toElement();
    if( !elem1.isNull() ) {
      if ((elem1.tagName()=="abteilung")&&(abtList)) {
        QString abteilungstr=elem1.attribute("name");
        if (abteilungstr.isNull()) continue;
        abtList->insertAbteilung(abteilungstr);
        if (elem1.attribute("open")=="yes")
          abtList->setAbteilungFlags(abteilungstr,IS_CLOSED,FLAG_MODE_NAND);
        if (elem1.attribute("open")=="no")
          abtList->setAbteilungFlags(abteilungstr,IS_CLOSED,FLAG_MODE_OR);

        if (!elem1.attribute("color").isEmpty()) {
           abtList->setBgColor(str2color(elem1.attribute("color")),abteilungstr);
        }
        for( QDomNode node2 = elem1.firstChild(); !node2.isNull(); node2 = node2.nextSibling() ) {
          QDomElement elem2 = node2.toElement();
          if( !elem2.isNull() ) {
            if (elem2.tagName()=="konto") {
              QString kontostr=elem2.attribute("name");
              if (kontostr.isNull()) continue;
              abtList->insertKonto(abteilungstr,kontostr);
              if (elem2.attribute("open")=="yes")
                abtList->setKontoFlags(abteilungstr,kontostr,IS_CLOSED,FLAG_MODE_NAND);
              if (elem2.attribute("open")=="no")
                abtList->setKontoFlags(abteilungstr,kontostr,IS_CLOSED,FLAG_MODE_OR);
              abtList->moveKontoPersoenlich(abteilungstr,kontostr,(elem2.attribute("persoenlich")=="yes"));
              if (!elem2.attribute("color").isEmpty()) {
                   abtList->setBgColor(str2color(elem2.attribute("color")),abteilungstr,kontostr);
              }

              for( QDomNode node3 = elem2.firstChild(); !node3.isNull(); node3 = node3.nextSibling() ) {
                QDomElement elem3 = node3.toElement();
                if( !elem3.isNull() ) {
                  if (elem3.tagName()=="unterkonto") {
                    QString unterkontostr=elem3.attribute("name");
                    if (unterkontostr.isNull()) continue;
                    abtList->insertUnterKonto(abteilungstr,kontostr,unterkontostr);
                    if (elem3.attribute("open")=="yes")
                      abtList->setUnterKontoFlags(abteilungstr,kontostr,unterkontostr,
                                                  IS_CLOSED,FLAG_MODE_NAND);
                    if (elem3.attribute("open")=="no")
                      abtList->setUnterKontoFlags(abteilungstr,kontostr,unterkontostr,
                                                  IS_CLOSED,FLAG_MODE_OR);
                    if ((elem3.attribute("persoenlich")=="yes")||(elem3.attribute("persoenlich")=="no"))
                      abtList->moveUnterKontoPersoenlich(abteilungstr,kontostr,unterkontostr,(elem3.attribute("persoenlich")=="yes"));

                    if (!elem3.attribute("color").isEmpty()) {
                      abtList->setBgColor(str2color(elem3.attribute("color")),abteilungstr,kontostr,unterkontostr);
                    }

                    bool ukontPers=((abtList->getUnterKontoFlags(abteilungstr,kontostr,unterkontostr))
                                    &UK_PERSOENLICH);
                    bool dummydeleted=false;

                    for( QDomNode node4 = elem3.firstChild(); !node4.isNull(); node4 = node4.nextSibling() ) {
                      QDomElement elem4 = node4.toElement();
                      if( !elem4.isNull() ) {
                        if (elem4.tagName()=="bereitschaft") {
                          QString bereitschaft=elem4.attribute("type");
                          if (bereitschaft.isNull()) continue;
                          UnterKontoListe::iterator itUk;
                          UnterKontoListe* ukl;
                          if (abtList->findUnterKonto(itUk,ukl,abteilungstr,kontostr,unterkontostr)) {
                            QStringList bereitschaften;
                            bereitschaften=itUk->second.getBereitschaft();
                            bereitschaften.append(bereitschaft);
                            itUk->second.setBereitschaft(bereitschaften);
                          }
                        }
                        if (elem4.tagName()=="eintrag") {
                          QString eintragstr=elem4.attribute("nummer");
                          if (eintragstr.isNull()) continue;
                          int idx=eintragstr.toInt();

                          EintragsListe::iterator eti;
                          EintragsListe* etl;

                          if (!abtList->findEintrag(eti,etl,abteilungstr,kontostr,unterkontostr,idx)) {
                            if ((idx!=0)&&(abtList->findEintrag(eti,etl,abteilungstr,kontostr,unterkontostr,0))&&((eti->second).isEmpty()))
                              {
                                abtList->deleteEintrag(abteilungstr,kontostr,unterkontostr,0); // Leere Dummy Eintraege mit Index 0 loswerden.
                                dummydeleted=true;
                              }
                            abtList->insertEintrag(abteilungstr,kontostr,unterkontostr,idx);
                          }
                          QSet<QString> specialRemunSet;
                          for( QDomNode node5 = elem4.firstChild(); !node5.isNull(); node5 = node5.nextSibling() ) {
                              QDomElement elem5 = node5.toElement();
                              if (elem5.tagName()=="specialremun")
                              {
                                specialRemunSet.insert(elem5.attribute("name"));
                              }
                          }
                          if (!specialRemunSet.isEmpty()) {
                              abtList->findEintrag(eti,etl,abteilungstr,kontostr,unterkontostr,idx);
                              eti->second.setAchievedSpecialRemunSet(specialRemunSet);
                          }
                          if (elem4.attribute("persoenlich")=="yes")
                            abtList->setEintragFlags(abteilungstr,kontostr,unterkontostr,idx,
                                                     UK_PERSOENLICH,FLAG_MODE_OR);
                          else
                            if (elem4.attribute("persoenlich")=="no")
                              abtList->setEintragFlags(abteilungstr,kontostr,unterkontostr,idx,
                                                       UK_PERSOENLICH,FLAG_MODE_NAND);
                            else
                              if (ukontPers) abtList->setEintragFlags(abteilungstr,kontostr,unterkontostr,
                                                                      idx,UK_PERSOENLICH,FLAG_MODE_OR);
                              else abtList->setEintragFlags(abteilungstr,kontostr,unterkontostr,
                                                                      idx,UK_PERSOENLICH,FLAG_MODE_NAND);
                          if (!elem4.attribute("sekunden").isNull()) {
                            abtList->setSekunden(abteilungstr,kontostr,unterkontostr,idx,
                                                 elem4.attribute("sekunden").toInt(),true);
                          }
                          if (!elem4.attribute("abzurechnende_sekunden").isNull()) {
                            abtList->setSekundenAbzur(abteilungstr,kontostr,unterkontostr,idx,
                                                      elem4.attribute("abzurechnende_sekunden").toInt());
                          }
                          if (!elem4.attribute("kommentar").isNull()) {
                            abtList->setKommentar(abteilungstr,kontostr,unterkontostr,idx,
                                                  elem4.attribute("kommentar"));
                          }
                        }
                        
                      }
                    }
                    if ((!dummydeleted)&&(ukontPers)) {
                      EintragsListe::iterator eti;
                      EintragsListe* etl;
                      if (!abtList->findEintrag(eti,etl,abteilungstr,kontostr,unterkontostr,0)) {
                        if (etl->size()==0) { //bei Bedarf Dummy Erzeugen
                          abtList->insertEintrag(abteilungstr,kontostr,unterkontostr,0);
                        }
                      }
                      abtList->setEintragFlags(abteilungstr,kontostr,unterkontostr,
                                               etl->begin()->first,UK_PERSOENLICH,FLAG_MODE_OR);
                    }
                    // Eintraege deaktivieren, falls eine Verbindung zur Datenbank aufgebaut werden konnte,
                    // sie dort aber nicht erscheinen.
                    if (((abtList->getUnterKontoFlags(abteilungstr,kontostr,unterkontostr)&IS_IN_DATABASE)==0)
                        &&(abtList->kontoDatenInfoConnected()))
                    {
                       int sek,sekabzur;
                       abtList->getUnterKontoZeiten(abteilungstr,kontostr,unterkontostr,sek,sekabzur);
                       // Nur deaktivieren, falls nicht bereits auf dieses Konto gebucht wurde
                       if ((sek==0)&&(sekabzur==0)) {
                         abtList->setUnterKontoFlags(abteilungstr,kontostr,unterkontostr,IS_DISABLED,FLAG_MODE_OR);
                       }
                       else
                         abtList->setUnterKontoFlags(abteilungstr,kontostr,unterkontostr,IS_DISABLED,FLAG_MODE_NAND);
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (elem1.tagName()=="general") {
        for( QDomNode node2 = elem1.firstChild(); !node2.isNull(); node2 = node2.nextSibling() ) {
          QDomElement elem2 = node2.toElement();
          if( !elem2.isNull() ) {
            if (elem2.tagName()=="timeincrement") {
              QString secondsstr=elem2.attribute("seconds");
              if (secondsstr.isNull()) continue;
              timeInc=secondsstr.toInt();
            }
            if (elem2.tagName()=="fasttimeincrement") {
              QString secondsstr=elem2.attribute("seconds");
              if (secondsstr.isNull()) continue;
              fastTimeInc=secondsstr.toInt();
            }
            if (elem2.tagName()=="zeitkommando") {
              QString kommandostr=elem2.firstChild().toCharacterData().data();
              if (kommandostr.isNull()) continue;
              zeitKommando=kommandostr;
            }
            if (elem2.tagName()=="zeitkontenkommando") {
              QString zeitkontenkommandostr=elem2.firstChild().toCharacterData().data();
              if (zeitkontenkommandostr.isNull()) continue;
              setZeitKontenKommando(zeitkontenkommandostr);
            }
            if (elem2.tagName()=="defcommentdisplay") {
              QString dmstring=elem2.attribute("mode");
              if (dmstring.isNull()) continue;
              DefCommentDisplayModeEnum dm=DM_BOLD;
              if (dmstring=="DefaultCommentsNotUsedBold") {
                dm=DM_NOTUSEDBOLD;
              } else
              if (dmstring=="NotBold") {
                dm=DM_NOTBOLD;
              }
              setDefCommentDisplayMode(dm);
            }
            if (elem2.tagName()=="dragndrop") {
              setDragNDrop(elem2.attribute("on")=="yes");
            }
            if (elem2.tagName()=="persoenliche_kontensumme") {
              setPersoenlicheKontensumme(elem2.attribute("on")=="yes");
            }
            if (elem2.tagName()=="max_working_time") {
              QString secondsstr=elem2.attribute("seconds");
              if (secondsstr.isNull()) continue;
              m_maxWorkingTime=secondsstr.toInt();
            }
            if (elem2.tagName()=="aktives_konto") {
              aktiveskontotag=elem2; // Aktives Konto merken und zum Schluss setzen, damit es vorher erzeugt wurde
            }
            if (elem2.tagName()=="windowposition") {
		bool ok;
		int x = QString(elem2.attribute("x")).toInt(&ok);
		int y = QString(elem2.attribute("y")).toInt(&ok);
		if (ok) {
		    QPoint pos(x, y);		    
		    QRect rootwinsize = QApplication::desktop()->screenGeometry();
		    if (rootwinsize.contains(pos))  // Position nicht setzen, wenn Fenster sonst ausserhalb
			mainwindowPosition = pos;
		}
            }
            if (elem2.tagName()=="windowsize") {
              QString xstr=elem2.attribute("width");
              if (xstr.isNull()) continue;
              QString ystr=elem2.attribute("height");
              if (ystr.isNull()) continue;
              mainwindowSize=QSize(xstr.toInt(),ystr.toInt());
            }
            if (elem2.tagName()=="saveeintrag") {
              alwaysSaveEintrag=(elem2.attribute("always")=="yes");
            }
            if (elem2.tagName()=="typecolumn") {
              m_showTypeColumn=(elem2.attribute("show")=="yes");
            }
            if (elem2.tagName()=="pspcolumn") {
              m_showPSPColumn=(elem2.attribute("show")=="yes");
            }
            if (elem2.tagName()=="specialremunselector") {
                setShowSpecialRemunSelector(elem2.attribute("show")=="yes");
            }
            if (elem2.tagName()=="usedefaultcommentifunique") {
              m_useDefaultCommentIfUnique=(elem2.attribute("on")=="yes");
            }
            if (elem2.tagName()=="poweruserview") {
              setPowerUserView((elem2.attribute("on")=="yes"));
            }
            if (elem2.tagName()=="overtimeregulatedmode") {
              setOvertimeRegulatedModeActive((elem2.attribute("on")=="yes"));
            }
            if (elem2.tagName()=="overtimeothermode") {
              setOvertimeOtherModeActive((elem2.attribute("on")=="yes"));
            }
            if (elem2.tagName()=="nightmode") {
              setNightModeActive((elem2.attribute("on")=="yes"));
            }
            if (elem2.tagName()=="publicholidaymode") {
              setPublicHolidayModeActive((elem2.attribute("on")=="yes"));
            }
            if (elem2.tagName()=="lastrecordedtimestamp") {
              QDateTime ts = QDateTime::fromString(elem2.attribute("value"), timestampFormat());
              setLastRecordedTimestamp(ts);
            }
            if (elem2.tagName()=="customfont") {
              setUseCustomFont(true);
              setCustomFont(elem2.attribute("family"));
              setCustomFontSize(elem2.attribute("size").toInt());
            }
            if (elem2.tagName()=="singleclickactivation") {
              setSingleClickActivation((elem2.attribute("on")=="yes"));
            }
            if (elem2.tagName()=="kontodlgwindowposition") {
              bool ok;
              int x = QString(elem2.attribute("x")).toInt(&ok);
              int y = QString(elem2.attribute("y")).toInt(&ok);
              if (ok) {
                  QPoint pos(x, y);
                  QRect rootwinsize = QApplication::desktop()->screenGeometry();
                  if (rootwinsize.contains(pos))  // Position nicht setzen, wenn Fenster sonst ausserhalb
                    unterKontoWindowPosition = pos;
              }
            }
            if (elem2.tagName()=="kontodlgwindowsize") {
              QString xstr=elem2.attribute("width");
              if (xstr.isNull()) continue;
              QString ystr=elem2.attribute("height");
              if (ystr.isNull()) continue;
              unterKontoWindowSize=QSize(xstr.toInt(),ystr.toInt());
            }
            if ((global) && (elem2.tagName()=="defaultcommentsfile")) {
                defaultcommentfiles.push_back(elem2.attribute("name","defaultcomments.xml"));
            }
            if ((global) && (elem2.tagName()=="column")) {
                columnwidth.push_back(elem2.attribute("width","50").toInt());
            }
            if (global && elem2.tagName() == "backends") {
              backends = elem2.attribute("names", defaultbackends);
	      if (compVersion("0.80.1", lastVersion)!=-1) // in case of upgrade add new backend
	      {
		  backends.replace("file", "json file");
	      }

              for( QDomNode node3 = elem2.firstChild(); !node3.isNull(); node3 = node3.nextSibling() ) {
                QDomElement elem3 = node3.toElement();
                if( !elem3.isNull() ) {
                  if ( elem3.tagName() == "database") {
                    databaseserver = elem3.attribute("server", defaultdatabaseserver);
                    database = elem3.attribute("name", defaultdatabase);
                    databaseuser = elem3.attribute("user");
                    databasepassword = elem3.attribute("password");
		  }
		}
	      }
	    }
          }
        }
      }
    }
  }

  if ((!aktiveskontotag.isNull())&&abtList) {
    QString abtstr=aktiveskontotag.attribute("abteilung");
    QString kostr=aktiveskontotag.attribute("konto");
    QString ukostr=aktiveskontotag.attribute("unterkonto");
    int idx=aktiveskontotag.attribute("index").toInt();
    abtList->setAsAktiv(abtstr,kostr,ukostr,idx);
  }
}

/** Schreibt saemtliche Einstellungen und Eintraege auf Platte */
void SCTimeXMLSettings::writeSettings(AbteilungsListe* abtList)
{
  // Globale Einstellungen
  writeSettings(true, abtList);
  // Einstellungen fuer den aktuellen Tag
  writeSettings(false, abtList);
}

/**
 * Schreibt die Einstellungen und Eintraege auf Platte. Wenn global==true,
 * werden globale Einstellungen fuer alle Tage gespeichert, sonst nur
 * fuer das aktuelle Datum.
 */
void SCTimeXMLSettings::writeSettings(bool global, AbteilungsListe* abtList)
{
  if ((abtList->checkInState())&&(!global)) {
      trace(QObject::tr("zeit-DAY.sh not written because it has already been checked in"));
      return;
  }
  #ifndef NO_XML
  QDomDocument doc("settings");

  QDomElement root = doc.createElement( "sctime" );
  // TODO: XML/HTML-Quoting
  root.setAttribute("version", qApp->applicationVersion());
  doc.appendChild( root );
  QDomElement generaltag = doc.createElement( "general" );

  if (global) {
    QDomElement timeinctag = doc.createElement( "timeincrement" );
    timeinctag.setAttribute("seconds",timeInc);
    generaltag.appendChild(timeinctag);

    QDomElement fasttimeinctag = doc.createElement( "fasttimeincrement" );
    fasttimeinctag.setAttribute("seconds",fastTimeInc);
    generaltag.appendChild(fasttimeinctag);

    QDomElement zeitkommandotag = doc.createElement( "zeitkommando" );
    zeitkommandotag.appendChild(doc.createTextNode(zeitKommando));
    generaltag.appendChild(zeitkommandotag);

    if (!zeitKontenKommando().isEmpty()) {
      QDomElement zeitkontenkommandotag = doc.createElement( "zeitkontenkommando" );
      zeitkontenkommandotag.appendChild(doc.createTextNode(zeitKontenKommando()));
      generaltag.appendChild(zeitkontenkommandotag);
    }

    if (MAX_WORKTIME_DEFAULT!=m_maxWorkingTime) {
        QDomElement maxworktag = doc.createElement( "max_working_time" );
        maxworktag.setAttribute("seconds",m_maxWorkingTime);
        generaltag.appendChild(maxworktag);
    }

    QDomElement mainwindowpositiontag = doc.createElement( "windowposition" );
    mainwindowpositiontag.setAttribute("x",mainwindowPosition.x());
    mainwindowpositiontag.setAttribute("y",mainwindowPosition.y());
    generaltag.appendChild(mainwindowpositiontag);

    QDomElement mainwindowsizetag = doc.createElement("windowsize");
    mainwindowsizetag.setAttribute("width",mainwindowSize.width());
    mainwindowsizetag.setAttribute("height",mainwindowSize.height());
    generaltag.appendChild(mainwindowsizetag);

    for (unsigned int i=0; i<columnwidth.size() - 1; i++) { // do not save the width of the comment column
        QDomElement columnwidthtag = doc.createElement("column");
        columnwidthtag.setAttribute("width",columnwidth[i]);
        generaltag.appendChild(columnwidthtag);
    }

    QDomElement defcommentdm = doc.createElement("defcommentdisplay");
    QString dm;
    switch(defCommentDisplayMode()) {
      case DM_NOTUSEDBOLD: dm="DefaultCommentsNotUsedBold"; break;
      case DM_NOTBOLD: dm="NotBold"; break;
      default: dm ="DefaultCommentsBold"; break;
    }
    defcommentdm.setAttribute("mode",dm);
    generaltag.appendChild(defcommentdm);

    QDomElement saveeintragtag = doc.createElement("saveeintrag");
    QString always="no";
    if (alwaysSaveEintrag) always="yes";
    saveeintragtag.setAttribute("always",always);
    generaltag.appendChild(saveeintragtag);

    QDomElement powerusertag = doc.createElement("poweruserview");
    QString on="no";
    if (powerUserView()) on="yes";
    powerusertag.setAttribute("on",on);
    generaltag.appendChild(powerusertag);

    QDomElement persoenlichekontensummentag = doc.createElement("persoenliche_kontensumme");
    on="no";
    if (persoenlicheKontensumme()) on="yes";
    persoenlichekontensummentag.setAttribute("on",on);
    generaltag.appendChild(persoenlichekontensummentag);

    QDomElement typecolumntag = doc.createElement("typecolumn");
    on="no";
    if (showTypeColumn()) on="yes";
    typecolumntag.setAttribute("show",on);
    generaltag.appendChild(typecolumntag);
    
    QDomElement pspcolumntag = doc.createElement("pspcolumn");
    on="no";
    if (showPSPColumn()) on="yes";
    pspcolumntag.setAttribute("show",on);
    generaltag.appendChild(pspcolumntag);
    
    QDomElement srseltag = doc.createElement("specialremunselector");
    on="no";
    if (showSpecialRemunSelector()) on="yes";
    srseltag.setAttribute("show",on);
    generaltag.appendChild(srseltag);
    
    QDomElement usedefaultcommentifuniquetag = doc.createElement("usedefaultcommentifunique");
    on="no";
    if (useDefaultCommentIfUnique()) on="yes";
    usedefaultcommentifuniquetag.setAttribute("on",on);
    generaltag.appendChild(usedefaultcommentifuniquetag);

    if (useCustomFont()) {
        QDomElement customfonttag = doc.createElement("customfont");
        QString size="";
        size=size.setNum(customFontSize());
        customfonttag.setAttribute("family",customFont());
        customfonttag.setAttribute("size",size);
        generaltag.appendChild(customfonttag);
    }

    QDomElement singleclicktag = doc.createElement("singleclickactivation");
    on="no";
    if (singleClickActivation()) on="yes";
    singleclicktag.setAttribute("on",on);
    generaltag.appendChild(singleclicktag);

    QDomElement dragndroptag = doc.createElement("dragndrop");
    on="no";
    if (dragNDrop()) on="yes";
    dragndroptag.setAttribute("on",on);
    generaltag.appendChild(dragndroptag);

    QDomElement overtimeregulatedmodetag = doc.createElement("overtimeregulatedmode");
    on =overtimeRegulatedModeActive() ? "yes" : "no";
    overtimeregulatedmodetag.setAttribute("on",on);
    generaltag.appendChild(overtimeregulatedmodetag);

    QDomElement overtimeothermodetag = doc.createElement("overtimeothermode");
    on =overtimeOtherModeActive() ? "yes" : "no";
    overtimeothermodetag.setAttribute("on",on);
    generaltag.appendChild(overtimeothermodetag);

    QDomElement nightmodetag = doc.createElement("nightmode");
    on =nightModeActive() ? "yes" : "no";
    nightmodetag.setAttribute("on",on);
    generaltag.appendChild(nightmodetag);

    QDomElement lastrecordedtimestamptag = doc.createElement("lastrecordedtimestamp");
    QString ts = lastRecordedTimestamp().toString(timestampFormat());
    lastrecordedtimestamptag.setAttribute("value",ts);
    generaltag.appendChild(lastrecordedtimestamptag);

    QDomElement publicholidaymodetag = doc.createElement("publicholidaymode");
    on =publicHolidayModeActive() ? "yes" : "no";
    publicholidaymodetag.setAttribute("on",on);
    generaltag.appendChild(publicholidaymodetag);

    QDomElement kontodlgwindowpositiontag = doc.createElement( "kontodlgwindowposition" );
    kontodlgwindowpositiontag.setAttribute("x",unterKontoWindowPosition.x());
    kontodlgwindowpositiontag.setAttribute("y",unterKontoWindowPosition.y());
    generaltag.appendChild(kontodlgwindowpositiontag);

    QDomElement kontodlgwindowsizetag = doc.createElement("kontodlgwindowsize");
    kontodlgwindowsizetag.setAttribute("width",unterKontoWindowSize.width());
    kontodlgwindowsizetag.setAttribute("height",unterKontoWindowSize.height());
    generaltag.appendChild(kontodlgwindowsizetag);

    for (unsigned int i=0; i<defaultcommentfiles.size(); i++) {
        QDomElement defaultcommentfiletag = doc.createElement("defaultcommentsfile");
        defaultcommentfiletag.setAttribute("name",defaultcommentfiles[i]);
        generaltag.appendChild(defaultcommentfiletag);
    }

    QDomElement qdeBackends = doc.createElement("backends");
    qdeBackends.setAttribute("names", backends);
    generaltag.appendChild(qdeBackends);

    QDomElement qdeDatabase = doc.createElement("database");
    // do not save database server and database name for now until we have some
    // concept how to update them in case they ever change
    //qdeDatabase.setAttribute("server", databaseserver);
    //qdeDatabase.setAttribute("name", database);
    if (!databaseuser.isEmpty())
      qdeDatabase.setAttribute("user", databaseuser);
    if (!databasepassword.isEmpty())
      qdeDatabase.setAttribute("password", databasepassword);
    if (qdeDatabase.hasAttributes())
      qdeBackends.appendChild(qdeDatabase);
  }

  QDomElement aktivtag = doc.createElement("aktives_konto");
  QString abt,ko,uko;
  int idx;
  abtList->getAktiv(abt,ko,uko,idx);
  aktivtag.setAttribute("abteilung",abt);
  aktivtag.setAttribute("konto",ko);
  aktivtag.setAttribute("unterkonto",uko);
  aktivtag.setAttribute("index",idx);
  generaltag.appendChild(aktivtag);

  root.appendChild(generaltag);

  for (AbteilungsListe::iterator abtPos=abtList->begin(); abtPos!=abtList->end(); ++abtPos) {
    QString abteilungstr=abtPos->first;
    QDomElement abttag = doc.createElement( "abteilung" );
    abttag.setAttribute("name",abteilungstr);
    bool abtIsEmpty=true;

    if (global) {
        if (abtList->hasBgColor(abteilungstr)) {
          abtIsEmpty=false;
          abttag.setAttribute("color",color2str(abtList->getBgColor(abteilungstr)));
        }
    }

    KontoListe* kontoliste=&(abtPos->second);
    for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
      QString kontostr=kontPos->first;
      QDomElement konttag = doc.createElement( "konto" );
      konttag.setAttribute("name",kontostr);

      bool kontIsEmpty=true;
      bool kontpers=((abtList->getKontoFlags(abteilungstr,kontostr))&UK_PERSOENLICH);
      if (kontpers) {
        kontIsEmpty=false;
        konttag.setAttribute("persoenlich","yes");
      }

      if (global) {
        if (abtList->hasBgColor(abteilungstr,kontostr)) {
          kontIsEmpty=false;
          konttag.setAttribute("color",color2str(abtList->getBgColor(abteilungstr,kontostr)));
        }
      }

      UnterKontoListe* unterkontoliste=&(kontPos->second);
      for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin();
           ukontPos!=unterkontoliste->end(); ++ukontPos) {
        QString unterkontostr=ukontPos->first;
        QDomElement ukonttag = doc.createElement( "unterkonto" );
        ukonttag.setAttribute("name",unterkontostr);

        bool ukontIsEmpty=true;
        bool ukontpers=(abtList->getUnterKontoFlags(abteilungstr,kontostr,unterkontostr)&UK_PERSOENLICH);
        if (ukontpers) {
          ukontIsEmpty=false;
          ukonttag.setAttribute("persoenlich","yes");
        }
        else if (kontpers) {
          ukontIsEmpty=false;
          ukonttag.setAttribute("persoenlich","no"); // Falls Einstellung vom zugeh. Konto abweicht.
        }

        if (global) {
          if (abtList->hasBgColor(abteilungstr,kontostr,unterkontostr)) {
            ukontIsEmpty=false;
            ukonttag.setAttribute("color",color2str(abtList->getBgColor(abteilungstr,kontostr,unterkontostr)));
          }
        }

        if (!global) {
          QStringList bereitschaften= ukontPos->second.getBereitschaft();
          for (int i=0; i<bereitschaften.size(); i++)
          {
            QDomElement bertag = doc.createElement( "bereitschaft" );
            bertag.setAttribute("type",bereitschaften.at(i));
            ukonttag.appendChild (bertag);
          }
        }

        if ((!global)||alwaysSaveEintrag) {
          EintragsListe* eintragsliste=&(ukontPos->second);

          for (EintragsListe::iterator etPos=eintragsliste->begin();
               etPos!=eintragsliste->end(); ++etPos) {
            if (!(etPos->second==UnterKontoEintrag())) {
              ukontIsEmpty=false;

              QString etStr=QString().setNum(etPos->first);
              QDomElement ettag = doc.createElement( "eintrag" );
              ettag.setAttribute("nummer",etStr);


              if (abtList->isPersoenlich(abteilungstr,kontostr,unterkontostr,etPos->first)) {
                ettag.setAttribute("persoenlich","yes");
              } else if (ukontpers) {
                ettag.setAttribute("persoenlich","no");
              }
              if (etPos->second.kommentar!="")
                ettag.setAttribute("kommentar",etPos->second.kommentar);
              if (!global) {
                if (etPos->second.sekunden!=0)
                  ettag.setAttribute("sekunden",etPos->second.sekunden);
                if (etPos->second.sekundenAbzur!=0)
                  ettag.setAttribute("abzurechnende_sekunden",etPos->second.sekundenAbzur);
                QSet<QString> specialRemunSet = etPos->second.getAchievedSpecialRemunSet();
                foreach (QString specialremun, specialRemunSet) {
                  QDomElement srtag = doc.createElement( "specialremun" );
                  srtag.setAttribute("name",specialremun);
                  ettag.appendChild(srtag);
                }
              }
              ukonttag.appendChild (ettag);
            }
          }
        }

        if (!ukontIsEmpty) {
          kontIsEmpty=false;
          if (ukontPos->second.getFlags()&IS_CLOSED)
            ukonttag.setAttribute("open","no");
          konttag.appendChild(ukonttag);
        }
      }
      if (!kontIsEmpty) {
        abtIsEmpty=false;
        if (kontPos->second.getFlags()&IS_CLOSED)
          konttag.setAttribute("open","no");
        abttag.appendChild(konttag);
      }
    }
    if (!abtIsEmpty) {
      if (abtPos->second.getFlags()&IS_CLOSED)
        abttag.setAttribute("open","no");
      root.appendChild( abttag );
      }
  }

  QString filename(configDir.filePath(global ? "settings.xml" : "zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".xml"));
  QFile fnew(filename + ".tmp");
  if (!fnew.open(QIODevice::WriteOnly)) {
      QMessageBox::critical(NULL, QObject::tr("sctime: saving settings"), QObject::tr("opening file %1 for writing: %2").arg(fnew.fileName(), fnew.errorString()));
      return;
  }
  // may contain passwords and private data in general
  fnew.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
  const char xmlcharmap[] = "UTF-8";
  QTextStream stream(&fnew);
  stream.setCodec(xmlcharmap);
  stream<<"<?xml version=\"1.0\" encoding=\""<< xmlcharmap <<"\"?>"<<endl;
  stream<<doc.toString()<<endl;
  fnew.close();
  QFile fcurrent(filename);
  if (global && backupSettingsXml && fcurrent.exists()) {
    // Backup erstellen für settings.xml
    QFile fbackup(filename + ".bak");
    if (fbackup.exists()) fbackup.remove();
    if (!fcurrent.copy(fbackup.fileName()))
      QMessageBox::warning(NULL, QObject::tr("sctime: saving settings"),
                           QObject::tr("%1 cannot be copied to %2: %3").arg(filename, fbackup.fileName(), fcurrent.errorString()));
    else
      backupSettingsXml = false;
  }
#ifndef WIN32
  // unter Windows funktioniert kein "rename", wenn es den Zielnamen schon gibt.
  // Doch unter UNIX kann ich damit Dateien atomar ersetzen.
  if (!rename(fnew.fileName().toLocal8Bit(), filename.toLocal8Bit())) return;
  QMessageBox::information(NULL, QObject::tr("sctime: saving settings"),
                        QObject::tr("%1 cannot be renamed to %2: %3").arg(fnew.fileName(), filename, strerror(errno)));
#endif
  fcurrent.remove();
  if (!fnew.rename(filename))
    QMessageBox::critical(NULL, QObject::tr("sctime: saving settings"),
                         QObject::tr("%1 cannot be renamed to %2: %3").arg(fnew.fileName(), filename, fnew.errorString()));

  #endif
}

QString SCTimeXMLSettings::color2str(const QColor& color)
{
  return QString().sprintf("#%.2x%.2x%.2x",color.red(),color.green(),color.blue());
}

QColor SCTimeXMLSettings::str2color(const QString& str)
{
  if (str.length()!=7)
    return Qt::white;
  QColor color;
  color.setRed(str.mid(1,2).toInt(NULL,16));
  color.setGreen(str.mid(3,2).toInt(NULL,16));
  color.setBlue(str.mid(5,2).toInt(NULL,16));
  return color;
}

