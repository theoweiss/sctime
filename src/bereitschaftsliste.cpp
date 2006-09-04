#include "bereitschaftsliste.h"
#include <assert.h>
#include <iostream>

int BereitschaftsListe::rowCount ( const QModelIndex & parent) const
{
  return m_liste.size();
}


int BereitschaftsListe::columnCount ( const QModelIndex & parent) const
{
  return 1;
}

QVariant BereitschaftsListe::data ( const QModelIndex & index, int role) const
{
  BereitschaftsEintrag eintrag=m_liste.at(index.row());
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch(index.column())
        {
            case 0:
              return QVariant(eintrag.bezeichnung);
            case 1:
              return QVariant(eintrag.aktiviert);
            default:
              assert(0);
        }
    }
    /*case Qt::CheckStateRole:
       return QVariant(eintrag.aktiviert);*/
    case Qt::ToolTipRole:
    {
        return QVariant(eintrag.beschreibung);
    }
    default:
        return QVariant();
  }
}

bool BereitschaftsListe::setData ( const QModelIndex & index, const QVariant & value, int role)
{
  /*BereitschaftsEintrag eintrag=m_liste.at(index.row());
  switch (role)
  {
    case Qt::CheckStateRole:
       eintrag.aktiviert=value.toBool();
       m_liste[index.row()]=eintrag;

       emit dataChanged(index,index);
       return true;
    default:
        return false;
  }*/
  return false;
}

QVariant BereitschaftsListe::headerData (int section, Qt::Orientation orientation, int role) const
{
  switch (role) {
    case Qt::DisplayRole:
      switch(orientation)
      {
        case Qt::Horizontal:
          return QVariant("Bezeichnung");
        default:
          return QVariant();
      }
    default:
      return QVariant();
  }
}

Qt::ItemFlags BereitschaftsListe::flags ( const QModelIndex & index ) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

void BereitschaftsListe::insertEintrag(QString bezeichnung, QString beschreibung, bool aktiviert, int flags)
{
   QModelIndex mi;
   beginInsertRows(mi,m_liste.size(),m_liste.size());
   BereitschaftsEintrag eintrag;
   eintrag.bezeichnung=bezeichnung;
   eintrag.beschreibung=beschreibung;
   eintrag.aktiviert=aktiviert;
   eintrag.flags=flags;
   m_liste.append(eintrag);
   endInsertRows();
}

BereitschaftsListe* BereitschaftsListe::getInstance()
{
  static BereitschaftsListe* instance=NULL;
  if (!instance)
    instance=new BereitschaftsListe();
  return instance;
}

QModelIndex BereitschaftsListe::indexOf(QString bezeichnung)
{
  for (int i=0; i<m_liste.size(); i++)
  {
     if (m_liste.at(i).bezeichnung==bezeichnung)
       return createIndex(i,0);
  }
  return QModelIndex();
}

void BereitschaftsListe::clear()
{
   if (m_liste.size())
   {
     QModelIndex mi;
     beginRemoveRows(mi,m_liste.size()-1,m_liste.size()-1);
     m_liste.clear();
     endRemoveRows();
   }
}


