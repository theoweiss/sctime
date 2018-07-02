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
    
    const QList<QString>& getSpecialRemunNames() const
    {
      return m_specialRemunNames;
    }
    
    void setSpecialRemunNames(const QList<QString>& srtn)
    {
      m_specialRemunNames=srtn;
    }

  private:
    DescData descData;
    QVector<DefaultComment> defaultCommentList;
    QStringList bereitschaft;
    int flags;
    QColor m_bgColor;
    bool m_hasColor;
    QList<QString> m_specialRemunNames;
};

#endif
