/*

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

#ifndef DEFAULTCOMMENT_H
#define DEFAULTCOMMENT_H

#include <QString>

class DefaultComment/*: public QObject*/
{
  //Q_OBJECT*/
  public:
    DefaultComment();
    DefaultComment(const QString & text, bool microaccount=false);
    bool isMicroAccount();
    QString getText();
    friend inline bool operator==(const DefaultComment& lhs, const DefaultComment& rhs){return lhs.m_text==rhs.m_text; };
  private:
    QString m_text;
    bool m_microaccount;
    //Q_DISABLE_COPY(DefaultComment);
};

#endif