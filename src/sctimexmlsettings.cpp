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

#include <iostream>

#include "sctimexmlsettings.h"
#include "abteilungsliste.h"
#include "qfile.h"
#include "qdir.h"
//Added by qt3to4:
#include <QTextStream>
#include "globals.h"
#include "utils.h"
#include "timecounter.h"
#include "qregexp.h"
#include "qtextcodec.h"
#ifndef NO_XML
#include "qdom.h"
#endif
#define WIN_CODEC "utf8"

/** Schreibt die Eintraege in ein Shellskript */
void SCTimeXMLSettings::writeShellSkript(AbteilungsListe* abtList)
{
  if (abtList->checkInState()) {
      std::cout<<"Shell Skript nicht geschrieben, da bereits eingecheckt"<<std::endl;
      return;
  }
  QString filename="/zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".sh";
  QFile shellFile(configDir+filename);

  if (!shellFile.open(QIODevice::WriteOnly)) {
      std::cout<<"Kann Ausgabedatei nicht schreiben: "<<(configDir+filename).toLocal8Bit().constData()<<std::endl;
      return;
  }

  QTextStream stream( & shellFile);

  int sek, abzurSek;
  abtList->getGesamtZeit(sek,abzurSek);
  QRegExp apostrophExp=QRegExp("'");

  TimeCounter tc(sek), tcAbzur(abzurSek);
#ifdef WIN32
  stream.setCodec(QTextCodec::codecForName ( WIN_CODEC ));
#endif
  QString codec=stream.codec()->name();
  stream<<"#!/bin/sh\n\n";
  stream<<"# Zeit Aufrufe von sctime "<<QUOTEME(VERSIONSTR)<<" generiert \n"
    <<"# Gesamtzeit: "<<tc.toString()<<"/"<<tcAbzur.toString()<<"\n";
  stream<<"ZEIT_ENCODING="<<codec<<"\n";
  stream<<"export ZEIT_ENCODING\n";

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
             stream<<zeitKommando<<" "<<
                     abtList->getDatum().toString("dd.MM.yyyy")<<" "<<
                     kontPos->first<<" "<<ukontPos->first<<"\t"<<
                     roundTo(1.0/3600*etPos->second.sekunden,0.01)<<"/"<<
                     roundTo(1.0/3600*etPos->second.sekundenAbzur,0.01)<<
                     " \'"<<kommentar.simplifyWhiteSpace()<<"\'\n";
          }
        }
      }
    }
  }
  stream<<endl;
  shellFile.close();
}

bool SCTimeXMLSettings::moveToCheckedIn(AbteilungsListe* abtList)
{
  QString checkedinstr="/checkedin";
  QFileInfo qf(configDir+checkedinstr);
  QDir qd(configDir);
  if (!qf.exists()) {
    if (!qd.mkdir(configDir+checkedinstr)) {
      return false;
    }
  }
  if (!qf.isDir()) {
    return false;
  }
  // erst .sh wegschieben
  QString filename="zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".sh";
  if (!qd.rename(configDir+"/"+filename, configDir+checkedinstr+"/"+filename))
    return false;
  // dann .xml
  filename="zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".xml";
  if (!qd.rename(configDir+"/"+filename, configDir+checkedinstr+"/"+filename))
    return false;
  return true;
}

void SCTimeXMLSettings::readSettings(AbteilungsListe* abtList)
{
  abtList->clearKonten();
  // Erst globale Einstellungen lesen
  readSettings(true, abtList);
  // Dann die Tagesspezifischen
  readSettings(false, abtList);
}

/**
 * Liest alle Einstellungen.
 */

void SCTimeXMLSettings::readSettings(bool global, AbteilungsListe* abtList)
{
  #ifndef NO_XML
  QDomDocument doc("settings");
  QString filename;

  if (global)
    filename="/settings.xml";
  else {
    filename="/zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".xml";
    QFileInfo qf(configDir+"/checkedin"+filename);
    if (qf.exists()) {
      filename="/checkedin"+filename;
      abtList->setCheckInState(true);
    }
    else
      abtList->setCheckInState(false);
  }

  QFile f( configDir + filename );
  if ( !f.open( QIODevice::ReadOnly ) ) {
      return;
      }
  if ( !doc.setContent( &f ) ) {
      f.close();
      return;
  }
  f.close();

  QDomElement aktiveskontotag;

  QDomElement docElem = doc.documentElement();

  if (global) {
    defaultcommentfiles.clear();
    columnwidth.clear();
    setUseCustomFont(false);
  }

  for(QDomNode node1 = docElem.firstChild(); !node1.isNull(); node1 = node1.nextSibling()) {
    QDomElement elem1 = node1.toElement();
    if( !elem1.isNull() ) {
      if (elem1.tagName()=="abteilung") {
        QString abteilungstr=elem1.attribute("name");
        if (abteilungstr.isNull()) continue;
        abtList->insertAbteilung(abteilungstr);
        if (elem1.attribute("open")=="yes")
          abtList->setAbteilungFlags(abteilungstr,IS_CLOSED,FLAG_MODE_NAND);
        if (elem1.attribute("open")=="no")
          abtList->setAbteilungFlags(abteilungstr,IS_CLOSED,FLAG_MODE_OR);

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

              bool kontoPers=((abtList->getKontoFlags(abteilungstr,kontostr))&UK_PERSOENLICH);

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
            if (elem2.tagName()=="dragndrop") {
              setDragNDrop(elem2.attribute("on")=="yes");
            }
            if (elem2.tagName()=="max_working_time") {
              QString secondsstr=elem2.attribute("seconds");
              if (secondsstr.isNull()) continue;
              _maxWorkingTime=secondsstr.toInt();
            }
            if (elem2.tagName()=="aktives_konto") {
              aktiveskontotag=elem2; // Aktives Konto merken und zum Schluss setzen, damit es vorher erzeugt wurde
            }
            if (elem2.tagName()=="windowposition") {
              QString xstr=elem2.attribute("x");
              if (xstr.isNull()) continue;
              QString ystr=elem2.attribute("y");
              if (ystr.isNull()) continue;
              mainwindowPosition=QPoint(xstr.toInt(),ystr.toInt());
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
            if (elem2.tagName()=="poweruserview") {
              setPowerUserView((elem2.attribute("on")=="yes"));
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
              QString xstr=elem2.attribute("x");
              if (xstr.isNull()) continue;
              QString ystr=elem2.attribute("y");
              if (ystr.isNull()) continue;
              unterKontoWindowPosition=QPoint(xstr.toInt(),ystr.toInt());
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
          }
        }
      }
    }
  }

  // fixme: Hack fuer reibungsfreie Migration auf neue sctime Version...
  if ((global) && (defaultcommentfiles.empty())) {
      std::cout<<defaultcommentfiles.size();
      defaultcommentfiles.push_back("defaultcomments.xml");
  }

  if (!aktiveskontotag.isNull()) {
    QString abtstr=aktiveskontotag.attribute("abteilung");
    QString kostr=aktiveskontotag.attribute("konto");
    QString ukostr=aktiveskontotag.attribute("unterkonto");
    int idx=aktiveskontotag.attribute("index").toInt();
    abtList->setAsAktiv(abtstr,kostr,ukostr,idx);
  }
  #endif
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
      std::cout<<"xml nicht geschrieben, da bereits eingecheckt"<<std::endl;
      return;
  }
  #ifndef NO_XML
  QDomDocument doc("settings");

  QDomElement root = doc.createElement( "sctime" );
  root.setAttribute("version",QUOTEME(VERSIONSTR));
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

    if (MAX_WORKTIME_DEFAULT!=_maxWorkingTime) {
        QDomElement maxworktag = doc.createElement( "max_working_time" );
        maxworktag.setAttribute("seconds",_maxWorkingTime);
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

    for (unsigned int i=0; i<columnwidth.size(); i++) {
        QDomElement columnwidthtag = doc.createElement("column");
        columnwidthtag.setAttribute("width",columnwidth[i]);
        generaltag.appendChild(columnwidthtag);
    }

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
    
    QDomElement typecolumntag = doc.createElement("typecolumn");
    on="no";
    if (showTypeColumn()) on="yes";
    typecolumntag.setAttribute("show",on);
    generaltag.appendChild(typecolumntag);
    
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

    QDomElement kontodlgwindowpositiontag = doc.createElement( "kontodlgwindowposition" );
    kontodlgwindowpositiontag.setAttribute("x",unterKontoWindowPosition.x());
    kontodlgwindowpositiontag.setAttribute("y",unterKontoWindowPosition.y());
    generaltag.appendChild(kontodlgwindowpositiontag);

    QDomElement kontodlgwindowsizetag = doc.createElement("kontodlgwindowsize");
    kontodlgwindowsizetag.setAttribute("width",unterKontoWindowSize.width());
    kontodlgwindowsizetag.setAttribute("height",unterKontoWindowSize.height());
    generaltag.appendChild(kontodlgwindowsizetag);

    for (int i=0; i<defaultcommentfiles.size(); i++) {
        QDomElement defaultcommentfiletag = doc.createElement("defaultcommentsfile");
        defaultcommentfiletag.setAttribute("name",defaultcommentfiles[i]);
        generaltag.appendChild(defaultcommentfiletag);
    }
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

  QString filename;

  if (global)
    filename=configDir+"/settings.xml";
  else
    filename=configDir+"/zeit-"+abtList->getDatum().toString("yyyy-MM-dd")+".xml";

  QFile f(filename+"~");
  f.remove();
  if ( !f.open( QIODevice::WriteOnly) ) {
      std::cerr<<"Kann Settings nicht speichern"<<std::endl;
      return;
  }
  QTextStream stream( & f);
#ifdef WIN32
  stream.setCodec(QTextCodec::codecForName ( WIN_CODEC ));
#endif
  QString codec=stream.codec()->name();
  stream<<"<?xml version=\"1.0\" encoding=\""<<codec<<"\"?>"<<endl;
  stream<<doc.toString()<<endl;

  f.close();
  QFile oldfile(filename);
  oldfile.remove();
  f.rename(filename);
  #endif
}
