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

#include <errno.h>
#include <string>

#ifdef WIN32
#include <windows.h>

#else
#include <stdio.h>
#include <unistd.h>
#endif

#include <QMessageBox>
#include <QFile>
#include "globals.h"
#include "kontodateninfozeit.h"

KontoDatenInfoZeit::KontoDatenInfoZeit() {
    m_DatenFileName="";
}

KontoDatenInfoZeit::KontoDatenInfoZeit(QString sourcefile) {
    m_DatenFileName=sourcefile;
}

bool KontoDatenInfoZeit::readCommentsFromZeitFile(QTextStream& ts, AbteilungsListe* abtList) {
  return readFile(ts, abtList, true);
}

bool KontoDatenInfoZeit::readFile(QTextStream& ts, AbteilungsListe * abtList, bool comments_only) {
  int zeilen = 0;
  for (QString l; !(l = ts.readLine()).isNull();) {
    zeilen++;
    if (l.isEmpty()) continue;
    QStringList ql = l.split("|");
    if (ql.size() < 12) {
      QMessageBox::critical(NULL, "sctime: Konfigurationsproblem.",
                            QString("Die Kontenliste muss mindestens 12 Spalten haben, Zeile %1 hat aber nur %2. Abbruch.").arg(zeilen).arg(ql.size()));
      exit(1);
    }
    QString abt = ql[0].simplified(), konto = ql[2].simplified(), unterkonto = ql[7].simplified();
    if (!comments_only) {
      QString typ = ql[10].simplified(), beschreibung = ql[11].simplified();
      QString verantwortlicher = ql[9].simplified();
      if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden.
      abtList->insertEintrag(abt,konto,unterkonto);
      abtList->setDescription(abt,konto,unterkonto,DescData(beschreibung ,verantwortlicher, typ));
      abtList->setUnterKontoFlags(abt,konto,unterkonto,IS_IN_DATABASE,FLAG_MODE_OR);
    }
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
  if (!zeilen) QMessageBox::critical(NULL, "sctime", "Die Kontenliste ist leer.");
  return zeilen > 0;
}

bool KontoDatenInfoZeit::readZeitFile(QTextStream& ts, AbteilungsListe * abtList) {
  emit kontoListeGeladen();
  return readFile(ts, abtList, false);
}

bool KontoDatenInfoZeit::readInto(AbteilungsListe * abtList) {
  return readInto2(abtList, false);
}

bool KontoDatenInfoZeit::readInto2(AbteilungsListe * abtList, bool comments_only) {
  abtList->clear();
  if (m_DatenFileName.isEmpty()) {
#ifdef WIN32
    return false;
#else
    bool result;
    QString command(m_Kommando.isEmpty()
                    ?  "zeitkonten --mikrokonten --separator='|'"
                    : m_Kommando);
    FILE *file = popen(command.toLocal8Bit(), "r");
    if (!file) {
      QMessageBox::critical(NULL, "sctime: Kontenliste laden",
                            QString("Kann Kommando '%1' nicht ausfuehren: %s").arg(command), strerror(errno));
      return false;
    }
    QTextStream ts(file, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    result = comments_only ? readCommentsFromZeitFile(ts,abtList) : readZeitFile(ts, abtList);
    int rc = pclose(file);
    if (rc == -1 || !WIFEXITED(rc) || WEXITSTATUS(rc)) {
      QMessageBox::critical(NULL, "sctime: Kontenliste laden",
                            QString("Fehler bei '%1': %2").arg(command).arg(rc == -1 ? strerror(errno) : "nicht normal beendet"));
      result = false;
    }
    return result;
#endif
  } else { // aus Datei lesen
    QFile qfile(m_DatenFileName);
    if (!qfile.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(NULL, "sctime: Kontenliste lesen",
                            QString("Kann  '%1' nicht oeffnen: %2").arg(m_DatenFileName, qfile.errorString()));
      return false;
    }
    QTextStream ts(&qfile);
    return comments_only ? readCommentsFromZeitFile(ts, abtList) : readZeitFile(ts, abtList);
  }
}

bool KontoDatenInfoZeit::readDefaultComments(AbteilungsListe * abtList) {
  return readInto2(abtList, true);
}

void KontoDatenInfoZeit::setKommando(const QString& command) {
  m_Kommando=command;
}

bool KontoDatenInfoZeit::checkIn(AbteilungsListe* abtlist) {
  QMessageBox::critical(NULL, "sctime: interner Fehler", "checkin ist nicht implementiert");
  return false;
}

