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
