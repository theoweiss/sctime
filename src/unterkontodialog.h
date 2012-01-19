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

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>

#include "abteilungsliste.h"
#include "timeedit.h"

#include "bereitschaftsview.h"
#include "globals.h"
#include "timemainwindow.h"
#include "sctimexmlsettings.h"

/**
 * Der Dialog zum Aendern der Eigenschaften eines Unterkontos.
 */
class UnterKontoDialog: public QDialog
{
  Q_OBJECT

  public:
    UnterKontoDialog(const QString& abt,const  QString& ko, const  QString& uko, int idx,
                     AbteilungsListe* abtlist,  QStringList* taglist,
                     bool connectZeiten, QMainWindow * parent=0, bool readOnly=false);
    ZeitBox* getZeitAbzurBox();
    ZeitBox* getZeitBox();
    QString getComment();
    void setSettings(SCTimeXMLSettings* s);
  public slots:
    virtual void accept();
    virtual void checkInput();

  signals:
    void entryChanged(const QString&, const QString&, const QString&, int idx);
    void bereitschaftChanged(const QString&, const QString&, const QString&);
    void entryActivated();

  private slots:
    virtual void projektAktivierenButtonClicked();
    virtual void addTag();

  private:
    QLineEdit* commentedit;
    QComboBox* commentcombo;
    QComboBox* tagcombo;
    QString unterKontoName;
    QString kontoName;
    QString abteilungsName;
    int eintragIndex;
    AbteilungsListe* abtList;
    ZeitBox *zeitBox;
    ZeitBox *zeitAbzurBox;
    QCheckBox* persoenlichesKonto;
    QPushButton* projektAktivieren;
    EintragsListe* m_unterkonto;
    QWidget* parent;
    SCTimeXMLSettings* settings;
    //BereitschaftsView* bereitschaftsView;
    //QStringList m_bereitschaften;
};

#endif

