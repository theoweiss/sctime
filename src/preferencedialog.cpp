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
#include <QSpinBox>
#include <QCheckBox>
#include <QFontDialog>
#include "sctimexmlsettings.h"


PreferenceDialog::PreferenceDialog(SCTimeXMLSettings* _settings, QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(customFontSelectButton, SIGNAL(clicked()), this, SLOT(selectCustomFont()));

    settings = _settings;
    zeitIncBox->setValue(settings->timeIncrement()/60);
    fastZeitIncBox->setValue(settings->fastTimeIncrement()/60);
    entrySaveCheckbox->setChecked(settings->alwaysSaveEntry());
    powerUserCheckbox->setChecked(settings->powerUserView());
    singleClickCheckbox->setChecked(settings->singleClickActivation());
    showTypeCheckBox->setChecked(settings->showTypeColumn());
    showPSPCheckBox->setChecked(settings->showPSPColumn());
    useDefaultCommentIfUniqueCheckBox->setChecked(settings->useDefaultCommentIfUnique());
    dragNDropCheckbox->setChecked(settings->dragNDrop());
    persoenlicheKontensummeCheckbox->setChecked(settings->persoenlicheKontensumme());
    customFontCheckBox->setChecked(settings->useCustomFont());
    customFontSelectButton->setEnabled(settings->useCustomFont());
    showSpecialRemunSelector->setChecked(settings->showSpecialRemunSelector());
    QString custFont=settings->customFont();
    int custFontSize=settings->customFontSize();
    if (custFont.isEmpty()) {
            selectedFont=this->font();
    } else {
            selectedFont=QFont(custFont, custFontSize);
    }
    fontPreview->setFont(selectedFont);

    /* make this a nice mainwindow-like looking preview */
    fontPreview->header()->resizeSection(0, 200);
    fontPreview->expandAll();
    fontPreview->topLevelItem(1)->child(0)->child(0)->setSelected(true);
#ifndef Q_OS_MAC
    fontPreview->setSelectionMode(QTreeWidget::NoSelection);
#else
    /* On Mac OS X with NoSelection the TreeView gives no visual feedback, what
     * the currently selected item is. So we have to switch on SingleSelection to
     * avoid user confusion. */
    fontPreview->setSelectionMode(QTreeWidget::SingleSelection);
#endif

}

void PreferenceDialog::selectCustomFont() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, selectedFont, this);
    if (ok) {
        selectedFont=font;
    }
    fontPreview->setFont(selectedFont);
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
    settings->setShowTypeColumn(showTypeCheckBox->isChecked());
    settings->setShowPSPColumn(showPSPCheckBox->isChecked());
    settings->setUseDefaultCommentIfUnique(useDefaultCommentIfUniqueCheckBox->isChecked());
    settings->setUseCustomFont(customFontCheckBox->isChecked());
    settings->setCustomFont(selectedFont.family());
    settings->setDragNDrop(dragNDropCheckbox->isChecked());
    settings->setPersoenlicheKontensumme(persoenlicheKontensummeCheckbox->isChecked());
    settings->setCustomFontSize(selectedFont.pointSize());
    settings->setShowSpecialRemunSelector(showSpecialRemunSelector->isChecked());
}
