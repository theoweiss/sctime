/*

    $Id$

    Copyright (C) 2003 Florian Schmitt, science + computing ag
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

#include "preferencedialog.h"
#include "qspinbox.h"
#include "qcheckbox.h"


PreferenceDialog::PreferenceDialog(SCTimeXMLSettings* _settings, QWidget *parent)
: PreferenceDialogBase(parent)
{
    settings = _settings;
    zeitIncBox->setValue(settings->timeIncrement()/60);
    fastZeitIncBox->setValue(settings->fastTimeIncrement()/60);
    entrySaveCheckbox->setChecked(settings->alwaysSaveEntry());
    powerUserCheckbox->setChecked(settings->powerUserView());
    singleClickCheckbox->setChecked(settings->singleClickActivation());
}

PreferenceDialog::~PreferenceDialog()
{
}

void PreferenceDialog::accept()
{
    QDialog::accept();
    settings->setTimeIncrement(zeitIncBox->value()*60);
    settings->setFastTimeIncrement(fastZeitIncBox->value()*60);
    settings->setAlwaysSaveEntry(entrySaveCheckbox->isChecked());
    settings->setPowerUserView(powerUserCheckbox->isChecked());
    settings->setSingleClickActivation(singleClickCheckbox->isChecked());
}
