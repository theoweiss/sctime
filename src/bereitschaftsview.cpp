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

#include "bereitschaftsview.h"
#include <QStringList>
#include "bereitschaftsliste.h"

BereitschaftsView::BereitschaftsView (QWidget * parent): QListView(parent)
{
  setModel(&m_model);
  setSelectionMode(QAbstractItemView::MultiSelection);
}

BereitschaftsView::~BereitschaftsView ()
{}

QStringList BereitschaftsView::getSelectionList()
{
   /*QStringList list;
   QModelIndexList qmi = selectedIndexes();
   for (int i=0; i<qmi.size(); i++)
   {
      list.append(model()->data(qmi.at(i),Qt::DisplayRole).toString());
   }*/
   return static_cast<BereitschaftsModel*>(model())->getSelectionList();
}

void BereitschaftsView::setSelectionList(QStringList list)
{
  static_cast<BereitschaftsModel*>(model())->setSelectionList(list);
  /*for (int i=0; i<list.size(); i++)
  {
    QModelIndex idx=static_cast<BereitschaftsModel*>(model())->indexOf(list.at(i));
    selectionModel()->select(idx,QItemSelectionModel::Select|QItemSelectionModel::Rows);
  }*/
}
