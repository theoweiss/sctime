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

#include "defaultcommentreader.h"

#include <QFile>
#include <QDir>
#include "globals.h"
#include "abteilungsliste.h"
#include "qdom.h"

/**
 * Liest alle Einstellungen.
 */

bool DefaultCommentReader::read(AbteilungsListe* abtList, const std::vector<QString>& xmlfilelist) {
  QDomDocument doc("comments");

  for (unsigned int i=0; i<xmlfilelist.size(); i++) {
    QDir dummy(xmlfilelist[i]);
    QString filename = dummy.isRelative() ? configDir.filePath(xmlfilelist[i]) :xmlfilelist[i];
    {
        QFile f(filename);
        if ( !f.open( QIODevice::ReadOnly ) ) {
          logError(QObject::tr("default comments: cannot open file '%1': %2").arg(filename, f.errorString()));
          return false;
        }
        QString domerror;
        int domerrorline;
        int domerrorcol;
        if ( !doc.setContent( &f, false, &domerror,&domerrorline,&domerrorcol ) ) {
          logError(QObject::tr("default comments: ") + domerror);
          return false;
        }
      }

      QDomElement docElem = doc.documentElement();

      for( QDomNode node1 = docElem.firstChild(); !node1.isNull(); node1 = node1.nextSibling() ) {
          QDomElement elem1 = node1.toElement();
          if( !elem1.isNull() ) {
              if (elem1.tagName()=="abteilung") {
                  QString abteilungstr=elem1.attribute("name");
                  if (abteilungstr.isNull()) continue;

                  for( QDomNode node2 = elem1.firstChild(); !node2.isNull(); node2 = node2.nextSibling() ) {
                      QDomElement elem2 = node2.toElement();
                      if( !elem2.isNull() ) {
                          if (elem2.tagName()=="konto") {
                              QString kontostr=elem2.attribute("name");
                              if (kontostr.isNull()) continue;

                              for( QDomNode node3 = elem2.firstChild(); !node3.isNull(); node3 = node3.nextSibling() ) {
                                  QDomElement elem3 = node3.toElement();
                                  if( !elem3.isNull() ) {
                                      if (elem3.tagName()=="unterkonto") {
                                          QString unterkontostr=elem3.attribute("name");
                                          if (unterkontostr.isNull()) continue;

                                          for( QDomNode node4 = elem3.firstChild(); !node4.isNull(); node4 = node4.nextSibling() ) {
                                              QDomElement elem4 = node4.toElement();
                                              if( !elem4.isNull() ) {
                                                  if (elem4.tagName()=="kommentar") {
                                                      QString commentstr=elem4.attribute("text");
                                                      if (commentstr.isNull()||commentstr.isEmpty()) continue;
                                                      UnterKontoListe::iterator itUk;
                                                      UnterKontoListe* ukl;
                                                      if (abtList->findUnterKonto(itUk,ukl,abteilungstr,kontostr,unterkontostr)) {
                                                          itUk->second.addDefaultComment(commentstr);
                                                      }
                                                      else
                                                        logError(QObject::tr("default comments: cannot find subaccount %1").arg(unterkontostr));
                                                  }
                                              }
                                          }
                                      }
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }
  }
  return true;
}


