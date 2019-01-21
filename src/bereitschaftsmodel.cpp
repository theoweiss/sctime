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

#include "bereitschaftsmodel.h"
#include "bereitschaftsliste.h"

BereitschaftsModel::BereitschaftsModel()
{
  BereitschaftsListe* bl=BereitschaftsListe::getInstance();
  for (int i=0; i<bl->size(); i++)
    m_activatedList.append(false);
}

int BereitschaftsModel::rowCount ( const QModelIndex & ) const
{
  return m_activatedList.size();
}

int BereitschaftsModel::columnCount ( const QModelIndex & ) const
{
  return 1;
}

QVariant BereitschaftsModel::data ( const QModelIndex & index, int role) const
{
  if ((index.row()<0)||(index.column()<0))
    return QVariant();
  BereitschaftsListe* bl=BereitschaftsListe::getInstance();
  BereitschaftsEintrag eintrag=bl->at(index.row());
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch(index.column())
        {
            case 0:
              return QVariant(eintrag.bezeichnung);
            /*case 1:
              return QVariant(eintrag.aktiviert);*/
            default:
              return QVariant();
        }
    }
    case Qt::CheckStateRole:
       return QVariant(m_activatedList.at(index.row()));
    case Qt::ToolTipRole:
    {
        return QVariant(eintrag.beschreibung);
    }
    default:
        return QVariant();
  }
}

bool BereitschaftsModel::setData ( const QModelIndex & index, const QVariant & value, int role)
{
  switch (role)
  {
    case Qt::CheckStateRole:
       m_activatedList[index.row()]=m_activatedList[index.row()]^value.toBool();

       emit dataChanged(index,index);
       return true;
    default:
        return false;
  }
 // return false;
}

QVariant BereitschaftsModel::headerData (int, Qt::Orientation orientation, int role) const
{
  switch (role) {
    case Qt::DisplayRole:
      switch(orientation)
      {
        case Qt::Horizontal:
          return QVariant(QObject::tr("Designation"));
        default:
          return QVariant();
      }
    default:
      return QVariant();
  }
}

Qt::ItemFlags BereitschaftsModel::flags ( const QModelIndex & ) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
}

QModelIndex BereitschaftsModel::indexOf(QString bezeichnung)
{
  BereitschaftsListe* bl=BereitschaftsListe::getInstance();
  for (int i=0; i<bl->size(); i++)
  {
     if (bl->at(i).bezeichnung==bezeichnung)
       return createIndex(i,0);
  }
  return QModelIndex();
}

void BereitschaftsModel::setSelectionList(QStringList list)
{
  BereitschaftsListe* bl=BereitschaftsListe::getInstance();
  for (int i=0; i<bl->size(); i++) {
    bool found=false;
    for (int j=0; j<list.size(); j++) {
      if (list.at(j)==bl->at(i).bezeichnung) {
        found=true;
        break;
      }
    }
    m_activatedList[i]=found;
  }
}

QStringList BereitschaftsModel::getSelectionList()
{
  QStringList ret;
  BereitschaftsListe* bl=BereitschaftsListe::getInstance();
  for (int i=0; i<bl->size(); i++) {
    if (m_activatedList.at(i))
       ret.append(bl->at(i).bezeichnung);
  }
  return ret;
}


