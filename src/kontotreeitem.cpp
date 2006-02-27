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

#include "kontotreeview.h"
#include "kontotreeitem.h"


KontoTreeItem::KontoTreeItem ( Q3ListView * parent ): Q3ListViewItem(parent) 
{ 
	isBold=false; 
}

KontoTreeItem::KontoTreeItem ( Q3ListViewItem * parent ): Q3ListViewItem(parent) 
{ 
	isBold=false; 
}


KontoTreeItem::KontoTreeItem ( Q3ListView * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8)
     :Q3ListViewItem(parent,label1,label2,label3,label4,label5,label6,label7,label8)
{
   isBold=false;
   isGray=false;
}


KontoTreeItem::KontoTreeItem ( Q3ListViewItem * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
    :Q3ListViewItem(parent,label1,label2,label3,label4,label5,label6,label7,label8)
{
   isBold=false;
   isGray=false;
}

void KontoTreeItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
  QColorGroup newcg=cg;
  if (column==0) {
    QFont newfont=p->font();
    if (isBold) newfont.setWeight(QFont::DemiBold);
    p->setFont(newfont);
  }
  else
    if (isGray) newcg.setColor(QColorGroup::Text,Qt::gray);
  if (text(column).stripWhiteSpace()=="0:00") {
    newcg.setColor(QColorGroup::Text,Qt::gray);
  }
  Q3ListViewItem::paintCell(p,newcg,column,width,align);
}

/*bool KontoTreeItem::acceptDrop ( const QMimeSource * mime )
{
	return mime->provides("application/sctime.seconds");
}

void KontoTreeItem::dropped (QDropEvent * e)
{
	
	QString data;
    data.fromLocal8Bit(e->encodedData("application/sctime.seconds"));
    QStringList datlist=data.split("|");
    KontoTreeView* kview=dynamic_cast<KontoTreeView *>(listView());
    kview->addToItem(this,datlist[0].toInt(),datlist[1].toInt());
	e->accept();
}*/

void KontoTreeItem::setBold(bool bold)
{
  if (bold!=isBold) {
    isBold=bold;
    repaint();
  }
}

void KontoTreeItem::setGray(bool gray)
{
  if (gray!=isGray) {
    isGray=gray;
    repaint();
  }
}



