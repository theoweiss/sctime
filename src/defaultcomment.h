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