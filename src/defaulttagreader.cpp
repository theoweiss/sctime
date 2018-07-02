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

#include "defaulttagreader.h"
#include <QFile>
#include "globals.h"
#include "qdom.h"

void DefaultTagReader::read(QStringList* taglist) {
  QDomDocument doc("defaulttags");
  QString filename;

  filename=configDir.filePath("defaulttags.xml");

  QFile f( filename );
  if ( !f.open( QIODevice::ReadOnly ) )
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
      if (elem1.tagName()=="tag") {
        QString textstr=elem1.attribute("text");
        if (textstr.isNull()) continue;

        taglist->append(textstr);

      }
    }
  }
}


