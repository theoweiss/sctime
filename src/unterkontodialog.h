/*

    $Id$

    Copyright (C) 2003 Florian Schmitt, Science and Computing AG
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

#ifndef UNTERKONTODIALOG_H
#define UNTERKONTODIALOG_H

#include "qdialog.h"
#include "qpushbutton.h"
#include "qcombobox.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qcheckbox.h"
#include "abteilungsliste.h"
#include "qlistview.h"
#include "timeedit.h"



/**
 * Der Dialog zum Aendern der Eigenschaften eines Unterkontos.
 */
class UnterKontoDialog: public QDialog
{
  Q_OBJECT

  public:
    UnterKontoDialog(const QString& abt,const  QString& ko, const  QString& uko, int idx, AbteilungsListe* abtlist, bool connectZeiten, QWidget * parent=0);
    ZeitBox* getZeitAbzurBox();
    ZeitBox* getZeitBox();

  public slots:
    virtual void accept();

  signals:
    void entryChanged(const QString&, const QString&, const QString&, int idx);

  private slots:
    virtual void aktivesProjektButtonClicked();

  private:
    QLineEdit* commentedit;
    QComboBox* commentcombo;
    QString unterKontoName;
    QString kontoName;
    QString abteilungsName;
    int eintragIndex;    
    AbteilungsListe* abtList;
    ZeitBox *zeitBox;
    ZeitBox *zeitAbzurBox;
    QCheckBox* persoenlichesKonto;
    QCheckBox* aktivesProjekt;
};

#endif

