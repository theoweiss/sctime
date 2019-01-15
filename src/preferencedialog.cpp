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

    switch(settings->defCommentDisplayMode()) {
        case SCTimeXMLSettings::DM_BOLD: radioDefCommBold->setChecked(true) ; break;
        case SCTimeXMLSettings::DM_NOTUSEDBOLD: radioAvailabeDefCommNotSelectedBold->setChecked(true); break;
        case SCTimeXMLSettings::DM_NOTBOLD: radioNoBold->setChecked(true); break;
    }

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
    SCTimeXMLSettings::DefCommentDisplayModeEnum dm=SCTimeXMLSettings::DM_BOLD;
    if (radioAvailabeDefCommNotSelectedBold->isChecked()) {
        dm = SCTimeXMLSettings::DM_NOTUSEDBOLD;
    } else
    if (radioNoBold->isChecked()) {
        dm = SCTimeXMLSettings::DM_NOTBOLD;
    }
    settings->setDefCommentDisplayMode(dm);
}
