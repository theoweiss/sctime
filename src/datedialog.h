/*

    Copyright (C) 2018 science+computing ag
       Authors: Florian Schmitt, Marcus Camen et al.

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

#ifndef DATEDIALOG_H
#define DATEDIALOG_H

#include "ui_datedialogbase.h"
#include <QDateTime>
#include <QDialog>


 /**
  * Dialog to read a date
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
  virtual void          setSelectedDate(const QDate& date);
  virtual void          todaySelected();
  virtual void          weekSelected(int week);
  virtual void          setSelectedDateAndClose(const QDate& date);

signals:
  void dateChanged(const QDate& date);

private:
  QDate selectedDate;
};

#endif
