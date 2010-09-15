/*

    $Id$

    Copyright (C) 2003 Florian Schmitt, Science + Computing ag
                       f.schmitt@science-computing.de
    Copyright (C) 2003 Marcus Camen, science + computing ag
                       m.camen@science-computing.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef DATEDIALOG_H
#define DATEDIALOG_H

#include "ui_datedialogbase.h"
#include <QDateTime>
#include <QDialog>


 /**
  * Der Dialog zum Eingeben eines Datums
  */
class DateDialog : public QDialog, private Ui::DateDialogBase
{
  Q_OBJECT

public:
  DateDialog(const QDate& datum, QWidget* parent = 0);
  ~DateDialog();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void          accept();
  virtual void          apply();
  virtual void          dateChangedSlot(QDate date);

signals:
  void dateChanged(const QDate& datum);

private:
  int currentMonth;
  int currentYear;
};

#endif
