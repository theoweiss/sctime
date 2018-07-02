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

#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include "ui_preferencedialogbase.h"
#include <QDialog>
#include <QFont>

class SCTimeXMLSettings;
 /**
  * Der Dialog zum Eingeben eines Datums
  */
class PreferenceDialog : public QDialog, private Ui::PreferenceDialogBase
{
  Q_OBJECT

public:
  PreferenceDialog(SCTimeXMLSettings* _settings, QWidget* parent = 0);
  ~PreferenceDialog();
  /*$PUBLIC_FUNCTIONS$*/

protected:
  SCTimeXMLSettings* settings;

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void          accept();
  virtual void          selectCustomFont();

signals:
  // signals
private:
  QFont selectedFont;
};

#endif
