/* $Id$*/

#include <iostream>

#include "defaultcommentreader.h"
#include "globals.h"
#include "abteilungsliste.h"
#include "qfile.h"
#include "qdir.h"
#ifndef NO_XML
#include "qdom.h"
#endif

/**
 * Liest alle Einstellungen.
 */

void DefaultCommentReader::read()
{
  #ifndef NO_XML
  QDomDocument doc("comments");
  QString filename;

  filename=configDir+"/defaultcomments.xml";

  QFile f( filename );
  if ( !f.open( IO_ReadOnly ) )
      return;
  if ( !doc.setContent( &f ) ) {
      f.close();
      return;
  }
  f.close();

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
                          if (commentstr.isNull()) continue;
                          UnterKontoListe::iterator itUk;
                          UnterKontoListe* ukl;
                          if (abtList->findUnterKonto(itUk,ukl,abteilungstr,kontostr,unterkontostr)) {
                            itUk->second.addDefaultComment(commentstr);
                          }
                          else
                            std::cerr<<"Kann Unterkonto für Defaultkommentar nicht finden!"<<std::endl;
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
  #endif
}


