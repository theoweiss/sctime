/*

    $Id$

    Copyright (C) 2003 Florian Schmitt, Science + Computing ag
                       f.schmitt@science-computing.de

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

#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include "preferencedialogbase.h"
#include "sctimexmlsettings.h"


 /**
  * Der Dialog zum Eingeben eines Datums
  */
class PreferenceDialog : public PreferenceDialogBase
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

signals:
  // signals
};

#endif
