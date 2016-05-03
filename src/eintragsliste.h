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
#include <QStringList>
#include <QColor>
#include <QVector>
#include "descdata.h"
#include "defaultcomment.h"
#include "unterkontoeintrag.h"

typedef std::map<int,UnterKontoEintrag> Map_Int_UnterKontoEintrag; // Visual-C-Workaround

class EintragsListe: public Map_Int_UnterKontoEintrag
{
  public:
    EintragsListe(): std::map<int,UnterKontoEintrag>()
    {
      flags=0;
      m_hasColor=false;
      m_bgColor=Qt::white;
    }

    void setDescription(DescData descdata)
    {
      descData=descdata;
    }

    DescData description()
    {
      return descData;
    }

    QVector<DefaultComment>* getDefaultCommentList()
    {
      return &defaultCommentList;
    }

    void addDefaultComment(const QString& text, bool microaccount=false)
    {
      // ignore duplicates
      if (!defaultCommentList.contains(DefaultComment(text)))
      {
         defaultCommentList.append(DefaultComment(text,microaccount));
      }
    }
    
    void addDefaultComment(const DefaultComment& comment)
    {
      // ignore duplicates
      if (!defaultCommentList.contains(comment))
      {
         defaultCommentList.append(comment);
      }
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
      bereitschaft.clear();
    }

    QStringList getBereitschaft()
    {
      return bereitschaft;
    }

    void setBereitschaft(QStringList _bereitschaft)
    {
      bereitschaft=_bereitschaft;
    }

    void setBgColor(QColor bgColor)
    {
      m_bgColor=bgColor;
      m_hasColor=true;
    }

    QColor getBgColor()
    {
      return m_bgColor;
    }

    bool hasBgColor()
    {
      return m_hasColor;
    }

    void unsetBgColor()
    {
      m_hasColor=false;
    }
    
    const SpecialRemunTypeList& getSpecialRemunTypeList() const
    {
      return m_specialRemunTypeList;
    }
    
    void setSpecialRemunTypeList(const SpecialRemunTypeList& srtl)
    {
      m_specialRemunTypeList=srtl;
    }

  private:
    DescData descData;
    QVector<DefaultComment> defaultCommentList;
    QStringList bereitschaft;
    int flags;
    QColor m_bgColor;
    bool m_hasColor;
    SpecialRemunTypeList m_specialRemunTypeList;
};

#endif
