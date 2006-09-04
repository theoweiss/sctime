#ifndef BEREITSCHAFTSVIEW_H
#define BEREITSCHAFTSVIEW_H

#include <QListView>
#include <QStringList>

class BereitschaftsView: public QListView
{
public:
  BereitschaftsView ( QWidget * parent = 0 );
  ~BereitschaftsView ();
  void setSelectionList(QStringList list);
  QStringList getSelectionList();
};

#endif
