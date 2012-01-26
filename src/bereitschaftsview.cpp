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
