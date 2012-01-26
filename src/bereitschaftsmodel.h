#ifndef BEREITSCHAFTSMODEL_H
#define BEREITSCHAFTSMODEL_H

#include <QStringList>
#include <QModelIndex>
class BereitschaftsModel: public QAbstractTableModel {
public:
  BereitschaftsModel();
  int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  bool setData (const QModelIndex & index, const QVariant & value, int role);
  QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  static BereitschaftsModel* getInstance();
  QModelIndex indexOf(QString);
  void setSelectionList(QStringList list);
  QStringList getSelectionList();
private:
  QList<bool> m_activatedList;
};

#endif
