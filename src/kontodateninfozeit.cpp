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

#include "kontodateninfozeit.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <QRegExp>
#include "globals.h"
#include "utils.h"
#include <QMessageBox>
#include <string>

Einchecker::Einchecker(AbteilungsListe * abtlist)
{
  abtList=abtlist;
  //connect(this,SIGNAL(finishedEintrag()),this,SLOT(nextEintrag()));
  /*connect( &proc, SIGNAL(processExited()),
            this, SLOT(onExit()) );*/
  //firstAbteilung();
}

void Einchecker::firstAbteilung()
{
  abtPos=abtList->begin();
  firstKonto();
}

void Einchecker::firstKonto()
{
  kontPos=abtPos->second.begin();
  firstUnterkonto();
}

void Einchecker::firstUnterkonto()
{
  ukontPos=kontPos->second.begin();
  firstEintrag();
}

void Einchecker::firstEintrag()
{
  etPos=ukontPos->second.begin();
  nextEintrag();
}

void Einchecker::nextAbteilung()
{
  abtPos++;
  if (abtPos==abtList->end())
    return;
  else {
    firstKonto();
  }
}

void Einchecker::nextKonto()
{
  kontPos++;
  if (kontPos==abtPos->second.end())
    nextAbteilung();
  else {
    firstUnterkonto();
  }
}

void Einchecker::nextUnterkonto()
{
  ukontPos++;
  if (ukontPos==kontPos->second.end())
     nextKonto();
  else
     firstEintrag();

}

void Einchecker::nextEintrag()
{
    /*while ((etPos->second.sekunden==0)&&(etPos->second.sekundenAbzur==0)&&(etPos!=ukontPos->second.end())) {
      std::cout<<etPos->first<<std::endl;
      etPos++;
    }
    if (etPos!=ukontPos->second.end()) {
      std::cout<<"Starting..."<<std::endl;
      checkin(abtList->getDatum(), kontPos->first, ukontPos->first, etPos->second.sekunden,
                        etPos->second.sekundenAbzur, etPos->second.kommentar);
    }
    else {
      nextUnterkonto();
    }*/
}

bool Einchecker::checkin(QDate date, const QString& konto, const QString& uko, int sek, int sekabzur, QString kommentar)
{
    char cstr[800];
    QRegExp apostrophExp=QRegExp("'");
    kommentar=kommentar.replace(apostrophExp,"");
   /* proc.clearArguments();
    proc.addArgument( "sh" );
    proc.addArgument( "zeit" );
    proc.addArgument( date.toString("dd.MM.yyyy") );
    proc.addArgument( konto );
    proc.addArgument( uko );
    proc.addArgument( QString("").arg(roundTo(1.0/3600*sek,0.01)) );
    proc.addArgument( QString("").arg(roundTo(1.0/3600*sekabzur,0.01)) );

    proc.addArgument( kommentar.simplifyWhiteSpace() );
    connect( &proc, SIGNAL(processExited()),
            this, SLOT(onExit()) );
    connect( &proc, SIGNAL(processExited()),
            this, SLOT(onExit()) );*/

    QString command="zeit "+date.toString("dd.MM.yyyy")+" "+konto+" "+uko+" "+QString("").arg(roundTo(1.0/3600*sek,0.01))
                    +" "+QString("").arg(roundTo(1.0/3600*sekabzur,0.01))+" "+kommentar.simplified();

    std::cout<<"uko"<<uko.toLocal8Bit().constData()<<std::endl;
    /*
    if ( !proc.launch("n\n") ) {
        // error handling
        QMessageBox::critical( 0,
                "Fataler Fehler",
                "Kann Zeitkommando nicht starten.",
                "Abbruch" );
        emit criticalError();
    }*/
    FILE* file;

    if ((file = popen("echo -e n\\n|"+command.toLatin1()+" 2>&1", "r")) == NULL) {
      QMessageBox::critical( 0,
                "Fataler Fehler",
                "Kann Zeitkommando nicht starten.",
                "Abbruch" );
      return false;
    }
    bool ret=true;
    QString  qs;
    char c;
    while ((!feof(file)) && (!ferror(file))) {

      // Konto, unterkonto sind eindeutig durch leerzeichen getrennt,
      // der Rest muss gesondert behandelt werden.
      if (fread(&c,1,1,file)>0)
        qs+=c;
    }
    pclose(file);
    std::cout<<qs.toLocal8Bit().constData()<<std::endl;

    QRegExp errorExp=QRegExp("\\* STOP \\*.*\\*{10,100}([^\\*].*)");
    int pos = errorExp.indexIn(qs);
    if (pos > -1) {
          std::cout<<"Fehler aufgetreten"<<std::endl;
          QString error = errorExp.cap(1);
          std::cout<<"error:"<<error.toLocal8Bit().constData()<<std::endl;
          QMessageBox::warning( 0,
                "Warnung",
                error,
                "OK" );
          ret=false;

    }
    return ret;
}

void Einchecker::onExit()
{
   /* QRegExp errorExp=QRegExp("\\* STOP \\*.*\\*{10,100}(.*)");
    QString output;
    // Read and process the data.
    output = (QString)proc.readStdout();
    std::cout<<"OutPut"<<output<<std::endl;
    int pos = errorExp.search(output);
    if (pos > -1) {
      QString error = errorExp.cap(1);
      QString error = "";
      QStringList arglist = proc.arguments();
      QStringList::Iterator it = arglist.begin();
      while( it != arglist.end() ) {
        error += ( *it );
        ++it;
      }
      QMessageBox::warning( 0,
                "Warnung",
                error,
                "OK" );
    //}
    etPos++;
    emit finishedEintrag();*/
}

KontoDatenInfoZeit::KontoDatenInfoZeit()
{
    m_DatenFileName="";
}

KontoDatenInfoZeit::KontoDatenInfoZeit(QString sourcefile)
{
    m_DatenFileName=sourcefile;
}

bool KontoDatenInfoZeit::readCommentsFromZeitFile(FILE* file, AbteilungsListe * abtList)
{
	    char zeile[800];
    int unterkontoCounter=0;

    while (!feof(file)) {
      // Konto, unterkonto sind eindeutig durch leerzeichen getrennt,
      // der Rest muss gesondert behandelt werden.
        if (fscanf(file,"%[^\n]",zeile)==1) {
        // Falls alle drei Strings korrekt eingelesen wurden...

            unterkontoCounter++;
            QString qstringzeile=QString::fromLocal8Bit(zeile);
            QStringList ql = qstringzeile.split("|");

            if (ql.size()<7) {
                continue;
            }

            QString abt = ql[0].simplified();
            QString konto = ql[2].simplified();
            QString unterkonto = ql[7].simplified();

            QString verantwortlicher = ql[9].simplified();
            QString typ = ql[10].simplified();

            QString beschreibung = ql[11].simplified();

            if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.

            //abtList->insertEintrag(abt,konto,unterkonto);
            //abtList->setDescription(abt,konto,unterkonto,DescData(beschreibung,verantwortlicher,typ));
            //abtList->setUnterKontoFlags(abt,konto,unterkonto,IS_IN_DATABASE,FLAG_MODE_OR);
            
            if (ql.size()>12) {
              // Do not simplify comment to preserve intentional whitespace.
              QString commentstr = ql[12];
              if (!commentstr.isEmpty()) {
                UnterKontoListe::iterator itUk;
                UnterKontoListe* ukl;
                if (abtList->findUnterKonto(itUk,ukl,abt,konto,unterkonto)) {
                  itUk->second.addDefaultComment(commentstr);
                }
              }
            }
        }
        fscanf(file,"\n");
    }
    return (unterkontoCounter>0);
}

bool KontoDatenInfoZeit::readZeitFile(FILE* file, AbteilungsListe * abtList)
{
    char zeile[800];
    int unterkontoCounter=0;

    while (!feof(file)) {
      // Konto, unterkonto sind eindeutig durch leerzeichen getrennt,
      // der Rest muss gesondert behandelt werden.
        if (fscanf(file,"%[^\n]",zeile)==1) {
        // Falls alle drei Strings korrekt eingelesen wurden...

            unterkontoCounter++;
            QString qstringzeile=QString::fromLocal8Bit(zeile);
            QStringList ql = qstringzeile.split("|");

            if (ql.size()<7) {
                continue;
            }

            QString abt = ql[0].simplified();
            QString konto = ql[2].simplified();
            QString unterkonto = ql[7].simplified();

            QString verantwortlicher = ql[9].simplified();
            QString typ = ql[10].simplified();

            QString beschreibung = ql[11].simplified();

            if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.

            abtList->insertEintrag(abt,konto,unterkonto);
            abtList->setDescription(abt,konto,unterkonto,DescData(beschreibung,verantwortlicher,typ));
            abtList->setUnterKontoFlags(abt,konto,unterkonto,IS_IN_DATABASE,FLAG_MODE_OR);
            
            if (ql.size()>12) {
              // Do not simplify comment to preserve intentional whitespace.
              QString commentstr = ql[12];
              if (!commentstr.isEmpty()) {
                UnterKontoListe::iterator itUk;
                UnterKontoListe* ukl;
                if (abtList->findUnterKonto(itUk,ukl,abt,konto,unterkonto)) {
                  itUk->second.addDefaultComment(commentstr);
                }
              }
            }
        }
        fscanf(file,"\n");
    }
    return (unterkontoCounter>0);
}

bool KontoDatenInfoZeit::readInto(AbteilungsListe * abtList)
{
  abtList->clear();
  FILE* file;
  int rc;
    
  if (m_DatenFileName.isEmpty()) {
    QString command;
    if (m_Kommando.isEmpty()){							
      command="zeitkonten --mikrokonten --separator='|'";
    }else
      command=m_Kommando;
      putenv("LC_CTYPE=de_DE.UTF-8");		
			
    file = popen(command.toLatin1(), "r");    
    if (!file) {
      std::cerr<<"Kann Kommando \""<<command.toStdString()<<"\" nicht ausfuehren."<<std::endl;
      return false;
    }    
    
  } else {
      file = fopen(m_DatenFileName.toLatin1(), "r");
      if (!file) {
          std::cerr<<"Kann "<<m_DatenFileName.toLocal8Bit().constData()<<" nicht oeffnen."<<std::endl;
          return false;
      }
  }
  
	bool result=readZeitFile(file,abtList);
		
  if (m_DatenFileName.isEmpty()){		
      rc = pclose(file);
      if(rc!=0)
      {
				std::cerr << "Kann Kommando \"zeitkonten --mikrokonten --separator='|'\" nicht ausfuehren. Kontoliste konnte nicht geladen werden."<<std::endl;
			}
	}
  else
      fclose(file);
  return result;
}

bool KontoDatenInfoZeit::readDefaultComments(AbteilungsListe * abtList)
{
	//abtList->clear();
  FILE* file;
  int rc;
  
  if (m_DatenFileName.isEmpty()) {
    QString command;
    if (m_Kommando.isEmpty()){			
      command="zeitkonten --mikrokonten --separator='|'";}
    else
      command=m_Kommando;      
    
		putenv("LC_CTYPE=de_DE.UTF-8");		  
		
    file = popen(command.toLatin1(), "r");           
    if (!file || file ==NULL) {
      std::cerr<<"Kann Kommando \""<<command.toStdString()<<"\" nicht ausfuehren."<<std::endl;
      return false;
    }
    
    
  } else {
      file = fopen(m_DatenFileName.toLatin1(), "r");
      if (!file) {
          std::cerr<<"Kann "<<m_DatenFileName.toLocal8Bit().constData()<<" nicht oeffnen."<<std::endl;
          return false;
      }
  }
  
	bool result=readCommentsFromZeitFile(file,abtList);
		
  if (m_DatenFileName.isEmpty()){
      rc=pclose(file);      
      if(rc!=0)
      {
				std::cerr << "Kann Kommando \"zeitkonten --mikrokonten --separator='|'\" nicht ausfuehren. Defaultkommentare konnten nicht geladen werden."<<std::endl;
			}
		}
  else
      fclose(file);
  return result;
}

void KontoDatenInfoZeit::setKommando(const QString& command)
{
  m_Kommando=command;
}

bool KontoDatenInfoZeit::checkIn(AbteilungsListe* abtlist)
{
  /*QString filename="/checkedin/zeit-"+abtlist->getDatum().toString("yyyy-MM-dd")+".sh";
  system("xterm -hold -e sh "+configDir+filename+" &");*/
  AbteilungsListe::iterator abtPos;
  Einchecker ec(abtlist);
  bool ret=true;
  for (abtPos=abtlist->begin(); abtPos!=abtlist->end(); ++abtPos) {
    QString abt=abtPos->first;
    KontoListe* kontoliste=&(abtPos->second);
    for (KontoListe::iterator kontPos=kontoliste->begin(); kontPos!=kontoliste->end(); ++kontPos) {
      UnterKontoListe* unterkontoliste=&(kontPos->second);
      for (UnterKontoListe::iterator ukontPos=unterkontoliste->begin(); ukontPos!=unterkontoliste->end(); ++ukontPos) {
        EintragsListe* eintragsliste=&(ukontPos->second);
        for (EintragsListe::iterator etPos=eintragsliste->begin(); etPos!=eintragsliste->end(); ++etPos) {
          if (etPos->second.sekunden!=0) {

             ret=ec.checkin(abtlist->getDatum(), kontPos->first, ukontPos->first, etPos->second.sekunden,
                        etPos->second.sekundenAbzur, etPos->second.kommentar);


          }
        }
      }
    }
  }
  return ret;
}

