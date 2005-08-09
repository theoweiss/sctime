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

// Wird unter Win32 nicht benoetigt
#ifndef WIN32

#include "kontodateninfozeit.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include "qregexp.h"
#include "globals.h"
#include "utils.h"
#include "qmessagebox.h"


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
                    +" "+QString("").arg(roundTo(1.0/3600*sekabzur,0.01))+" "+kommentar.simplifyWhiteSpace();

    std::cout<<"uko"<<uko.toStdString()<<std::endl;
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

    if ((file = popen("echo -e n\\n|"+command+" 2>&1", "r")) == NULL) {
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
    std::cout<<qs.toStdString()<<std::endl;

    QRegExp errorExp=QRegExp("\\* STOP \\*.*\\*{10,100}([^\\*].*)");
    int pos = errorExp.search(qs);
    if (pos > -1) {
          std::cout<<"Fehler aufgetreten"<<std::endl;
          QString error = errorExp.cap(1);
          std::cout<<"error:"<<error.toStdString()<<std::endl;
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


bool KontoDatenInfoZeit::readInto(AbteilungsListe * abtList)
{
  abtList->clear();
  char rest[800], konto[200], unterkonto[200];

  FILE* file;

  if ((file = popen("zeitkonten --beschreibung", "r")) != NULL)
  {
    int unterkontoCounter=0;

    QRegExp kostenstelle("\\s.\\d\\d\\d\\d\\d\\b");
    while (!feof(file)) {
      // Konto, unterkonto sind eindeutig durch leerzeichen getrennt,
      // der Rest muss gesondert behandelt werden.
      if (fscanf(file,"%s%s%[^\n]",konto,unterkonto,rest)==3) {
        // Falls alle drei Strings korrekt eingelesen wurden...

        unterkontoCounter++;
        QString qstringrest(rest), abt,beschreibung;

        // Kostenstelle trennt Abteilung von Beschreibung, also
        // dort splitten
        abt = qstringrest.section(kostenstelle,0,0).simplifyWhiteSpace();
        beschreibung = qstringrest.section(kostenstelle,1);

        if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.

        abtList->insertEintrag(abt,konto,unterkonto);
        abtList->setBeschreibung(abt,konto,unterkonto,beschreibung);
        abtList->setUnterKontoFlags(abt,konto,unterkonto,IS_IN_DATABASE,FLAG_MODE_OR);
      }
    }
    pclose(file);
    return (unterkontoCounter>0);
  }
  else
  {
    std::cerr<<"Kann \"zeitkonten\" nicht ausfuehren."<<std::endl;
    return false;
  }
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

#endif
