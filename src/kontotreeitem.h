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

#include <QTreeWidgetItem>
#include <QDropEvent>
#include <QColor>
#include <QPen>
#include <QPainter>

#include <vector>

class KontoTreeItem: public QTreeWidgetItem
{
  public:

    KontoTreeItem ( QTreeWidget * parent );
    KontoTreeItem ( QTreeWidgetItem * parent );


    KontoTreeItem ( QTreeWidget * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );

    KontoTreeItem ( QTreeWidgetItem * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );

    void setBold(bool bold);

    void setGray();

    void setBgColor(const QColor bgColor);

    QString getLabel1();

    KontoTreeItem* nextSibling( );

  protected:
    void paintEvent(QPaintEvent *event);

  private:
    bool isBold;
    bool isGray;
    QColor m_bgColor;
};

#endif

