#ifndef BEREITSCHAFTSLISTE_H
#define BEREITSCHAFTSLISTE_H

#include <QAbstractTableModel>

struct BereitschaftsEintrag
{
  QString bezeichnung;
  QString beschreibung;
  int flags;
};

class BereitschaftsListe: public QList<BereitschaftsEintrag>
{
public:
  void insertEintrag(QString bezeichnung, QString beschreibung, int flags=0);
  static BereitschaftsListe* getInstance();
private:
  BereitschaftsListe() {};
};

#endif // BEREITSCHAFTSLISTE_H
