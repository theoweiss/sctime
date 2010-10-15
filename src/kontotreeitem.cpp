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


KontoTreeItem::KontoTreeItem ( QTreeWidget * parent ): QTreeWidgetItem(parent)
{
  isBold=false;
  m_bgColor=Qt::white;
  setGray();
}

KontoTreeItem::KontoTreeItem ( QTreeWidgetItem * parent ): QTreeWidgetItem(parent)
{
  isBold=false;
  m_bgColor=Qt::white;
  setGray();
}

KontoTreeItem::KontoTreeItem ( QTreeWidget * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8)
     :QTreeWidgetItem(parent,0)
{
  isBold=false;
  setGray();
  m_bgColor=Qt::white;

  this->setText(0, label1);
  this->setText(1, label2);
  this->setText(2, label3);
  this->setText(3, label4);
  this->setText(4, label5);
  this->setText(5, label6);
  this->setText(6, label7);
  this->setText(7, label8);
}


KontoTreeItem::KontoTreeItem ( QTreeWidgetItem * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
    :QTreeWidgetItem(parent)
{
  isBold=false;
  setGray();
  m_bgColor=Qt::white;

  this->setText(0, label1);
  this->setText(1, label2);
  this->setText(2, label3);
  this->setText(3, label4);
  this->setText(4, label5);
  this->setText(5, label6);
  this->setText(6, label7);
  this->setText(7, label8);
}

void KontoTreeItem::setBold(bool bold)
{
  isBold=bold;
  QFont f = font(0);

  if( bold )
  {
    f.setWeight(QFont::Bold);
  }
  else
  {
    f.setWeight(QFont::Normal);
  }

  setFont(0, f);
}

void KontoTreeItem::setGray()
{
  std::vector<int> columns;
  columns.push_back(3);
  columns.push_back(4);
  QBrush brush;
  for(int i=0; i<columns.size(); i++)
  {
    brush = foreground( columns.at(i) );
    if ((text(columns.at(i)).simplified()=="0:00")||(text(columns.at(i)).simplified().startsWith("+"))) {
      brush.setColor( Qt::gray );
      isGray=true;      
    }
    else
    {
      brush.setColor(Qt::black);
      isGray=false;
    }
    setForeground(columns.at(i), brush);
  }
}

void KontoTreeItem::setBgColor(const QColor bgColor)
{
  if (bgColor!=m_bgColor) {
    m_bgColor = bgColor;
    for( int i=0; i<6; i++ )
    {
      setBackgroundColor(i, bgColor);
    }
  }
}


KontoTreeItem* KontoTreeItem::nextSibling( )
{
  KontoTreeItem *parent = (KontoTreeItem*)this->parent();
  KontoTreeItem *nextSibling;
  if(parent){
    nextSibling = (KontoTreeItem*) this->parent()->child(parent->indexOfChild(this)+1);
  }
  else {
    KontoTreeView *treeWidget = (KontoTreeView*)this->treeWidget();
    nextSibling = (KontoTreeItem*) treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(this)+1);
  }
  return nextSibling;
}
