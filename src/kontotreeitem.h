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

#ifndef KONTOTREE_ITEM_H
#define KONTOTREE_ITEM_H

#include "qlistview.h"
#include "qpainter.h"

class KontoTreeItem: public QListViewItem
{
  public:

    KontoTreeItem ( QListView * parent ): QListViewItem(parent) { isBold=false; }
    KontoTreeItem ( QListViewItem * parent ): QListViewItem(parent) { isBold=false; }

    
    KontoTreeItem ( QListView * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )
     :QListViewItem(parent,label1,label2,label3,label4,label5,label6,label7,label8)
    {
      isBold=false;
      isGray=false;
    }


    KontoTreeItem ( QListViewItem * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )
     :QListViewItem(parent,label1,label2,label3,label4,label5,label6,label7,label8)
    {
      isBold=false;
      isGray=false;
    }

    
    virtual void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
    {
      QColorGroup newcg=cg;
      if (column==0) {
        QFont newfont=p->font();
        if (isBold) newfont.setWeight(QFont::DemiBold);
        p->setFont(newfont);
      }
      else
        if (isGray) newcg.setColor(QColorGroup::Text,gray);
     if (text(column).stripWhiteSpace()=="0:00") {
        newcg.setColor(QColorGroup::Text,gray);
      }
      QListViewItem::paintCell(p,newcg,column,width,align);
    }


    void setBold(bool bold)
    {
      if (bold!=isBold) {
        isBold=bold;
        repaint();
      }
    }

    void setGray(bool gray)
    {
      if (gray!=isGray) {
        isGray=gray;
        repaint();
      }
    }

  private:
    bool isBold;
    bool isGray;
};

#endif

