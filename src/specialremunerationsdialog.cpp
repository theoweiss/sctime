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

#include "specialremunerationsdialog.h"
#include <QDialog>
#include "specialremuntypemap.h"
#include "abteilungsliste.h"
#include "unterkontoliste.h"

SpecialRemunerationsDialog::SpecialRemunerationsDialog(AbteilungsListe* _abtlist, const QString& abt, const QString& ko, const QString& uko, int idx, QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    abtlist = _abtlist;
    UnterKontoListe *ukl;

    if (!abtlist->findUnterKonto(ukiter, ukl, abt, ko, uko)) {
      QMessageBox::critical(this, tr("sctime: Special Remuneration times"), tr("No subaccount selected!"));
      return;
    }
    abteilung=abt;
    konto=ko;
    unterkonto=uko;
    eintragsIdx=idx;
    fillListWidget(srListWidget, abtlist, &(ukiter->second), eintragsIdx);
}

void SpecialRemunerationsDialog::fillListWidget(QListWidget* widget, AbteilungsListe* abtlist, EintragsListe* etl, int eintragsIdx)
{
    QSet<QString> selection = (*etl)[eintragsIdx].getAchievedSpecialRemunSet();
    QList<QString> ukosrl = etl->getSpecialRemunNames();
    QList<QString> globalsrl = abtlist->getGlobalSpecialRemunNames();
    for (QList<QString>::iterator srlIt = globalsrl.begin(); srlIt != globalsrl.end(); ++srlIt ) {
       QListWidgetItem* item=new QListWidgetItem(*srlIt,widget);
       widget->addItem(item);
       item->setSelected(selection.remove(*srlIt));
    }
    for (QList<QString>::iterator srlIt = ukosrl.begin(); srlIt != ukosrl.end(); ++srlIt ) {
       QListWidgetItem* item=new QListWidgetItem(*srlIt,widget);
       widget->addItem(item);
       item->setSelected(selection.remove(*srlIt));
    }
    // finally add already selected SpecialRemunTypes, which are no longer available 
    for (QSet<QString>::iterator selIt = selection.begin(); selIt != selection.end(); ++selIt ) {
       QListWidgetItem* item=new QListWidgetItem(*selIt,widget);
       widget->addItem(item);
       item->setSelected(true);
    }
}

SpecialRemunerationsDialog::~SpecialRemunerationsDialog()
{
}

void SpecialRemunerationsDialog::accept()
{
    QDialog::accept();
    QList<QListWidgetItem *> selitems = srListWidget->selectedItems();
    QSet<QString> selection;
    for (QList<QListWidgetItem *>::iterator selIt = selitems.begin(); selIt != selitems.end(); ++selIt ) {
      selection.insert((*selIt)->text());
    }
    ukiter->second[eintragsIdx].setAchievedSpecialRemunSet(selection);
    emit specialRemunerationsChanged(abteilung,konto,unterkonto,eintragsIdx);
}