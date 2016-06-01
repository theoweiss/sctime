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

#ifndef SPECIALREMUNERATIONSDIALOG_H
#define SPECIALREMUNERATIONSDIALOG_H

#include "ui_specialremunerationdialogbase.h"
#include "unterkontoliste.h"
#include "eintragsliste.h"
class QDialog;
class AbteilungsListe;

 /**
  * Der Dialog zum Eingeben eines Datums
  */
class SpecialRemunerationsDialog : public QDialog, private Ui::SpecialRemunerationDialogBase
{
  Q_OBJECT
private:
  QString abteilung,konto,unterkonto;
  int eintragsIdx;
  UnterKontoListe::iterator ukiter;
  
public:
  SpecialRemunerationsDialog(AbteilungsListe* _abtlist, const QString& abt, const QString& ko, const QString& uko, int idx, QWidget* parent = 0);
  ~SpecialRemunerationsDialog();
  /*$PUBLIC_FUNCTIONS$*/

  static void fillListWidget(QListWidget* widget, AbteilungsListe* abtlist, EintragsListe* etl, int eintragsIdx);
  
protected:
  AbteilungsListe* abtlist;

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void          accept();

signals:
  // signals
  void specialRemunerationsChanged(const QString&, const QString&, const QString&, int idx);
};

#endif