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

#ifndef BEREITSCHAFTSVIEW_H
#define BEREITSCHAFTSVIEW_H

#include <QListView>
class QStringList;
#include "bereitschaftsmodel.h"

class BereitschaftsView: public QListView
{
public:
  BereitschaftsView ( QWidget * parent = 0 );
  ~BereitschaftsView ();
  void setSelectionList(QStringList list);
  QStringList getSelectionList();
private:
  BereitschaftsModel m_model;
};

#endif
