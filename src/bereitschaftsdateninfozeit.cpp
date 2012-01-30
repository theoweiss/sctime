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

#include "bereitschaftsdateninfozeit.h"

#include <errno.h>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include "globals.h"
#include "bereitschaftsliste.h"


BereitschaftsDatenInfoZeit::BereitschaftsDatenInfoZeit()
  : m_DatenFileName("") {}

BereitschaftsDatenInfoZeit::BereitschaftsDatenInfoZeit(QString sourcefile)
  : m_DatenFileName(sourcefile) {}

bool BereitschaftsDatenInfoZeit::readBereitschaftsFile(QTextStream& ts, BereitschaftsListe * berList) {
  int zeilen = 0;
  for (QString l; !(l = ts.readLine()).isNull();) {
    zeilen++;
    if (l.isEmpty()) continue;
     QStringList ql = l.split("|");
     if (ql.size()<2) {
       QMessageBox::critical(NULL, QObject::tr("sctime: Liste der Bereitschaftsarten lesen"),
                             QObject::tr("Zeile %1 muss zwei Spalten haben, hat aber nur %2").arg(zeilen).arg(ql.size()));
       exit(1);
     }
     QString name = ql[0].simplified();
     QString beschreibung = ql[1].simplified();
     if (beschreibung.isEmpty()) beschreibung = ""; // Leerer String, falls keine Beschr. vorhanden. //FIXME: notwendig?
     berList->insertEintrag(name, beschreibung);
   }
   if (zeilen == 0)
     QMessageBox::warning(NULL, "sctime: Liste der Bereitschaftsarten lesen", "Liste ist leer");
     return zeilen > 0;
}

bool BereitschaftsDatenInfoZeit::readInto(BereitschaftsListe * berList) {
  berList->clear();

  if (m_DatenFileName.isEmpty()) {
#ifdef WIN32
    return false;
#else
    FILE* file;
    QString command = "zeitbereitls --separator='|'";
    file = popen(command.toLocal8Bit(), "r");
    if (!file) {
      QMessageBox::critical(NULL, QObject::tr("sctime: Bereitschaftsarten laden"),
                            QObject::tr("Kann Kommando '%1' nicht ausfuehren: %s").arg(command), strerror(errno));
      return false;
    }
    QTextStream ts(file, QIODevice::ReadOnly);
    ts.setCodec("UTF-8");
    bool result = readBereitschaftsFile(ts,berList);
    int rc = pclose(file);
    if (rc == -1 || !WIFEXITED(rc) || WEXITSTATUS(rc)) {
      QMessageBox::critical(NULL, QObject::tr("sctime: Bereitschaftsarten laden"),
                            QObject::tr("Fehler bei '%1': %2").arg(command).arg(rc == -1 ? strerror(errno) : "nicht normal beendet"));
      result = false;
    }
    return result;
#endif
  } else { // aus Datei lesen
    QFile qfile(m_DatenFileName);
    if (!qfile.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(NULL, QObject::tr("sctime: Bereitschaftsarten laden"),
                            QObject::tr("Kann Datei '%1' nicht öffnen: %2").arg(m_DatenFileName, qfile.errorString()));
      return false;
    }
    QTextStream ts (&qfile);
    return readBereitschaftsFile(ts, berList);
  }
}


