#ifndef BEREITSCHAFTSLISTE_H
#define BEREITSCHAFTSLISTE_H

#include <QAbstractTableModel>

struct BereitschaftsEintrag
{
  QString bezeichnung;
  QString beschreibung;
  bool aktiviert;
  int flags;
};

class BereitschaftsListe: public QAbstractTableModel
{
public:
  int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  void insertEintrag(QString bezeichnung, QString beschreibung, bool aktiviert=false, int flags=0);
  bool setData (const QModelIndex & index, const QVariant & value, int role);
  QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  void clear();
  static BereitschaftsListe* getInstance();
  QModelIndex indexOf(QString);
private:
  QList<BereitschaftsEintrag> m_liste;
  BereitschaftsListe() {};
};

#endif // BEREITSCHAFTSLISTE_H
