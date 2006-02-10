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

#ifndef EINTRAGSLISTE_H
#define EINTRAGSLISTE_H

#include <map>
#include "unterkontoeintrag.h"
#include "qstring.h"
#include "qstringlist.h"
#include "descdata.h"
#include <iostream>

typedef std::map<int,UnterKontoEintrag> Map_Int_UnterKontoEintrag; // Visual-C-Workaround

class EintragsListe: public Map_Int_UnterKontoEintrag
{
  public:
    EintragsListe(): std::map<int,UnterKontoEintrag>()
    {
      flags=0;
    }

    void setDescription(DescData descdata)
    {
      descData=descdata;
    }

    DescData description()
    {
      return descData;
    }

    QStringList* getDefaultCommentList()
    {
      return &defaultCommentList;
    }

    void addDefaultComment(const QString& comment)
    {
      defaultCommentList.append(comment);
    }

    void clearDefaultCommentList()
    {
      defaultCommentList.clear();
    }

    void setFlags(int _flags)
    {
      flags=_flags;
    }

    int getFlags()
    {
      return flags;
    }

    void clear()
    {
      //flags=0;
      Map_Int_UnterKontoEintrag::clear();
    }

  private:
    DescData descData;
    QStringList defaultCommentList;
    int flags;
};

#endif
