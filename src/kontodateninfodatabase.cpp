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

#include "kontodateninfodatabase.h"

#include <QObject>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QApplication>
#include <QVariant>
#include <QSqlError>
#include <QSqlQuery>
#include "globals.h"
#include "descdata.h"
#include "DBConnector.h"
#include "unterkontoeintrag.h"
#include "abteilungsliste.h"

static void addOnce(QString& list, const QString& word) {
  if (word.isEmpty()) return;
  if (list.isEmpty())
    list = word;
  else if (!(word == list || list.startsWith(word + " ") || list.contains(" " + word + " ") || list.endsWith(" " + word)))
    list.append(" ").append(word);
}

KontoDatenInfoDatabase::KontoDatenInfoDatabase(DBConnector* dbconnector){ m_dbconnector=dbconnector; }

/**
 * Liest aus einer ODBC-Datenbank nach abtList
 */
bool KontoDatenInfoDatabase::readInto(AbteilungsListe * abtList) {
  emit kontoListeGeladen();
  return readInto2(abtList, false);
}

bool KontoDatenInfoDatabase::readInto2(AbteilungsListe *abtList, bool comments_only) {
  abtList->clear();
  QSqlDatabase db = m_dbconnector->open();
  if (!db.isOpen()) {
    QMessageBox::warning(NULL, QObject::tr("sctime: Kontenliste laden"),
      tr("Fehler beim Aufbau der Verbindung zur Datenbank: %1").arg(db.lastError().driverText()));
    return false;
  }
  bool ret = false;
  QString step = tr("Kontenliste anfordern");
  // zeitkonten  --sql --mikrokonten | sed -E -e 's/(.*)/"\1 "/' 
  QSqlQuery query(
//"set client_encoding to 'utf-8'; "
"Select  "
"   gb.name, " // 0
"   team.kostenstelle, "
"   konto.name,  " 
"   f_username(konto.verantwortlich), " // 3
"   f_username(coalesce(konto.stellvertreter, konto.verantwortlich)), " 
"   konto.abgerechnet_bis, "
"   konto.zeitlimit, "  // 6
"   u.name, "
"   f_username(coalesce(u.verantwortlich, konto.verantwortlich)), "
"   f_username(coalesce(u.stellvertreter, u.verantwortlich, konto.verantwortlich)), " // 9
"   coalesce(unterkonto_art.name || ' (' || u.art || ')', u.art), "
"   coalesce(u.beschreibung, '') || coalesce('; noch nicht abgerechnet: ' || (get_budget_saldo(u.unterkonto_id)::numeric(8,2)), ''), "
"   coalesce(uk.kommentar, '') " // 12
"From "
"  gb "
"  join konto on (gb.gb_id = konto.gb_id) "
"  join team on (team.team_id = konto.team_id)  "
"  join unterkonto u on (u.konto_id = konto.konto_id) "
"  join unterkonto_art on (u.art = unterkonto_art.art) "
"  left join unterkonto_kommentar uk on (u.unterkonto_id = uk.unterkonto_id) "
"Where "
" u.eintragbar "
"Order By gb.name, konto.name, u.name, uk.kommentar "
   , db);
  step = tr("Befehlsstatus prüfen");
  if (query.isActive() ) {
    step = tr("ersten Datensatz holen");
    while (query.next() ) {
      ret = true;
      QString abt = query.value(0).toString().simplified(), ko = query.value(2).toString().simplified();
      QString uko = query.value(7).toString().simplified(), beschreibung = query.value(11).toString();
      QString commentstr = query.value(12).toString(); // Leerzeichen behalten
      if (commentstr.endsWith(":")) commentstr.append(" ");
      if (comments_only) {
	if (!commentstr.isEmpty()) {
	  UnterKontoListe::iterator itUk;
	  UnterKontoListe* ukl;
	  if (abtList->findUnterKonto(itUk,ukl,abt,ko,uko)) {
	    itUk->second.addDefaultComment(commentstr);
	  }
	}
      } else {
        QString responsible = query.value(3).toString().simplified();
        QString type = query.value(10).toString().simplified();
        if (type.isNull())
    	  type="";
        if (beschreibung.isNull())
  	  beschreibung="";
	addOnce(responsible, query.value(4).toString());
        addOnce(responsible, query.value(8).toString());
        addOnce(responsible, query.value(9).toString());

	abtList->insertEintrag(abt,ko,uko);
	abtList->setDescription(abt,ko,uko,DescData(beschreibung,responsible,type));
	abtList->setUnterKontoFlags(abt,ko,uko,IS_IN_DATABASE,FLAG_MODE_OR);
	if (!commentstr.isEmpty()) {
	  UnterKontoListe::iterator itUk;
	  UnterKontoListe* ukl;
	  if (abtList->findUnterKonto(itUk,ukl,abt,ko,uko))
	    itUk->second.addDefaultComment(commentstr);
	}
      }
    }
  }
  if (!ret)
    QMessageBox::warning(NULL, QObject::tr("sctime: Kontenliste laden"),
    tr("Fehler bei: %1 (%2)").arg(step, query.lastError().databaseText()));
  return ret;
}

/**
 * Liest aus einer ODBC-Datenbank nach abtList aber NUR Default Comments
 */
bool KontoDatenInfoDatabase::readDefaultCommentsInto(AbteilungsListe *abtList) {
  return readInto2(abtList, true);
}

bool KontoDatenInfoDatabase::checkIn(AbteilungsListe* abtlist) {
  QMessageBox::critical(NULL, "sctime", QObject::tr("Kann nicht einchecken, da nicht implementiert"));
  return false;
}
