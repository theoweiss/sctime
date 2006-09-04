#include "bereitschaftsview.h"
#include "bereitschaftsliste.h"
#include <iostream>

BereitschaftsView::BereitschaftsView ( QWidget * parent): QListView(parent)
{
  setModel(BereitschaftsListe::getInstance());
  setSelectionMode(QAbstractItemView::MultiSelection);
}

BereitschaftsView::~BereitschaftsView ()
{}

QStringList BereitschaftsView::getSelectionList()
{
   QStringList list;
   QModelIndexList qmi = selectedIndexes();
   for (int i=0; i<qmi.size(); i++)
   {
      list.append(model()->data(qmi.at(i),Qt::DisplayRole).toString());
   }
   return list;
}

void BereitschaftsView::setSelectionList(QStringList list)
{
  for (int i=0; i<list.size(); i++)
  {
    QModelIndex idx=static_cast<BereitschaftsListe*>(model())->indexOf(list.at(i));
    selectionModel()->select(idx,QItemSelectionModel::Select|QItemSelectionModel::Rows);
  }
}
