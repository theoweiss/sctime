#ifndef BEREITSCHAFTSLISTE_H
#define BEREITSCHAFTSLISTE_H

#include <QList>
#include <QString>

struct BereitschaftsEintrag
{
  QString bezeichnung;
  QString beschreibung;
};

class BereitschaftsListe: public QList<BereitschaftsEintrag>
{
public:
  void insertEintrag(QString bezeichnung, QString beschreibung);
  static BereitschaftsListe* getInstance();
private:
  BereitschaftsListe() {};
};

#endif // BEREITSCHAFTSLISTE_H
